//
//  bigram_anomaly.h
//  TextDB
//
//  Created by Anubhav on 11/4/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__bigram_anomaly__
#define __TextDB__bigram_anomaly__

#include <stdio.h>
#include <unordered_map>
#include <vector>
#include <boost/filesystem.hpp>
#include <fstream>

namespace fs = boost::filesystem;

using namespace std;

class BigramAnomalyPerceptron
{
    unordered_map<string, double> weights;
    double bias = -0.25;
    
    vector<string> tokenize(const vector<string>& s, size_t token_size);
    bool predict(unordered_map<string, double> freq);
    vector<string> sanitize(string s);
public:
    BigramAnomalyPerceptron()
    {}
    BigramAnomalyPerceptron(fs::path path);
    bool is_anomaly(string s);
};

#endif /* defined(__TextDB__bigram_anomaly__) */
