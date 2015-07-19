//
//  naive_bayes_sentiment.h
//  TextDB
//
//  Created by Anubhav on 7/18/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__naive_bayes_sentiment__
#define __TextDB__naive_bayes_sentiment__

#include <stdio.h>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class NaiveBayesSentiment
{
    
    class SentimentClass
    {
    public:
        enum Type {
            POS,
            NEG
        };
        unordered_map<string, int> freq;
        int n = 0;
    };

private:
public:
    unordered_map<string, int> freq;
    unordered_map<string, SentimentClass> classes;
    int n = 0;

    void train(vector<pair<string, string>> trainDocs);
    pair<string, double> test(string rawtext);
};



#endif /* defined(__TextDB__naive_bayes_sentiment__) */