//
//  acceptor.h
//  TextDB
//
//  Created by Anubhav on 3/6/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__acceptor__
#define __TextDB__acceptor__

#include <stdio.h>
#include "operation.h"
#include <boost/filesystem.hpp>
#include <fstream>

namespace fs = boost::filesystem;
template <class Value>
class Acceptor
{
private:
    // to be persisted variables
    long long minProposal;
    ofstream mpstream;
public:
    Acceptor(fs::path persistence);
    bool promise(long long n);
    bool accept(long long n, Value v);
    Value acceptedValue;
    long long acceptedProposal = 0;
};

#endif /* defined(__TextDB__acceptor__) */
