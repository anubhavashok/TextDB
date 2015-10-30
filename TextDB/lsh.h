//
//  lsh.h
//  TextDB
//
//  Created by Anubhav on 10/9/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__lsh__
#define __TextDB__lsh__

#include <stdio.h>
#include <unordered_map>
#include <vector>
#include <functional>
#include <cmath>

#define CHAR_SPACE 26
#define N_GRAM_SIZE 3
#define BAND_SIZE 3


using namespace std;

class LSH
{
public:
    void add(string s, string name);
    unordered_map<string, double> test(string s, string name);
private:
    long long lookup(string s);
    vector<long long> generate_col(vector<string> shingles);
    vector<long long> generate_signature(vector<long long> col);
    unordered_map<long long, vector<string>> buckets;
    void update_lsh(vector<long long> signature, string name);
    unordered_map<string, double> check_lsh(vector<long long> signature, string name);
    vector<string> generate_shingles(string s);
    
    // ensure all hashes are modulo size of n-gram space
    vector<function<long long(long long)>> hashes {
        [](long long i) { return (3*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (34*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (54*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (3400*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (6500*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); }
    };
};

#endif /* defined(__TextDB__lsh__) */
