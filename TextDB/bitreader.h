//
//  bitreader.h
//  TextDB
//
//  Created by Anubhav on 8/27/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#ifndef __TextDB__bitreader__
#define __TextDB__bitreader__

#include <iostream>
#include <vector>
#include <boost/dynamic_bitset.hpp>
#include "encoder.h"

class BitReader
{
private:
    // number of bits in one character
    const size_t charsize = CHAR_BIT;
    size_t pos;
    std::vector<char> data;
    bool end;
public:
    // read
    // UNUSED
    BitReader(const std::vector<char>& data, Encoder::CharacterEncoding _encoding);
    BitReader(Encoder::CharacterEncoding _encoding);
    void read(std::string filePath, bool compressed);
    void clear();
    bool getNextBit();
    boost::dynamic_bitset<> getNextBits(size_t num_bits);
    std::string getNextString(size_t stringsize);
    std::string getNextString(size_t stringsize, size_t ncharbits);
    size_t remainingChars();
    bool eof();

    
    static boost::dynamic_bitset<> num2widx(size_t i, size_t num_bits);
    Encoder* encoder;
    
    // debug
    void print();
};

#endif /* defined(__TextDB__bitreader__) */
