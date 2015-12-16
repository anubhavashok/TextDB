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
        [](long long i) { return (54*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (3400*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (500*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (6512*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (6523*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (6534*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (6545*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (6556*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (6567*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (6578*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (6589*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (6511*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (1200*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (1212*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (1234*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (1245*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (1256*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); },
        [](long long i) { return (1267*i + 1)% (int)pow(CHAR_SPACE, N_GRAM_SIZE); }

    };
};

#endif /* defined(__TextDB__lsh__) */
