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

bool BitReader::nextBit()
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

bool BitReader::eof()
{
    return end;
}

std::bitset<18> BitReader::num2widx(size_t num)
{
    assert(sizeof(num) > 3);
    std::bitset<18> idx;
    size_t mask = 1;
    for (size_t i = 0; i < 18; i++) {
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
    int mask = 1;
    mask <<= offset;
    data[idx] = data[idx] | mask;
    pos++;
}

void BitReader::saveToFile(std::string path)
{
    assert(data.size() >= 5);
    std::cout << data.size() << std::endl;
    ofstream fout(path, ios::out | ios::binary);
    for (char c: data) {
        fout << c;
    }
    fout.close();
}


void BitReader::print()
{
    for (char c: data) {
        cout << c << endl;
    }
}

