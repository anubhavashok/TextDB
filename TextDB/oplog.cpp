//
//  oplog.cpp
//  TextDB
//
//  Created by Anubhav on 3/2/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "oplog.h"
#include "operation.h"
#include <future>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "proposer.cpp"
#include "acceptor.cpp"
#include <curlpp/cURLpp.hpp>
#include <chrono>
#include <algorithm>
#include <limits>
#include "db.h"

// Static variables

void Oplog::insert(Operation op)
{
    operations.push_back(op);
}


Operation Oplog::insert(string serialized)
{
    cout << "Deserializing operation" << endl;
    stringstream instream(ios_base::in | ios_base::out);
    instream << serialized;
    boost::archive::text_iarchive ar(instream);
    Operation op;
    ar >> op;
    insert(op);
    return op;
}

vector<pair<long long, string>> Oplog::pingAllReplicas(vector<string> replicas)
{
    vector<pair<long long, string>> times;
    cout << "Ping all replicas to order by latency" << endl;
    for (string s: replicas)
    {
        // prepare get request
        try {
            curlpp::Cleanup myCleanup;
            std::ostringstream os;
            curlpp::options::Url url(s + "/ping");
            milliseconds start = duration_cast< milliseconds >(high_resolution_clock::now().time_since_epoch());
            os << url;
            milliseconds end = duration_cast< milliseconds >(high_resolution_clock::now().time_since_epoch());
            times.push_back(make_pair(end.count()-start.count(), s));
            cout << "Ping to replica: " << s << " took " << (end-start).count() << endl;
        } catch(curlpp::RuntimeError & e) {
            std::cout << e.what() << std::endl;
            times.push_back(make_pair(numeric_limits<long long>::max(), s));
        } catch(curlpp::LogicError & e) {
            std::cout << e.what() << std::endl;
            times.push_back(make_pair(numeric_limits<long long>::max(), s));
        }
    }
    sort(times.begin(), times.end(), [](pair<long long, string> p1, pair<long long, string> p2){ return p1.first - p2.first;});
    return times;
}


Oplog::Oplog(vector<string> replicas, fs::path replpath, shared_ptr<DB> db)
: proposer(replicas), acceptor(replpath), db(db)
{
    //ordered_replicas = pingAllReplicas(replicas);
    
    oplog_disk.open((replpath / "oplog.var").string(), ios::app);
    
    // Read persisted operations that have already been committed
    max_n = 0;
    ifstream oplog_read((replpath / "oplog.var").string());
    string instring;
    while(std::getline(oplog_read, instring, '|').good()) {
        cout << "Loading persisted oplog " << endl;
        stringstream instream(ios_base::in | ios_base::out);
        instream << instring;
        boost::archive::text_iarchive ar(instream);
        Operation op;
        ar >> op;
        cout << "(op): " << op.cmd << " " << op.n << endl;
        operations.push_back(op);
        max_n = max(max_n, op.n);
    }
    sync();
    // start paxos
    DB::ready = true;
}

bool Oplog::propose(Operation op)
{
    // run multi-paxos consensus routine (max 10 iterations)
    int i = 10;
    bool success = false;
    while ((!success) && (i > 0)) {
        success = proposer.propose(op);
    }

    if (success) {
        cout << "Adding op to operations vector" << endl;
        operations.push_back(op);
        // since handleQuery already commits, do not commit here
        //commit(op);
    }
    return success;
}

void Oplog::sync()
{
    for (auto p: ordered_replicas) {
        string replica = p.second;
        try {
            curlpp::options::Url url(replica + "/helpsync/" + to_string(max_n));
            ostringstream os;
            os << url;
            // commit all operations
            string instring = os.str();
            stringstream instream(ios_base::in | ios_base::out);
            instream << instring;
            boost::archive::text_iarchive ar(instream);
            OperationContainer container;
            ar >> container;
            cout << "container.operations.size()" << container.operations.size() << endl;
            for (const Operation& op: container.operations) {
                commit(op);
            }
            break;
        } catch (exception& e) {
        }
    }
    
    
}


void Oplog::commit(Operation op)
{
    // perform state machine operation
    // persist operation to oplog on disk
    stringstream outstream(ios_base::out);
    boost::archive::text_oarchive ar(outstream);
    ar << op;
    string outstring = outstream.str();
    oplog_disk << outstring << "|";
    oplog_disk.flush();
    cout << "fake_commit: " << op.cmd << " " << op.n << endl;
    db->queryFunctions[op.cmd](db.get(), cout, op.args);
}

vector<Operation> Oplog::helpSync(long long n)
{
    vector<Operation> new_ops;
    cout << "helpSync operations.size(): " << operations.size() << endl;
    for (const Operation& op: operations) {
        if (op.n > n) new_ops.push_back(op);
    }
    return new_ops;
}