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
#include <map>
#include <boost/dynamic_bitset.hpp>
#include "bitreader.h"
#include "bitwriter.h"
#include "encoder.h"
#include <boost/filesystem.hpp>
#include <fstream>

// Each collection has its own encoding type
// Each collection has its own storage and word table
// Enable reloading of collection from disk
// Enable creation of collection during runtime
// Create Collection

namespace fs = boost::filesystem;

class Collection
{
    using widx = boost::dynamic_bitset<>;
    struct Comparer {
        bool operator() (const boost::dynamic_bitset<> &b1, const boost::dynamic_bitset<> &b2) const {
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
    size_t size();
    size_t size(std::string name);
    void kick();
    void kick(std::string name);
    
    /* API */
    
    // add
    bool add(std::string name, std::string path);
    bool add(std::string name, std::vector<std::string> text);

    // get
    std::vector<std::string> get(std::string name);
    std::string getSentence(std::string name, size_t start);

    // remove
    bool remove(std::string name);
    
    // sentiment
    double getSentimentScore(std::string name);
    
    // all words
    std::vector<std::string> getWords();
    
    std::vector<std::string> listFiles();
    
    bool exists(std::string name);
    
private:
    
    Collection(fs::path collectionPath);
    
    // WORD INDEX
    std::map<widx, std::string, Comparer> idx2word;
    std::map<std::string, widx> word2idx;

    // STORAGE
    std::map<std::string, std::vector<widx>> storage;

    // I/O
    size_t nbits;
    BitReader bitReader;
    BitWriter bitWriter;
    fs::path collectionPath;
    //Encoder::CharacterEncoding encoding;
    
    // widx-string (vice-versa)
    std::vector<widx> serialize(std::vector<std::string> doc);
    std::vector<std::string> deserialize(std::vector<widx> doc);
    
    // add - helpers
    widx addWord(std::string word);
    widx uint2widx(unsigned long i);
    std::vector<std::string> find_new_words(std::vector<std::string> doc);
    void aow(fs::path path, std::vector<widx> doc);
    void aow_words(std::vector<std::string> new_words);

    
    
    std::map<std::string, std::vector<std::string> > search(std::string queryString);
};

#endif /* defined(__TextDB__collection__) */
