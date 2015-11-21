//
//  tests.h
//  TextDB
//
//  Created by Anubhav on 11/3/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__tests__
#define __TextDB__tests__

#include <stdio.h>
#include "gtest/gtest.h"
#include <iostream>
#include <unordered_map>
#include "db.h"
#include "sentiment.h"
#include "collection.h"
#include "encoder.h"
#include "LRU.h"
#include "entry.h"
#include "raft.h"
#include "bigram_anomaly.h"

using namespace std;

int run_tests(int* argc, char** argv);

#endif /* defined(__TextDB__tests__) */
