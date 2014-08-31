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

class BitReader
{
private:
    // size of a character in bits
    const size_t charsize = CHAR_BIT;
    size_t pos;
    std::vector<char> data;
    bool end;
public:
    // read
    BitReader(const std::vector<char>& data);
    BitReader(std::string path);
    bool getNextBit();
    boost::dynamic_bitset<> getNextBits(size_t nbits);
    std::string getNextString(size_t stringsize);
    std::string getNextString(size_t stringsize, size_t ncharbits);
    bool eof();
    
    // write
    BitReader();
    void setNextBit(bool bit);
    void setNextBits(unsigned long data, size_t nbits);
    void setNextString(std::string word);
    void setNextString(std::string word, size_t ncharbits);
    void saveToFile(std::string path);
    
    static boost::dynamic_bitset<> num2widx(size_t i, size_t nbits);
    
    // debug
    void print();
};

#endif /* defined(__TextDB__bitreader__) */
