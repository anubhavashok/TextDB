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
    bool nextBit();
    bool eof();
    
    // write
    BitReader();
    void setNextBit(bool bit);
    void saveToFile(std::string path);
    
    static std::bitset<18> num2widx(size_t i);
    
    // debug
    void print();
};

#endif /* defined(__TextDB__bitreader__) */
