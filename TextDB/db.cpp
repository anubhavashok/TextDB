//
//  db.cpp
//  TextDB
//
//  Created by Anubhav on 8/26/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#include "db.h"
#include <fstream>
#include <cassert>

// index of word
// max value is ~250,000 since there are only that many english words
using widx = std::bitset<18>;

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
    assert(i < (2^18));
    widx res(i);
    return res;
}

widx DB::addWord(std::string word)
{
    if (word2idx.count(word)) {
        widx idx = word2idx[word];
        // assert(idx < 2^18);
        // assert(idx2word[idx] == word);
        return idx;
    } else {
        // careful if parallel
        // len might have changed
        size_t len = idx2word.size();
        widx idx = uint2widx((unsigned long)len);//convert len to idx;
        word2idx[word] = idx;
        idx2word[idx] = word;
        return len;
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
        
    } else if (cmd == "get") {
        assert(in.size() == 2);
        std::string name = in[1];
        // get from db
        std::vector<std::string> res;
        res = get(name);
        for (std::string word: res) {
            cout << word << " " << endl;
        }
    }
}

void DB::add(std::string name, std::string path)
{
    std::vector<widx> serializedDoc = serializeDoc(path);
    storage[name] = serializedDoc;
}

void DB::add(std::string name, std::vector<std::string> text)
{
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