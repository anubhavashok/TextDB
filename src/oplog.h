//
//  oplog.h
//  TextDB
//
//  Created by Anubhav on 3/2/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__oplog__
#define __TextDB__oplog__

#include <stdio.h>
#include <vector>
#include <unordered_map>
#include <map>
#include <sstream>
#include "operation.h"
#include "acceptor.h"
#include "proposer.h"
#include <fstream>
class DB;
using namespace std;

class Oplog
{
private:
    size_t size;
    vector<Operation> operations;
    vector<pair<long long, string>> ordered_replicas;
    ofstream oplog_disk;
    long long max_n;
    vector<pair<long long, string>> pingAllReplicas(vector<string> replicas);
    shared_ptr<DB> db;
public:
    Oplog(vector<string> replicas, fs::path replpath, shared_ptr<DB> db);
    void insert(Operation op);
    // N vectors of bools where N is the number of replica sets
    Operation insert(string serialized);
    static vector<string> replicas;
    Acceptor<Operation> acceptor;
    Proposer<Operation> proposer;
    bool propose(Operation op);
    void sync();
    void commit(Operation op);
    vector<Operation> helpSync(long long n);
};

#endif /* defined(__TextDB__oplog__) */
