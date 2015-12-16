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
        r += pow(CHAR_SPACE, i) * ((long long)s[i] - (long long)'a');
    }
    return r;
}

// Generate column of indexes
vector<long long> LSH::generate_col(vector<string> shingles) {
    vector<long long> col;
    for(string shingle: shingles) {
        long long r = lookup(shingle);
        col.push_back(r);
    }
    
    return col;
}

// Generate signature for column
vector<long long> LSH::generate_signature(vector<long long> col)
{
    vector<long long> signature(hashes.size(), numeric_limits<long long>::max());
    for (int i = 0; i < col.size(); i++) {
        for (int j = 0; j < hashes.size(); j++) {
            signature[j] = min(signature[j], hashes[j](col[i] + 1));
        }
    }
    return signature;
}

// Update locality sensitive hash with signature and document name
void LSH::update_lsh(vector<long long> signature, string name)
{
    for (int i = 0; i < signature.size(); i += BAND_SIZE) {
        size_t end = min(signature.size()-1, (size_t)BAND_SIZE+i);
        string band = "";
        for (int j = i; j <= end; j++) {
            band += to_string(j) + to_string(signature[j]);
        }
        // hash band
        long long hash_val = hash_str(band);
        buckets[hash_val];
        buckets[hash_val].insert(name);
    }
}



// Check membership of document by signature
// Returns probability map of duplicates
unordered_map<string, double> LSH::check_lsh(vector<long long> signature, string name)
{
    vector<string> candidates;
    unordered_map<string, double> probabilities;
    for (int i = 0; i < signature.size(); i += BAND_SIZE) {
        size_t end = min(signature.size()-1, (size_t)BAND_SIZE+i);
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
    assert(BAND_SIZE <= signature.size());
    int r = ceil(signature.size()/BAND_SIZE);
    for (string n: candidates) {
        if (n != name) {
            probabilities[n] += 1;
        }
    }
    
    for (auto p: probabilities) {
        cout << "probabilities count: " << p.second << endl;
        probabilities[p.first] = p.second/r;
        cout << "normalized: " << probabilities[p.first] << endl;
    }
    return probabilities;
}

// Generate shingles from string
vector<string> LSH::generate_shingles(string raw)
{
    // Remember to convert all to lower and remove punctuation
    boost::to_lower(raw);
    string s = "";
    for (char c: raw) {
        if (isalpha(c)) {
            s += c;
        }
    }
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

void LSH::remove_lsh(vector<long long> signature, string name)
{
    for (int i = 0; i < signature.size(); i += BAND_SIZE) {
        size_t end = min(signature.size()-1, (size_t)BAND_SIZE+i);
        string band = "";
        for (int j = i; j <= end; j++) {
            band += to_string(j) + to_string(signature[j]);
        }
        // hash band
        long long hash_val = hash_str(band);
        if (buckets.count(hash_val)) {
            buckets[hash_val].erase(name);
        }
    }
}

void LSH::remove(string s, string name)
{
    // find which buckets name is currently in, remove all occurences
    boost::erase_all(s, " ");
    //s.erase(remove_if(s.begin(), s.end(), isspace), s.end());
    vector<string> shingles = generate_shingles(s);
    vector<long long> cols = generate_col(shingles);
    vector<long long> signature = generate_signature(cols);
    remove_lsh(signature, name);
}