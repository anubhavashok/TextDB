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
    const std::string name;
    
    std::map<widx, std::string, Comparer> idx2word;
    std::map<std::string, widx> word2idx;
    
    Collection(fs::path path, Encoder::CharacterEncoding _encoding);
    void loadWordIndex();
    void load(std::string name);
    
    
    size_t size();
    void kick();
    
    bool add(std::string name, std::string path);
    bool add(std::string name, std::vector<std::string> text);

    
private:
    size_t nbits;
    BitReader bitReader;
    BitWriter bitWriter;
    Collection(fs::path collectionPath);
    fs::path collectionPath;
    //Encoder::CharacterEncoding encoding;
    
    std::map<std::string, std::vector<widx>> storage;
    
    std::vector<widx> serializeDoc(std::vector<std::string> doc);
    std::vector<widx> serializeDoc(std::string path);
    
    widx addWord(std::string word);
    void handleQuery(std::vector<std::string> in, std::ostream& htmlout);
    
    
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
    
    static std::string urlDecode(std::string &SRC);
    
    std::vector<std::string> find_new_words(std::vector<std::string> doc);
    void aow(fs::path path, std::vector<widx> doc);
    void aow_words(std::vector<std::string> new_words);
};

#endif /* defined(__TextDB__collection__) */
