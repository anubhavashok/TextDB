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
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>

// Static variables
unordered_map<int, map<Operation, bool>> Oplog::replicated;
unordered_map<int, string> Oplog::ip_table;


void Oplog::insert(Operation op)
{
    operations.push_back(op);
    // async replicate
    std::async(std::launch::async, Oplog::replicate, op);
}



void Oplog::replicate(Operation op)
{
    // check that op isn't received by recepeitne
    for (auto& m: Oplog::replicated) {
        if (!m.second.count(op)) {
            // send op, wait for reciept, timeout after 14 seconds
            stringstream outstream(ios_base::binary | ios_base::out);
            boost::archive::binary_oarchive ar(outstream);
            ar << op;
            string outstring = outstream.str();
            // send string to replica ip_table[m.first]
            m.second[op] = true;
        }
    }
}


Operation Oplog::insert(string serialized)
{
    cout << "Deserializing operation" << endl;
    stringstream instream(ios_base::binary | ios_base::in);
    boost::archive::binary_iarchive ar(instream);
    Operation op;
    ar >> op;
    insert(op);
    return op;
}

void Oplog::updateTable(int replicaID, map<Operation, bool> table)
{
    for (auto& p: table) {
        Operation op = p.first;
        if (!Oplog::replicated[replicaID].count(op)) {
            Oplog::replicated[replicaID][op] = true;
        }
    }
}

Oplog::Oplog(vector<string> replicas)
{
    int i = 0;
    for (string replica: replicas) {
        
        Oplog::ip_table[i] = replica;
        Oplog::replicated[i];
        i++;
    }
}