//
//  db.h
//  TextDB
//
//  Created by Anubhav on 8/26/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#ifndef __TextDB__db__
#define __TextDB__db__

#include <iostream>
#include <map>
#include <vector>
#include <boost/dynamic_bitset.hpp>

using namespace std;

class DB
{
private:
    // nbits of word index
    size_t nbits = 1;
    // index of word
    // max value is ~250,000 since there are only that many english words
    using widx = boost::dynamic_bitset<>;
    
    // index of a character
    // max value is 32, but only numbers up to 26 are used
    // stores a character instead of 8 bits for a char
    using cidx = std::bitset<5>;
    
    // maps index to word
    struct Comparer {
        bool operator() (const boost::dynamic_bitset<> &b1, const boost::dynamic_bitset<> &b2) const {
            return b1.to_ulong() < b2.to_ulong();
        }
    };
    std::map<widx, std::string, Comparer> idx2word;
    
    // maps word to index
    std::map<std::string, widx> word2idx;
    
    // stores text data
    // by mapping key name to vectors of widxs representing docs
    std::map<std::string, std::vector<widx>> storage;
public:
    std::vector<widx> serializeDoc(std::vector<std::string> doc);
    std::vector<widx> serializeDoc(std::string path);
    
    widx addWord(std::string word);
    void handleQuery(std::vector<std::string> in);
    
    
    void add(std::string name, std::string path);
    void add(std::string name, std::vector<std::string> text);
    std::vector<std::string> get(std::string name);
    widx uint2widx(unsigned long i);
    
    void encodeAndSave(std::string path);
    void decodeAndLoad(std::string path);
    void saveUncompressed(std::string path);
    
    void printIndex();
};


#endif /* defined(__TextDB__db__) */
