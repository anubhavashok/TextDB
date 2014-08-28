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
#include "bitreader.h"

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

// UNUSED
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


void DB::encodeAndSave(std::string path)
{
    // Fix this after creating BitReader
    BitReader bitReader;
    
    // set word len
    size_t mask = 1;
    size_t len = idx2word.size();
    for (size_t i = 0; i < 18; i ++) {
        if (mask & len) {
            bitReader.setNextBit(true);
        } else {
            bitReader.setNextBit(false);
        }
        mask <<= 1;
    }
    assert(idx2word.size() > 0);
    for(std::pair<widx, std::string> wordPair: idx2word) {
        std::string word = wordPair.second;
        assert(word.size() <= 20);
        // word should already be in lower case but just in case
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        
        // encode wordlen
        size_t wordlen = word.size();
        size_t mask = 1;
        for (size_t i = 0; i < 5; i++) {
            if (mask & wordlen) {
                bitReader.setNextBit(true);
            } else {
                bitReader.setNextBit(false);
            }
            mask <<= 1;
        }
        
        // encode word
        for (char c: word) {
            size_t charnum = c - 'a';
            // we can still accomodate for 6 more characters
            // to be decided
            assert(charnum < 32);
            assert(charnum < 26);
            size_t mask = 1;
            for (size_t i = 0; i < 5; i ++) {
                if (mask & charnum) {
                    bitReader.setNextBit(true);
                } else {
                    bitReader.setNextBit(false);
                }
                mask <<= 1;
            }
        }
    }
    // save to file
    bitReader.saveToFile(path);
}

void DB::decodeAndLoad(std::string path)
{
    std::vector<char> data;
    ifstream fin(path, ios::in | ios::binary);
    char c;
    // read raw data as chars
    while (fin.get(c)) {
        data.push_back(c);
    }

    fin.close();
    BitReader bitReader(data);

    // read num words - first 18 bits
    std::bitset<18> len;
    for (size_t i = 0; i < 18; i ++) {
        if (bitReader.eof()) {
            // something went wrong
            cout << "Unexpected error when decoding file" << endl;
        }
        bool nextBit = bitReader.nextBit();
        if (nextBit) {
            len.set(i);
        }
    }
    std::vector<std::string> words;
    while (!bitReader.eof()) {
        // read word len
        std::bitset<5> len;
        for (size_t i = 0; i < 5; i ++) {
            if (bitReader.nextBit()) {
                len.set(i);
            }
        }
        // read word
        size_t nchars = len.to_ulong();
        std::string word;
        for (size_t i = 0; i < nchars; i ++) {
            // read 5 bit character
            std::bitset<5> charbits;
            for (size_t j = 0; j < 5; j++) {
                if (bitReader.nextBit()) {
                    charbits.set(j);
                }
            }
            char c = charbits.to_ulong() + 'a';
            word += c;
        }
        words.push_back(word);
    }
    idx2word.empty();
    word2idx.empty();
    for (size_t i = 0; i < words.size(); i ++) {
        widx idx = BitReader::num2widx(i);
        idx2word[idx] = words[i];
        word2idx[words[i]] = idx;
    }
    //assert(len.to_ulong() == words.size());
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
    fout.close();
}