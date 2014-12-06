//
//  sentiment.cpp
//  TextDB
//
//  Created by Anubhav on 12/5/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#include "sentiment.h"

#include <boost/filesystem.hpp>
#include <iostream>
#include <fstream>

namespace fs = boost::filesystem;
using namespace std;

SentimentAnalysis::SentimentAnalysis(fs::path p, fs::path n)
{
    loadSentimentWords(p, n);
}


void SentimentAnalysis::loadSentimentWords(fs::path p, fs::path n)
{
    if (!fs::exists(p) || !fs::exists(n)) {
        // log error
        cerr << "Check sentiment file path" << endl;
    }
    fileToWordMap(p, sentimentMap, 1);
    fileToWordMap(n, sentimentMap, -1);
}

void SentimentAnalysis::fileToWordMap(fs::path f, map<string, double>& v, double score)
{
    ifstream in(f.string());
    string w;
    while (in >> w) {
        std::transform(w.begin(), w.end(), w.begin(), ::tolower);
        v[w] = score;
    }
}

double SentimentAnalysis::analyse(const vector<string>& text)
{
    double score = 0;
    int count = 0;
    for (string w: text) {
        if (sentimentMap.count(w)) {
            count++;
            score += sentimentMap[w];
        }
    }
    return (double)score/count;
}