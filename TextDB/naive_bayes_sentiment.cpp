//
//  naive_bayes_sentiment.cpp
//  TextDB
//
//  Created by Anubhav on 7/18/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "naive_bayes_sentiment.h"
#include <vector>
#include <cmath>
#include <boost/algorithm/string.hpp>
#include <boost/regex.hpp>
#include <iostream>

using namespace std;

boost::regex trainRegex = boost::regex("[\\w']+|[.,!?;]");

static vector<string> normalize_text(string rawtext)
{
    vector<string> text;
    boost::sregex_token_iterator iter(rawtext.begin(), rawtext.end(), trainRegex, 0);
    boost::sregex_token_iterator end;
    
    for( ; iter != end; ++iter ) {
        string w = *iter;
        boost::to_lower(w);
        text.push_back(w);
    }
    return text;
}

NaiveBayesSentiment::NaiveBayesSentiment()
{
    classes["pos"] = SentimentClass(SentimentClass::Type::POS);
    classes["neg"] = SentimentClass(SentimentClass::Type::NEG);

}

void NaiveBayesSentiment::train(vector<pair<string, string>> trainDocs)
{
    cout << "Training marked docs" << endl;
    for (auto p: trainDocs) {
        string rawtext = p.second;
        string c = p.first;
        vector<string> text = normalize_text(rawtext);
        cout << "trainClass: " << c << endl;
        if (!this->classes.count(c)) {
            continue;
        }
        cout << "found " << c << endl;
        this->classes[c].n += 1;
        cout << "c.n " << this->classes[c].n << endl;
        n += 1;
        cout << "total: " << n << endl;
        for (string w: text) {
            boost::to_lower(w);
            // remove all extraneous punctuation with this regex [\w']+|[.,!?;]
            freq[w] += 1;
            classes[c].freq[w] += 1;
        }
    }
}


pair<string, double> NaiveBayesSentiment::test(string rawtext)
{
    cout << "total: " << n << endl;
    vector<string> text = normalize_text(rawtext);
    cout << "text: " << rawtext << endl;
    string chosen;
    double max_score = numeric_limits<int>::min();
    cout << "classes size: " << classes.size() << endl;
    for (auto p: this->classes) {
        cout << "in loop: " << endl;
        SentimentClass& c = p.second;
        cout << "p.first: " << p.first << endl;
        cout << "c.n: " << c.n << endl;
        if (c.n <= 0) {
            continue;
        }
        double pc = c.n/(1.0*n);
        cout << "pc: " << pc << endl;
        double score = pc;
        for (string w: text) {
            boost::to_lower(w);
            // remove all extraneous punctuation with this regex [\w']+|[.,!?;]
            double pt = (1.0+c.freq[w])/(1.0*freq[w] + freq.size());
            score += pt;
        }
        cout << "score: " << score << endl;
        if (score > max_score) {
            max_score = score;
            chosen = p.first;
        }
    }
    cout << "chosen: " << chosen << endl;
    return make_pair(chosen, max_score);
}