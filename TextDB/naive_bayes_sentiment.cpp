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

using namespace std;

boost::regex trainRegex = boost::regex("[\\w']+|[.,!?;]");

vector<string> normalize_text(string rawtext)
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
    for (auto p: trainDocs) {
        string rawtext = p.first;
        string c = p.second;
        boost::to_lower(c);
        vector<string> text = normalize_text(rawtext);

        if (!classes.count(c)) {
            continue;
        }
        classes[c].n += 1;
        n += 1;
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
    vector<string> text = normalize_text(rawtext);
    
    string chosen;
    double max_score = -1;
    for (auto p: classes) {
        SentimentClass c = p.second;
        if (c.n <= 0) {
            continue;
        }
        double pc = log(c.n/(1.0*n));
        double score = pc;
        for (string w: text) {
            boost::to_lower(w);
            // remove all extraneous punctuation with this regex [\w']+|[.,!?;]
            double pt = log((1.0+c.freq[w])/(1.0*freq[w] + freq.size()));
            score += pt;
        }
        max_score = (max_score == -1)? score: max_score;
        if (score > max_score) {
            max_score = score;
            chosen = p.first;
        }
    }
    return make_pair(chosen, max_score);
}