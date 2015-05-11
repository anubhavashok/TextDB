//
//  propser.h
//  TextDB
//
//  Created by Anubhav on 3/6/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__proposer__
#define __TextDB__proposer__

#include <stdio.h>
#include <vector>

using namespace std;

template <class Value>
class Proposer
{
private:
    vector<string> prepare_ok(long long n, Value& val);
    vector<string> accept_ok(Value v, long long n, const vector<string>& promised);
    // listener role
    void commit(Value v);
    vector<string> replicas;
    int majority;
    string get(string addr);

public:
    Proposer(vector<string> _replicas);
    bool propose(Value v);
};
#endif /* defined(__TextDB__proposer__) */
