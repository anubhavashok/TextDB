//
//  bitwriter.h
//  TextDB
//
//  Created by Anubhav on 12/29/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#ifndef __TextDB__bitwriter__
#define __TextDB__bitwriter__

#include <stdio.h>
#include "encoder.h"
#include <iostream>
#include <vector>
#include <limits.h>

class BitWriter
{
public:
    BitWriter(Encoder::CharacterEncoding);
    void write(std::string s);
    void write(unsigned long mydata, size_t num_bits);
    void write(std::string word, size_t ncharbits);
    void saveToFile(std::string path, bool compress);
    void appendToFile(std::string path, bool compress);
    void clear();
    std::string string();
    void print();
    // void shift(int offset);
    
    Encoder* encoder;

private:
    // number of bits in one character
    const size_t charsize = CHAR_BIT;
    size_t pos;
    std::vector<char> data;
    bool end;

    void setNextBit(bool bit);
    char generateNewLastByte(char lastByte, char firstByte, int offset);
    
};

#endif /* defined(__TextDB__bitwriter__) */
