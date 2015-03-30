//
//  raft.h
//  TextDB
//
//  Created by Anubhav on 3/23/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__raft__
#define __TextDB__raft__
#include "entry.h"
#include <map>
#include <boost/optional.hpp>
#include <unordered_set>

#include <stdio.h>
using namespace std;
class Raft
{
public:
    Raft(vector<string> _replicas);
    void appendEntries(int term, int leaderId, int prevLogIndex, int prevLogTerm, int leaderCommit);
    int requestVote(int term, int candidateId, int lastLogIndex, int lasLogTerm);
    void leaderLoop();
    void startElection();
    friend class DB;
    
private:
    enum Role {
        Follower,
        Candidate,
        Leader
    };
    Role role;
    
    int id;
    
    // persistent variables
    int currentTerm;
    boost::optional<int> votedFor;
    map<int, Entry> log;
    
    // volatile variables
    int commitIndex;
    int lastApplied;
    
    // volatile leader variables
    vector<int> nextIndex;
    vector<int> matchIndex;
    
    // helpers
    string get(string addr);
    
    vector<string> replicas;
    
    // heartbeat
    boost::optional<chrono::time_point<chrono::system_clock>> lastHeartbeat;
    
    // leaders
    unordered_set<int> leaders;
    
    // leader only
    map <int, vector<Entry>> backlog;
    map <int, Entry> lastEntries;

    void addEntry(Operation op);
    void forwardRequestToLeader(const Operation& op);

    
};
#endif /* defined(__TextDB__raft__) */
