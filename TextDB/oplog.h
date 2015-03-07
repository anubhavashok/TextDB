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

using namespace std;

class Oplog
{
    size_t size;
    vector<Operation> operations;
public:
    Oplog(vector<string> replicas);
    void insert(Operation op);
    vector<Operation> get(int n);
    static void replicate(Operation op);
    // N vectors of bools where N is the number of replica sets
    static unordered_map<int, map<Operation, bool>> replicated;
    static unordered_map<int, string> ip_table;
    Operation insert(string serialized);
    void updateTable(int replicaID, map<Operation, bool> table);
    
};

#endif /* defined(__TextDB__oplog__) */
