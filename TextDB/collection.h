//
//  collection.h
//  TextDB
//
//  Created by Anubhav on 12/28/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#ifndef __TextDB__collection__
#define __TextDB__collection__

#include <stdio.h>
#include <iostream>
#include <unordered_map>
#include <map>
#include <boost/dynamic_bitset.hpp>
#include "bitreader.h"
#include "bitwriter.h"
#include "encoder.h"
#include <boost/filesystem.hpp>
#include <fstream>
#include "cache.h"
#include <boost/any.hpp>
#include "naive_bayes_sentiment.h"
#include "lsh.h"

// Each collection has its own encoding type
// Each collection has its own storage and word table
// Enable reloading of collection from disk
// Enable creation of collection during runtime
// Create Collection

namespace fs = boost::filesystem;
using namespace std;

class Collection
{
    using widx = boost::dynamic_bitset<>;
    struct Comparer {
        bool operator()(const boost::dynamic_bitset<> &b1, const boost::dynamic_bitset<> &b2) const {
            return b1.to_ulong() < b2.to_ulong();
        }
    };
public:
    Collection(fs::path path, Encoder::CharacterEncoding _encoding);

    /* Public attributes */
    
    const std::string name;
    
    /* Persistence/Management */
    
    void loadWordIndex();
    bool load(std::string name);
    void initializeLocalitySensitiveHashing();
    boost::uintmax_t size();
    boost::uintmax_t size(std::string name);
    void kick();
    void kick(std::string name);
    
    /* API */
    
    // add
    bool add(std::string name, std::string path);
    bool add(std::string name, std::vector<std::string> text);

    // get
    std::string get(std::string name);
    std::string getSentence(std::string name, size_t start);

    // remove
    bool remove(std::string name);
    void removeWordsFromMapping(std::string name);
    
    // modify
    bool modify(string name, vector<string> doc);
    
    // sentiment
    NaiveBayesSentiment naiveBayesSentiment;
    double getSentimentScore(std::string name);
    void mark(string name, string sentimentClass);
    void train();
    pair<string, double> test(string t);
    
    // deduplication
    LSH localitySensitiveHashing;
    unordered_map<string, double> getAllDuplicates(string name);
    
    // all words
    std::vector<std::string> getWords();
    
    std::vector<std::string> listFiles();
    
    bool exists(std::string name);
    
    // cache
    bool is_cached(std::string name, std::string attr);
    boost::any get_cached(std::string name, std::string attr);
    void clear_cache(std::string name);
    void add_to_cache(std::string name, std::string attr, boost::any val);
    string get_frequency_table(string name);
    vector<vector<string>> get_all();
    vector<string> get_all_string();
    boost::uintmax_t disk_size();
    vector<string> get_vector(std::string name);
    vector<string> getInterestingDocuments(int n);
    vector<string> getRelatedDocuments(string documentName, int n);

    
private:
    
    Collection(fs::path collectionPath);
    
    // WORD INDEX
    std::map<widx, std::string, Comparer> idx2word;
    std::unordered_map<std::string, widx> word2idx;
    std::map<widx, std::vector<std::string>, Comparer> idx2docs;

    // STORAGE
    std::unordered_map<std::string, std::vector<widx>> storage;
    std::unordered_map<std::string, Cache> cache;

    // I/O
    size_t nbits = 0;
    BitReader bitReader;
    BitWriter bitWriter;
    fs::path collectionPath;
    //Encoder::CharacterEncoding encoding;
    
    // widx-string (vice-versa)
    std::vector<widx> serialize(const std::vector<std::string>& doc);
    std::vector<std::string> deserialize(const std::vector<widx>& doc);
    
    // add - helpers
    widx addWord(std::string word);
    widx uint2widx(unsigned long i);
    std::vector<std::string> find_new_words(const std::vector<std::string>& doc);
    void aow(fs::path path, const std::vector<widx>& doc);
    void aow_words(const std::vector<std::string>& new_words);

    std::string reassembleText(const std::vector<std::string>& words);
    
    std::unordered_map<std::string, std::vector<std::string> > search(std::string queryString);
};

#endif /* defined(__TextDB__collection__) */
