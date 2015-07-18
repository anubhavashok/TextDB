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
#include "encoder.h"


/*
 * BitReader
 * Constructor that constructs a bitreader that wraps a provided data vector
 * @param data a const reference vector of chars containing the compressed/byte encoded characters read in from a file
 */

BitReader::BitReader(const std::vector<char>& data, Encoder::CharacterEncoding _encoding)
{
    encoder = Encoder::createEncoder(_encoding);
    for (char c: data) {
        this->data.push_back(c);
    }
    pos = 0;
    end = false;
}

/*
 * BitReader
 * Constructor that constructs an empty bitreader
 */

BitReader::BitReader(Encoder::CharacterEncoding _encoding)
{
    encoder = Encoder::createEncoder(_encoding);
    data = std::vector<char>();
    pos = 0;
    end = false;
}


void BitReader::read(std::string path, bool compressed)
{
    if (!boost::filesystem::exists(path) || boost::filesystem::path(path).empty()) {
        cout << "error in opening file in bitreader" << endl;
        exit(-1);
    }
    ifstream fin(path, ios::in | ios::binary);
    char c;
    // read raw data as chars
    std::string datastring;

    if (compressed) {
        std::string compresseddatastring = "";
        while (!fin.eof()) {
            c = fin.get();
            compresseddatastring += c;
        }
        snappy::Uncompress(compresseddatastring.data(), compresseddatastring.size(), &datastring);
    } else {
        datastring = std::string((std::istreambuf_iterator<char>(fin)), std::istreambuf_iterator<char>());
    }
    
    std::vector<char> uncompresseddata(datastring.begin(), datastring.end());
    fin.close();
    // TODO: call self
    data = uncompresseddata;
    pos = 0;
    end = false;
}

void BitReader::clear()
{
    data.clear();
    pos = 0;
    end = false;
}

/*
 * getNextBit
 * Access function to read next bit from characters vector
 * @return a bool equivalent to the state of next bit
 */

bool BitReader::getNextBit()
{
    size_t charsize = 8;
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

/*
 * getNextBits
 * Access function to read next num_bits
 * @param num_bits a size_t that represents the number of bits to return
 * @return a dynamic_bitset of size num_bits that contains the next num_bits
 */

boost::dynamic_bitset<> BitReader::getNextBits(size_t num_bits)
{
    if (eof()) {
        // reached end of file prematurely
        cout << "Unexpected error when decoding file" << endl;
    }
    boost::dynamic_bitset<> mydata(num_bits);
    for (size_t i = 0; i < num_bits; i ++) {
        if (getNextBit()) {
            mydata.set(i);
        }
    }
    return mydata;
}

/*
 * getNextString
 * Convenience function that reads next bits as a string of charsize 5
 * @param stringsize a size_t representing size of the string to read
 * @return a string that is of size stringsize
 */

std::string BitReader::getNextString(size_t stringsize)
{
    return getNextString(stringsize, encoder->charSize());
}

/*
 * getNextString
 * Convenience function that reads next bits as a string of charsize ncharbits
 * @param stringsize a size_t representing size of the string to read
 * @param ncharbits a size_t representing the size of each character
 * @return a string that is of size stringsize
 */

std::string BitReader::getNextString(size_t stringsize, size_t ncharbits)
{
    std::string word;
    for (size_t i = 0; i < stringsize; i ++) {
        // read ncharbit bit character
        boost::dynamic_bitset<> charbits = getNextBits(ncharbits);
        char c = encoder->decode(charbits.to_ulong());
        word += c;
    }
    return word;
}

/*
 * eof
 * Function that returns true if the bitreader has no more bits to read
 * @return a bool that indicates if there are anymore bits left to read or not
 */

bool BitReader::eof()
{
    return end;
}

/*
 * remainingChars
 * A function that returns the number of bits remaining in the current character
 * @return a size_t representing the number of bits remaining
 */

size_t BitReader::remainingChars()
{
    size_t charsize = 8;
    return data.size() - pos/charsize;
}


/*
 * print
 * Debug function that prints out the current data as characters
 */

void BitReader::print()
{
    for (char c: data) {
        cout << c << endl;
    }
}



