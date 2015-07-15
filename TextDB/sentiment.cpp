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
#include <boost/algorithm/string.hpp>
#include "db.h"

namespace fs = boost::filesystem;
using namespace std;

SentimentAnalysis::SentimentAnalysis(fs::path data)
{
    fs::path p = data / fs::path("positive.txt");
    fs::path n = data / fs::path("negative.txt");
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

void SentimentAnalysis::fileToWordMap(fs::path f, unordered_map<string, double>& v, double score)
{
    ifstream in(f.string());
    string w;
    while (in >> w) {
        boost::trim(w);
        boost::to_lower(w);
        v[w] = score;
    }
}

double SentimentAnalysis::analyse(const std::string& textstring)
{
    double score = 0;
    int count = 0;
    std::vector<std::string> text;
    boost::split(text, textstring, boost::is_any_of(DB::allowed_puncs));
    for (string w: text) {
        boost::trim(w);
        boost::to_lower(w);
        if (sentimentMap.count(w)) {
            // descriptor
            count++;
            score += sentimentMap[w];
        }
    }
    if (count == 0) {
        return 0;
    }
    return (double)score/count;
}