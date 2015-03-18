//
//  operation.h
//  TextDB
//
//  Created by Anubhav on 3/2/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__operation__
#define __TextDB__operation__

#include <stdio.h>
#include <iostream>
#include <vector>
#include <boost/archive/binary_iarchive.hpp>
#include <boost/archive/binary_oarchive.hpp>
#include <boost/serialization/vector.hpp>


using namespace std;

class Operation
{
public:
    // consensus ordering number
    long long n;
    string cmd;
    vector<string> args;
    bool operator<(const Operation& op1) const
    {
        return cmd < op1.cmd;
    }
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & n;
        ar & cmd;
        ar & args;
    }
};

class OperationContainer
{
public:
    vector<Operation> operations;
private:
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & operations;
    }

};

#endif /* defined(__TextDB__operation__) */
