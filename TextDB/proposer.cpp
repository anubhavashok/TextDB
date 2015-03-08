//
//  propser.cpp
//  TextDB
//
//  Created by Anubhav on 3/6/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "proposer.h"
#include "oplog.h"
#include <iostream>
#include <chrono>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTMLForm.h>
#include <Poco/Net/HTTPResponse.h>
#include "operation.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Easy.hpp>



using namespace std::chrono;
using namespace Poco::Net;


template <class Operation>
Proposer<Operation>::Proposer(vector<string> _replicas)
{
    replicas = _replicas;
    majority = replicas.size()/2 + 1;
}

template <class Operation>
void Proposer<Operation>::propose(Operation val)
{
    // pick n,
    milliseconds ms = duration_cast< milliseconds >(high_resolution_clock::now().time_since_epoch());
    long long n = ms.count(); // Risky but it should work assuming no clock drift
    // Note: v is getting modified, so if retry, use original v
    Operation v = val;
    vector<string> promised = prepare_ok(n, v);
    if (promised.size() >= majority) {
        // ready to accept
        vector<string> accepted = accept_ok(v, n, promised);
        if (accepted.size() >= majority) {
            // chosen, commit
            commit(v);
        } else {
            // retry
        }
    } else {
        // retry
    }
}

template <class Operation>
vector<string> Proposer<Operation>::prepare_ok(long long n, Operation& val)
{
    cout << "Prepare request: " << n << endl;
    vector<string> promised;
    long long maxAccepted = 0;
    Operation maxAcceptedValue;
    
    for (const string& replica: replicas) {
        // send prepare request to replica
        // if promise_ok, add to promised
        string addr = replica + "/paxos/prepare/"+to_string(n);
        string res = get(addr);
        if (res != "-1") {
            stringstream s;
            s << res;
            long long accepted;
            s >> maxAccepted;
            // remove delimiter
            s.get();
            if (accepted > maxAccepted) {
                string serializedValue;
                s >> serializedValue;
                serializedValue = curlpp::unescape(serializedValue);
                stringstream instream(ios_base::in | ios_base::out);
                instream << serializedValue;
                boost::archive::text_iarchive ar(instream);
                Operation v;
                ar >> v;
                maxAcceptedValue = v;
                maxAccepted = accepted;
            }
            promised.push_back(replica);
        }
    }
    if (maxAccepted > n) {
        // a value was found
        val = maxAcceptedValue;
    }
    return promised;
}


template <class Operation>
vector<string> Proposer<Operation>::accept_ok(Operation v, long long n, const vector<string>& promised)
{
    vector<string> chosen;
    for (const string& replica: promised) {
        // send accepted request
        
        stringstream outstream(ios_base::out);
        boost::archive::text_oarchive ar(outstream);
        ar << v;
        string outstring = outstream.str();
        cout << outstring << endl;

        string addr = replica + "/paxos/accept/"+to_string(n)+"/"+curlpp::escape(outstring);
        string res = get(addr);
        cout << res << endl;
        // check if ACK or NACK
        if (res != "-1") {
            chosen.push_back(replica);
        }
    }
    return chosen;
}


template <class Operation>
void Proposer<Operation>::commit(Operation v)
{
    // send v to commit endpoint of all replicas
}
using namespace curlpp::options;

template <class Operation>
string Proposer<Operation>::get(string addr)
{
    try {
        curlpp::Cleanup myCleanup;
        std::ostringstream os;
        os << curlpp::options::Url(addr);
        return os.str();
    } catch(curlpp::RuntimeError & e) {
        std::cout << e.what() << std::endl;
        return "-1";
    } catch(curlpp::LogicError & e) {
        std::cout << e.what() << std::endl;
        return "-1";
    }
}
