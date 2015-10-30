//
//  lsh.cpp
//  TextDB
//
//  Created by Anubhav on 10/9/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "lsh.h"

#include <iostream>
#include <vector>
#include <set>
#include <unordered_map>
#include <limits>
#include <functional>
#include <cmath>
#include <boost/algorithm/string.hpp>

using namespace std;

hash<string> hash_str;

// Return index corressponding to ASCII ordered shingle
long long LSH::lookup(string s)
{
    long long r = 0;
    for (int i = 0; i < s.size(); i++) {
        // replace 'a' with first character of range of charspace
        r += pow(CHAR_SPACE, i) * (s[i] - 'a');
    }
    return r;
}

// Generate columnof indexes
vector<long long> LSH::generate_col(vector<string> shingles) {
    vector<long long> col;
    for(string shingle: shingles) {
        long long r = lookup(shingle);
        col.push_back(r+1);
    }
    
    return col;
}

// Generate signature for column
vector<long long> LSH::generate_signature(vector<long long> col)
{
    vector<long long> signature(hashes.size(), numeric_limits<long long>::max());
    for (int i = 0; i < col.size(); i++) {
        for (int j = 0; j < hashes.size(); j++) {
            signature[j] = min(signature[j], hashes[j](col[i]));
        }
    }
    return signature;
}

// Update locality sensitive hash with signature and document name
void LSH::update_lsh(vector<long long> signature, string name)
{
    size_t b = BAND_SIZE;
    for (int i = 0; i < signature.size(); i++) {
        size_t end = min(signature.size()-1, b+i);
        string band = "";
        for (int j = i; j <= end; j++) {
            band += to_string(j) + to_string(signature[j]);
        }
        // hash band
        long long hash_val = hash_str(band);
        buckets[hash_val];
        buckets[hash_val].push_back(name);
    }
}

// Check membership of document by signature
// Returns probability map of duplicates
unordered_map<string, double> LSH::check_lsh(vector<long long> signature, string name)
{
    vector<string> candidates;
    unordered_map<string, double> probabilities;
    size_t b = BAND_SIZE;
    for (int i = 0; i < signature.size(); i++) {
        size_t end = min(signature.size()-1, b+i);
        string band = "";
        for (int j = i; j <= end; j++) {
            band += to_string(j) + to_string(signature[j]);
        }
        // hash band
        long long hash_val = hash_str(band);
        if (buckets.count(hash_val)) {
            candidates.insert(candidates.end(), buckets[hash_val].begin(), buckets[hash_val].end());
        }
    }
    int r = ceil(hashes.size()/b);
    for (string n: candidates) {
        if (n != name)
            probabilities[name]++;
    }
    
    for (auto p: probabilities) {
        probabilities[p.first] /= r;
    }
    return probabilities;
}

// Generate shingles from string
vector<string> LSH::generate_shingles(string s)
{
    // Remember to convert all to lower and remove punctuation
    vector<string> shingles;
    for (int i = 0; i < s.size() - N_GRAM_SIZE + 1; i++) {
        string shingle = s.substr(i, N_GRAM_SIZE);
        shingles.push_back(shingle);
    }
    return shingles;
}

void LSH::add(string s, string name)
{
    boost::erase_all(s, " ");
    //s.erase(remove_if(s.begin(), s.end(), isspace), s.end());
    vector<string> shingles = generate_shingles(s);
    vector<long long> cols = generate_col(shingles);
    vector<long long> signature = generate_signature(cols);
    update_lsh(signature, name);
}

unordered_map<string, double> LSH::test(string s, string name)
{
    boost::erase_all(s, " ");
    //s.erase(remove_if(s.begin(), s.end(), isspace), s.end());
    vector<string> shingles = generate_shingles(s);
    vector<long long> cols = generate_col(shingles);
    vector<long long> signature = generate_signature(cols);
    return check_lsh(signature, name);
}