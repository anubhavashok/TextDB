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
#include <unordered_map>
#include <vector>
#include <boost/dynamic_bitset.hpp>
#include "sentiment.h"
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include "collection.h"
#include "encoder.h"
#include "LRU.h"


using namespace std;
namespace fs = boost::filesystem;

class DB
{
private:
    /* CONSTANTS 
       memory_limit
            max memory usage of TextDB in bytes
            default 2GB
       memory_epsilon 
            memory buffer before caching
            default 10MB
    */
    size_t memory_limit = 2000000000;
    size_t memory_epsilon = 10000;
    
    /* OBJECTS */
    std::unordered_map<std::string, std::function<void(DB* db, ostream& htmlout, const std::vector<std::string>& args)>> queryFunctions;
    SentimentAnalysis sentimentAnalysis;
    LRU lru;


    // index of word
    // max value is ~250,000 since there are only that many english words
    using widx = boost::dynamic_bitset<>;
    
    struct Comparer {
        bool operator() (const boost::dynamic_bitset<> &b1, const boost::dynamic_bitset<> &b2) const {
            return b1.to_ulong() < b2.to_ulong();
        }
    };
    
    void init_query_operations();
    
    std::pair<std::string, std::string> parseCollectionsDirName(std::string);
    int get_occupied_space();
    std::string reassembleText(const std::vector<std::string>& words);

public:
    const static std::string allowed_puncs;

    DB(fs::path data);
    fs::path datapath;
    std::unordered_map<std::string, Collection*> collections;

    
    void handleQuery(std::vector<std::string> in, ostream& htmlout);
    
    // Q
    bool add(std::string collection, std::string name, std::string path);
    bool add(std::string collection, std::string name, const std::vector<std::string>& text);
    bool remove(std::string collection, std::string name);
    std::string get(std::string collection, std::string name);
    void drop(std::string collection);
    widx uint2widx(unsigned long i);
    std::string getSentence(std::string collection, std::string name, size_t start);

    //std::unordered_map<std::string, const std::vector<std::string>& > search(std::string queryString);

    // Text Mining
    double getSentimentScore(std::string collection, std::string name);
    
    void printIndex();
    
    static std::string urlDecode(const string &SRC);
    
    void createCollection(std::string _name, Encoder::CharacterEncoding _encoding);
    std::vector<std::string> listCollections();
};


#endif /* defined(__TextDB__db__) */
