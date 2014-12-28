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
#include "sentiment.h"
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>

using namespace std;
namespace fs = boost::filesystem;

class DB
{
private:
    // nbits of word index
    size_t nbits = 1;
    // index of word
    // max value is ~250,000 since there are only that many english words
    using widx = boost::dynamic_bitset<>;
    
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
    
    // Sentiment Analysis
    SentimentAnalysis sentimentAnaylsis;

public:
    
    DB(fs::path data);
    
    std::vector<widx> serializeDoc(std::vector<std::string> doc);
    std::vector<widx> serializeDoc(std::string path);
    
    widx addWord(std::string word);
    void handleQuery(std::vector<std::string> in, ostream& htmlout);
    
    
    void add(std::string name, std::string path);
    void add(std::string name, std::vector<std::string> text);
    bool remove(std::string name);
    std::vector<std::string> get(std::string name);
    widx uint2widx(unsigned long i);
    std::string getSentence(std::string name, size_t start);

    
    void encodeAndSave(std::string path);
    void decodeAndLoad(std::string path);
    void saveUncompressed(std::string path);
    std::map<std::string, std::vector<std::string> > search(std::string queryString);

    // Text Mining
    double getSentimentScore(std::string name);
    
    void printIndex();
    
    static std::string urlDecode(string &SRC);
};


#endif /* defined(__TextDB__db__) */
