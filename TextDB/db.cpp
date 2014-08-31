//
//  db.cpp
//  TextDB
//
//  Created by Anubhav on 8/26/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#include "db.h"
#include <cmath>
#include <fstream>
#include <cassert>
#include "bitreader.h"
#include <snappy.h>
#include <boost/filesystem/path.hpp>
#include "options.h"

namespace fs = boost::filesystem;

// index of word
// max value is ~250,000 since there are only that many english words
using widx = boost::dynamic_bitset<>;

// index of a character
// max value is 32, but only numbers up to 26 are used
// stores a character instead of 8 bits for a char
using cidx = std::bitset<5>;



std::vector<widx> DB::serializeDoc(std::vector<std::string> doc)
{
    std::vector<widx> res;
    for (std::string word: doc) {
        widx idx = addWord(word);
        res.push_back(idx);
    }
    return res;
}

// UNUSED
std::vector<widx> DB::serializeDoc(std::string path)
{
    ifstream fin(path);
    std::vector<widx> res;
    while (!fin.eof()) {
        std::string word;
        fin >> word;
        widx idx = addWord(word);
        res.push_back(idx);
    }
    return res;
}

widx DB::uint2widx(unsigned long i)
{
    assert(i < pow(2, nbits));
    widx res = boost::dynamic_bitset<>(nbits, i);
    return res;
}

widx DB::addWord(std::string word)
{
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);
    if (idx2word.size() >= pow(2, nbits)-1) {
        nbits++;
        cout << "increasing index size to " << nbits << endl;
    }
    if (word2idx.count(word)) {
        widx idx = word2idx[word];
        assert(idx.to_ulong() < pow(2, nbits));
        // assert(idx2word[idx] == word);
        return idx;
    } else {
        // careful if parallel
        // len might have changed
        size_t len = idx2word.size();
        widx idx = uint2widx((unsigned long)len);//convert len to idx;
        word2idx[word] = idx;
        idx2word[idx] = word;
        return idx;
    }
}

void DB::handleQuery(std::vector<std::string> in)
{
    std::string cmd = in[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    if (cmd == "add") {
        assert(in.size() >= 3);
        std::string name = in[1];
        
        // have either text or path to doc
        std::vector<std::string> text;
        for (size_t i = 2; i < in.size(); i++) {
            text.push_back(in[i]);
        }
        // index word and add to db
        add(name, text);
        
    } else if (cmd == "adddoc") {
        assert(in.size() >= 3);
        std::string name = in[1];
        
        // have either text or path to doc
        fs::path docPath = in[2];
        // read doc and load
        std::vector<std::string> text;
        ifstream fin(docPath.string());
        while (!fin.eof()) {
            std::string word;
            fin >> word;
            assert(word.length() < 32);
            text.push_back(word);
        }
        // index word and add to db
        add(name, text);
        
    } else if (cmd == "get") {
        assert(in.size() == 2);
        std::string name = in[1];
        // get from db
        std::vector<std::string> res;
        res = get(name);
        for (std::string word: res) {
            cout << word << " " << endl;
        }
    } else {
        cout << "Unknown query" << endl;
    }
}

// UNUSED
void DB::add(std::string name, std::string path)
{
    std::vector<widx> serializedDoc = serializeDoc(path);
    storage[name] = serializedDoc;
}

void DB::add(std::string name, std::vector<std::string> text)
{
    if (idx2word.size() >= pow(2, nbits)) {
        nbits++;
    }
    std::vector<widx> serializedDoc = serializeDoc(text);
    storage[name] = serializedDoc;
}

std::vector<std::string> DB::get(std::string name)
{
    std::vector<std::string> deserializedDoc;
    if (storage.count(name)) {
        std::vector<widx> serializedDoc = storage[name];
        for (widx idx: serializedDoc) {
            std::string word = idx2word.find(idx)->second;
            deserializedDoc.push_back(word);
        }

    }
    return deserializedDoc;
}


void DB::encodeAndSave(std::string path)
{
    // Fix this after creating BitReader
    BitReader bitReader;
    
    // set num words (max val 2^18 for possible eng words)
    // is set only once at start of the file so it can be 18
    // nbits inferred from this value
    size_t len = idx2word.size();
    cout << "len is " << len << endl;
    assert(len != 0);
    bitReader.setNextBits(len, 18);
    
    assert(idx2word.size() > 0);
    for(std::pair<widx, std::string> wordPair: idx2word) {
        std::string word = wordPair.second;
        assert(word.size() <= 20);
        // word should already be in lower case but just in case
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        
        // encode wordlen (max val = 32 chars in each word)
        size_t wordlen = word.size();
        bitReader.setNextBits(wordlen, 5);
        
        // encode word (max val = 32 but only 26 used to encode each english lower case char)
        bitReader.setNextString(word);
    }
    // save to file
    bitReader.saveToFile(path);
    
    // ENCODE KEY-DOC mapping
    BitReader docBitReader;
    
    // create file to store all docs
    for (std::pair<std::string, std::vector<widx>> doc: storage) {
        
        // output number of chars in name (max 32)
        std::string name = doc.first;
        assert(name.size() < 32);
        docBitReader.setNextBits(name.size(), 5);
        
        // output name
        docBitReader.setNextString(name);
        
        // output number of words in doc
        docBitReader.setNextBits(doc.second.size(), 32);
        
        // output nbits for each word
        // remember: nbits is inferrred from num words at the start of decoding
        for (widx idx: doc.second) {
            assert(idx.size() <= nbits);
            docBitReader.setNextBits(idx.to_ulong(), nbits);
        }
    }
    
    docBitReader.saveToFile(path + ".kvp");
}

void DB::decodeAndLoad(std::string path)
{
    BitReader bitReader(path);

    // read num words - first 18 bits
    size_t len = bitReader.getNextBits(18).to_ulong();
    assert(len > 0);
    cout << "word len: " << len << endl;
    nbits = ceil(log(len)/log(2));
    nbits = 1;
    std::vector<std::string> words;
    while (!bitReader.eof()) {
        
        // read word len
        boost::dynamic_bitset<> len = bitReader.getNextBits(5);
        
        // read word
        size_t nchars = len.to_ulong();
        std::string word = bitReader.getNextString(nchars);
        words.push_back(word);
    }

    idx2word.empty();
    word2idx.empty();
    for (size_t i = 0; i < words.size(); i ++) {
        if (i >= pow(2, nbits)) {
            nbits++;
        }
        widx idx(nbits, i);
        idx2word[idx] = words[i];
        word2idx[words[i]] = idx;
    }
    // assert(len.to_ulong() == words.size());
    
    // DECODE KEY-DOC STORAGE
    std::string docPath = path + ".kvp";
    BitReader docBitReader(docPath);
    while (!docBitReader.eof()) {
        
        // read size of name
        size_t keyLen = docBitReader.getNextBits(5).to_ulong();

        // read name
        std::string key = docBitReader.getNextString(keyLen);
        cout << "key is: " << key << endl;

        // read length of words (standard 32 bit unsigned int)
        size_t nwords = docBitReader.getNextBits(32).to_ulong();

        // read each word idx
        std::vector<widx> values;
        for (size_t n = 0; n < nwords; n++) {
            widx idx = docBitReader.getNextBits(nbits);
            // TODO: make sure each idx is stored optimally
            values.push_back(idx);
        }
        storage[key] = values;
    }
    
}

void DB::printIndex()
{
    for (std::pair<widx, std::string> wordPair: idx2word) {
        cout << wordPair.second << endl;
    }
}

void DB::saveUncompressed(std::string path)
{
    ofstream fout(path);
    fout << idx2word.size() << endl;
    for (std::pair<widx, std::string> wordPair: idx2word) {
        fout << wordPair.second.length() << "|" << wordPair.second << endl;
    }
    fout << "DOCUMENTS" << endl;
    for (std::pair<std::string, std::vector<widx>> doc: storage) {
        std::string name  = doc.first;
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
        fout << name << endl;
        for (widx idx: doc.second) {
            fout << idx2word[idx] << " ";
        }
        fout << endl;
    }
    fout.close();
}