//
//  raft.cpp
//  TextDB
//
//  Created by Anubhav on 3/23/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "raft.h"
#include <boost/format.hpp>
#include <cURLpp.hpp>
#include "entry.h"
#include <boost/serialization/serialization.hpp>
#include <sstream>
#include "entry.h"
#include <boost/archive/text_oarchive.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Easy.hpp>
#include <cURLpp.hpp>
#include <chrono>
#include <thread>
#include <boost/optional.hpp>
#include <boost/algorithm/string.hpp>
#include "db.h"

using namespace std;

Raft::Raft(vector<string> _replicas, int candidateId, shared_ptr<DB> db)
: role(Role::Follower), lastHeartbeat(boost::none), candidateId(candidateId), db(db), nextIndex(0)
{
    for (int i = 0; i < _replicas.size(); i++) {
        vector<string> args;
        boost::split(args, _replicas[i], boost::is_any_of(","));
        int rid = stoi(args[1]);
        replicas.push_back(args[0]);
        replicaId.push_back(rid);
        replicaIdReverseMap[rid] = i;
    }
    cout << "RAFT started" << endl;
}


void Raft::appendEntries(int term, int leaderId, int prevLogIndex, int prevLogTerm, int leaderCommit)
{
    int received = 1;
    int lastCommitted = commitIndex;
    for (int i = 0; i < replicas.size(); i++) {
        string r = replicas[i];
        vector<Entry> entries = backlog[i];
        stringstream outstream(ios_base::out);
        boost::archive::text_oarchive ar(outstream);
        Entries e(entries);
        ar << e;
        string entriesString = outstream.str();

        int prevLogIndex = nextIndex[i] - 1;
        int prevLogTerm = (prevLogIndex < 0? -1: log[prevLogIndex].term);
        string req = (boost::format("/appendentries/%s/%s/%s/%s/%s/%s/") % term % leaderId % prevLogIndex % prevLogTerm % curlpp::escape(entriesString) % leaderCommit).str();
        
        string output = get(r + req);
        cout << output << endl;
        if (output == "OK") {
            if (!backlog[i].empty()) {
                lastCommitted = max(lastCommitted, backlog[i].back().index);
            }
            nextIndex[i] = lastCommitted + 1;
            backlog[i] = vector<Entry>();
            received++;
        } else {
            nextIndex[i]--;
            assert(log.count(nextIndex[i]-1));
            backlog[i].push_back(log[nextIndex[i]-1]);
        }
    }
    if (received > replicas.size()/2) {
        // ready to commit all entries from commitIndex up to lastApplied
        for (int i = commitIndex; i < lastCommitted; i++) {
            for (auto p: log) {
                cout << p.first << ": " << p.second.op.cmd << endl;
            }
            if (log.count(i)) {
                cout << "Size of log: " << log.size() << endl;
                cout << "Applying command" << i << ": " << log[i].op.cmd << endl;
                db->commit(log[i].op);
            } else {
                cout << "Why is there no entry in i: " << i << "commitIndex: " << commitIndex << "lastCommitted: "  << lastCommitted << endl;
            }
        }
        commitIndex = lastCommitted;
    }
}
int Raft::requestVote(int term, int candidateId, int lastLogIndex, int lastLogTerm)
{
    string req = (boost::format("/requestvote/%s/%s/%s/%s/") % term % candidateId % lastLogIndex % lastLogTerm).str();
    int numVotes = 0;
    for (const string& r: replicas) {
        string output = get(r + req);
        cout << "replica at: " << r << endl;
        
        if ((output != "ERR") && (output != "-1")) {
            vector<string> args;
            boost::split(args, output, boost::is_any_of("/"));
            int term = stoi(args[0]);
            int voteGranted = stoi(args[1]);
            if (term > currentTerm) {
                currentTerm = term;
                role = Raft::Follower;
                return 0;
            }
            if (voteGranted > 0) {
                numVotes++;
            }
        }
    }
    cout << numVotes << endl;
    return numVotes;
}

string Raft::get(string addr)
{
    try {
        curlpp::Cleanup myCleanup;
        std::ostringstream os;
        os << curlpp::options::Url(addr);
        return os.str();
    } catch(curlpp::RuntimeError & e) {
        std::cout << e.what() << std::endl;
        return "ERR";
    } catch(curlpp::LogicError & e) {
        std::cout << e.what() << std::endl;
        return "ERR";
    }
}

void Raft::leaderLoop()
{
    if (role == Raft::Leader) {
        cout << "I am the leader sending heartbeat" << endl;
        appendEntries(currentTerm, candidateId, -1, -1, commitIndex);
        this_thread::sleep_for(std::chrono::milliseconds(2000));
    } else if (role == Raft::Follower) {
        if ((lastHeartbeat == boost::none) || ((std::chrono::system_clock::now() - *lastHeartbeat) > std::chrono::milliseconds(4000))) {
            cout << "Starting election" << endl;
            // start new election
            startElection();
        } else {
            this_thread::sleep_for(std::chrono::milliseconds(4000));
        }
    } else {
        cout << "still candidate" << endl;
        this_thread::sleep_for(chrono::milliseconds(2000));
    }
}

// http://www.concentric.net/~Ttwang/tech/inthash.htm
unsigned long mix(unsigned long a, unsigned long b, unsigned long c)
{
    a=a-b;  a=a-c;  a=a^(c >> 13);
    b=b-c;  b=b-a;  b=b^(a << 8);
    c=c-a;  c=c-b;  c=c^(b >> 13);
    a=a-b;  a=a-c;  a=a^(c >> 12);
    b=b-c;  b=b-a;  b=b^(a << 16);
    c=c-a;  c=c-b;  c=c^(b >> 5);
    a=a-b;  a=a-c;  a=a^(c >> 3);
    b=b-c;  b=b-a;  b=b^(a << 10);
    c=c-a;  c=c-b;  c=c^(b >> 15);
    return c;
}
void Raft::startElection()
{
    leaders.clear();
    unsigned long seed = mix(clock(), time(NULL), getpid());
    srand((int)seed);
    int t = rand() % 2500 + 2500;
    cout << "electionTimeout: " << t << endl;
    this_thread::sleep_for(chrono::milliseconds(t));
    cout << "FINISHED TIMEOUT" << endl;
    cout << "votedFor: " << votedFor.is_initialized() << endl;
    if (votedFor == boost::none) {
        // only proceed with election if didn't vote for anyone during initial timeout
        role = Raft::Candidate;
        votedFor = candidateId;
        bool undecided = true;
        // Try election until voted as leader or receives an appendEntries RPC
        do {
            currentTerm++;
            // check for election timeout, in the case of split votes
            int numVotes = requestVote(currentTerm, candidateId, commitIndex, currentTerm-1);
            if (numVotes >= replicas.size()/2 + 1) {
                role = Raft::Leader;
                // set all nextIndexes to leaderCommit + 1;
                nextIndex = {commitIndex + 1};
                undecided = false;
            } else {
                this_thread::sleep_for(chrono::milliseconds(rand() % 200 + 200));
            }
        } while ((undecided) && (role == Raft::Candidate));
        
        // reset votedFor for next election
    }
    votedFor = boost::none;
}

void Raft::addEntry(Operation op)
{
    if (role == Raft::Leader) {
        Entry e;
        e.op = op;
        lastApplied++;
        e.index = lastApplied;
        e.term = currentTerm;
        for (int i = 0; i < replicas.size(); i++) {
            backlog[i].push_back(e);
        }
        log[lastApplied] = e;
    } else {
        // forward request to leader
        forwardRequestToLeader(op);
        for(int i: leaders) {
            cout << "leaderID: " << i << endl;
        }
    }
}

void Raft::forwardRequestToLeader(const Operation& op)
{
    // note we only need to froward write requests
    // read requests can be served locally
    cout << "forwarding request to leader" << endl;
    for (int i: leaders) {
        string addr = replicas[replicaIdReverseMap[i]];
        // send
        string cmd = op.cmd;
        string query = "";
        for (string arg: op.args) {
            query += arg + "/";
        }
        string url = addr + "/" + cmd + "/" + query;
        get(url);
    }
}