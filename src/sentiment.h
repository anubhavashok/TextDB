//
//  sentiment.h
//  TextDB
//
//  Created by Anubhav on 12/5/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#ifndef __TextDB__sentiment__
#define __TextDB__sentiment__

#include <iostream>
#include <vector>
#include <unordered_map>
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;

class SentimentAnalysis
{
public:
    SentimentAnalysis(fs::path data);
    std::unordered_map<std::string, double> sentimentMap;
    
    void loadSentimentWords(fs::path p, fs::path n);
    void fileToWordMap(fs::path p, std::unordered_map<std::string, double>& m, double score);
    double analyse(const std::string& text);
};

#endif /* defined(__TextDB__sentiment__) */
