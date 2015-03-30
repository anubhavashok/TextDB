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


using namespace std;

Raft::Raft(vector<string> _replicas)
: role(Role::Follower), replicas(_replicas), lastHeartbeat(boost::none)
{
    cout << "RAFT started" << endl;
}


void Raft::appendEntries(int term, int leaderId, int prevLogIndex, int prevLogTerm, int leaderCommit)
{
    int received = 1;
    for (int i = 0; i < replicas.size(); i++) {
        string r = replicas[i];
        vector<Entry> entries = backlog[i];
        stringstream outstream(ios_base::out);
        boost::archive::text_oarchive ar(outstream);
        Entries e(entries);
        ar << e;
        string entriesString = outstream.str();

        
        string req = (boost::format("/appendentries/%s/%s/%s/%s/%s/%s/") % term % leaderId % prevLogIndex % prevLogTerm % curlpp::escape(entriesString) % leaderCommit).str();
        
        string output = get(r + req);
        if (output == "OK") {
            received++;
        }
    }
    if (received > replicas.size()/2) {
        // ready to commit
    }
}
int Raft::requestVote(int term, int candidateId, int lastLogIndex, int lastLogTerm)
{
    string req = (boost::format("/requestvote/%s/%s/%s/%s/") % term % candidateId % lastLogIndex % lastLogTerm).str();
    int numVotes = 0;
    for (const string& r: replicas) {
        string output = get(r + req);
        cout << output << endl;
        if (output == "OK") numVotes++;
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
        appendEntries(currentTerm, id, commitIndex, currentTerm, lastApplied);
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
    int t = rand() % 10000 + 10000;
    cout << "electionTimeout: " << t << endl;
    this_thread::sleep_for(chrono::milliseconds(t));
    role = Raft::Candidate;
    votedFor = id;
    bool undecided = true;
    // Try election until voted as leader or receives an appendEntries RPC
    do {
        currentTerm++;
        // check for election timeout, in the case of split votes
        int numVotes = requestVote(currentTerm, id, commitIndex, currentTerm-1);
        if (numVotes >= replicas.size()/2 + 1) {
            role = Raft::Leader;
            
            undecided = false;
        } else {
            this_thread::sleep_for(chrono::milliseconds(rand() % 200 + 200));
        }
    } while ((undecided) && (role == Raft::Candidate));
    
    // reset votedFor for next election
    votedFor = boost::none;
}

void Raft::addEntry(Operation op)
{
    if (role == Raft::Leader) {
        Entry e;
        e.op = op;
        e.index = commitIndex + 1;
        commitIndex++;
        e.term = currentTerm;
        // commit,
        for (int i = 0; i < replicas.size(); i++) {
            backlog[i].push_back(e);
        }
        
    } else {
        // forward request to leader
        forwardRequestToLeader(op);
    }
}

void Raft::forwardRequestToLeader(const Operation& op)
{
    for (int i: leaders) {
        string addr = replicas[i];
        // send
        
    }
}