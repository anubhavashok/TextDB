//
//  bigram_anomaly.cpp
//  TextDB
//
//  Created by Anubhav on 11/4/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "bigram_anomaly.h"
#include <boost/algorithm/string.hpp>
#include <iostream>

BigramAnomalyPerceptron::BigramAnomalyPerceptron(fs::path path)
{
    ifstream in((path / "bg_frequencies.txt").string());
    while (!in.eof()) {
        string bg;
        int f = 0;
        in >> bg >> f;
        boost::to_lower(bg);
        weights[bg] = f;
    }
    
    double top = 0;
    double c = weights["hg"];
    for (auto p: weights) {
        // hand picked zero point
        weights[p.first] -= c;
        top = max(top, weights[p.first]);
    }
    for (auto p: weights) {
        if (p.second > 0) {
            weights[p.first] = p.second/top;
        } else {
            weights[p.first] = 1000*p.second/top;
        }
    }
}

vector<string> BigramAnomalyPerceptron::sanitize(string s)
{
    s.erase(remove_if(s.begin(), s.end(), [](char c) {
        return ispunct(c);
    }), s.end());
    
    // Normalize whitespace
    replace(s.begin(), s.end(), '\n', ' ');
    replace(s.begin(), s.end(), '\t', ' ');
    replace(s.begin(), s.end(), '\r', ' ');
    
    boost::to_lower(s);
    
    vector<string> words;
    boost::split(words, s, boost::is_any_of(" "));
    
    return words;
}

vector<string> BigramAnomalyPerceptron::tokenize(const vector<string>& s, size_t token_size)
{
    vector<string> tokens;
    for (string w: s) {
        if (w.empty()) {
            continue;
        }
        // TODO: What about the edge case of single character words? (We still want to count them)
        for (int i = 0; i < w.size() - token_size+1; i++) {
            tokens.push_back(w.substr(i, token_size));
        }
    }
    return tokens;
}

bool BigramAnomalyPerceptron::predict(unordered_map<string, double> freq)
{
    double activation = 0;
    long long n = 0;
    for (auto p: freq) {
        string k = p.first;
        if (!weights.count(k)) {
            continue;
        }
        n += p.second;
        activation += p.second * weights[k];
    }
    activation += bias;
    activation /= n;
    cout << "ACTIVATION: " << activation << endl;
    return activation > 0;
}

void print_words(const vector<string>& words)
{
    for (string w: words) {
        cout << w << " ";
    }
    cout << endl;
}

bool BigramAnomalyPerceptron::is_anomaly(string s)
{
    vector<string> words = sanitize(s);
    vector<string> bigrams = tokenize(words, 2);
    unordered_map<string, double> freq;
    for (string b: bigrams) {
        // Create int obj = 0 at b
        freq[b];
        // Increment count
        freq[b]++;
    }
    return !predict(freq);
}