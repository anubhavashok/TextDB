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
#include <set>

#define CHAR_SPACE 26
#define N_GRAM_SIZE 3
#define BAND_SIZE 2


using namespace std;

class LSH
{
public:
    void add(string s, string name);
    void remove(string s, string name);
    unordered_map<string, double> test(string s, string name);
private:
    long long lookup(string s);
    vector<long long> generate_col(vector<string> shingles);
    vector<long long> generate_signature(vector<long long> col);
    unordered_map<long long, set<string>> buckets;
    void update_lsh(vector<long long> signature, string name);
    unordered_map<string, double> check_lsh(vector<long long> signature, string name);
    vector<string> generate_shingles(string s);
    void remove_lsh(vector<long long> signature, string name);
    
    // ensure all hashes are modulo size of n-gram space
    vector<function<long long(long long)>> hashes {
        [](long long i) { return (3*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (34*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (19*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (17*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (23*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (31*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (37*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (47*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (41*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (43*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (53*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (59*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (61*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (67*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (71*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (73*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (79*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (83*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (89*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (97*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); }

    };
};

#endif /* defined(__TextDB__lsh__) */
