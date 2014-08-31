//
//  bitreader.cpp
//  TextDB
//
//  Created by Anubhav on 8/27/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#include "bitreader.h"

#include <cassert>
#include <fstream>

#include <iostream>
using namespace std;

#include <snappy.h>
#include <boost/dynamic_bitset.hpp>
#include <boost/filesystem.hpp>


BitReader::BitReader(const std::vector<char>& data)
{
    for (char c: data) {
        this->data.push_back(c);
    }
    pos = 0;
    end = false;
}

BitReader::BitReader()
{
    data = std::vector<char>();
    pos = 0;
    end = false;
}

BitReader::BitReader(std::string path)
{
    if (!boost::filesystem::exists(path) || boost::filesystem::path(path).empty()) {
        cout << "error in opening file in bitreader" << endl;
        exit(-1);
    }
    ifstream fin(path, ios::in | ios::binary);
    char c;
    // read raw data as chars
    std::string compresseddatastring = "";
    while (!fin.eof()) {
        c = fin.get();
        compresseddatastring += c;
    }
    std::string datastring;
    snappy::Uncompress(compresseddatastring.data(), compresseddatastring.size(), &datastring);
    
    std::vector<char> uncompresseddata(datastring.begin(), datastring.end());
    fin.close();
    // TODO: call self
    data = uncompresseddata;
    pos = 0;
    end = false;
}


bool BitReader::getNextBit()
{
    size_t idx = pos / charsize;
    size_t offset = pos % charsize;
    
    if (idx >= data.size()) {
        end = true;
        return false;
    }
    
    char c = data[idx];
    int mask = 1;
    mask <<= offset;
    pos++;
    return mask & c;
}

boost::dynamic_bitset<> BitReader::getNextBits(size_t nbits)
{
    if (eof()) {
        // reached end of file prematurely
        cout << "Unexpected error when decoding file" << endl;
    }
    boost::dynamic_bitset<> mydata(nbits);
    for (size_t i = 0; i < nbits; i ++) {
        if (getNextBit()) {
            mydata.set(i);
        }
    }
    return mydata;
}

std::string BitReader::getNextString(size_t stringsize)
{
    return getNextString(stringsize, 5);
}

std::string BitReader::getNextString(size_t stringsize, size_t ncharbits)
{
    std::string word;
    for (size_t i = 0; i < stringsize; i ++) {
        // read ncharbit bit character
        boost::dynamic_bitset<> charbits = getNextBits(ncharbits);
        char c = charbits.to_ulong() + 'a';
        word += c;
    }
    return word;
}

bool BitReader::eof()
{
    return end;
}

boost::dynamic_bitset<> BitReader::num2widx(size_t num, size_t nbits)
{
    assert(sizeof(num) > 3);
    boost::dynamic_bitset<> idx;
    size_t mask = 1;
    for (size_t i = 0; i < nbits; i++) {
        if (mask & num) {
            idx.set(i);
        }
        mask <<= 1;
    }
    return idx;
}

void BitReader::setNextBit(bool bit)
{
    size_t idx = pos / charsize;
    size_t offset = pos % charsize;
    
    if (idx >= data.size()) {
        char newchar = 0;
        data.push_back(newchar);
    }
    int mask = bit;
    mask <<= offset;
    data[idx] = data[idx] | mask;
    pos++;
}

void BitReader::setNextBits(unsigned long mydata, size_t nbits)
{
    size_t mask = 1;
    for (size_t i = 0; i < nbits; i ++) {
        if (mask & mydata) {
            setNextBit(true);
        } else {
            setNextBit(false);
        }
        mask <<= 1;
    }
}

void BitReader::setNextString(std::string word)
{
    setNextString(word, 5);
}

void BitReader::setNextString(std::string word, size_t ncharbits)
{
    for (char c: word) {
        size_t charnum = c - 'a';
        // we can still accomodate for 6 more characters
        // to be decided
        assert(charnum < 32);
        assert(charnum < 26);
        setNextBits(charnum, ncharbits);
    }
}

void BitReader::saveToFile(std::string path)
{
    assert(data.size() >= 5);
    ofstream fout(path, ios::out | ios::binary);
    std::string datastring(data.begin(), data.end());
    std::string compresseddatastring;
    snappy::Compress(datastring.data(), datastring.size(), &compresseddatastring);
    for (char c: compresseddatastring) {
        fout.put(c);
    }
    fout.close();
}


void BitReader::print()
{
    for (char c: data) {
        cout << c << endl;
    }
}




