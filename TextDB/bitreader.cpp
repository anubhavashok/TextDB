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
        fin >> datastring;
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
 * Access function to read next nbits
 * @param nbits a size_t that represents the number of bits to return
 * @return a dynamic_bitset of size nbits that contains the next nbits
 */

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

/*
 * getNextString
 * Convenience function that reads next bits as a string of charsize 5
 * @param stringsize a size_t representing size of the string to read
 * @return a string that is of size stringsize
 */

std::string BitReader::getNextString(size_t stringsize)
{
    return getNextString(stringsize, 5);
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

///*
// * setNextBit
// * A function that sets the next bit to true or false
// * @param bit a bool that the next bit is set to
// */
//
//void BitReader::setNextBit(bool bit)
//{
//    size_t idx = pos / charsize;
//    size_t offset = pos % charsize;
//    
//    if (idx >= data.size()) {
//        char newchar = 0;
//        data.push_back(newchar);
//    }
//    int mask = bit;
//    mask <<= offset;
//    data[idx] = data[idx] | mask;
//    pos++;
//}
//
///*
// * setNextBits
// * A function that sets the next nbits to match mydata
// * @param mydata an unsigned long representing the value to set the next nbits tos
// * @param nbits a size_t representing the number of bits to set
// */
//
//void BitReader::setNextBits(unsigned long mydata, size_t nbits)
//{
//    size_t mask = 1;
//    for (size_t i = 0; i < nbits; i ++) {
//        if (mask & mydata) {
//            setNextBit(true);
//        } else {
//            setNextBit(false);
//        }
//        mask <<= 1;
//    }
//}
//
///*
// * setNextString
// * A function that sets the next bits to word (where each character is 5bits)
// * @param word a string that contains the value to set the next bits to
// */
//
//void BitReader::setNextString(std::string word)
//{
//    setNextString(word, 5);
//}
//
///*
// * setNextString
// * A function that sets the next bits to word (where each character is ncharbits)
// * @param word a string that contains the value to set the next bits to
// * @param ncharbits a size_t representing the number of bits to encode a character in
// */
//
//void BitReader::setNextString(std::string word, size_t ncharbits)
//{
//    for (char c: word) {
//        size_t charnum = encoder->encode(c);
//        // we can still accomodate for 6 more characters
//        // to be decided
//        assert(charnum < 32);
//        setNextBits(charnum, ncharbits);
//    }
//}

/*
 * remainingChars
 * A function that returns the number of bits remaining in the current character
 * @return a size_t representing the number of bits remaining
 */

size_t BitReader::remainingChars()
{
    return data.size() - pos/charsize;
}

///*
// * saveToFile
// * A function that compresses and saves the current data to file
// * @param path a string that contains the absolute path to the file the data is to be saved to
// */
//
//void BitReader::saveToFile(std::string path)
//{
//    assert(data.size() >= 5);
//    ofstream fout(path, ios::out | ios::binary);
//    std::string datastring(data.begin(), data.end());
//    std::string compresseddatastring;
//    snappy::Compress(datastring.data(), datastring.size(), &compresseddatastring);
//    for (char c: compresseddatastring) {
//        fout.put(c);
//    }
//    fout.close();
//}

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

///*
// * char2num
// * A convenience function that converts a character to a number for encoding
// * @param c a char representing the character to be converted
// * @param a size_t representing the number
// */
//
//size_t BitReader::char2num(char c)
//{
//    if (isalpha(c) && islower(c)) {
//        return c - 'a';
//    } else if (c == '\n') {
//        return 27;
//    } else if (c == '.') {
//        return 28;
//    } else if (c == ',') {
//        return 29;
//    } else if (c == '(') {
//        return 30;
//    } else if (c == ')') {
//        return 31;
//    } else {
//        // invalid character
//        cout << "char is :" << c << endl;
//        assert(false);
//    }
//}
//
///*
// * num2char
// * A convenience function that converts a number to the appropriate character
// * @param num a size_t representing the number to be converted
// * @param a char representing the converted char
// */
//
//char BitReader::num2char(size_t num)
//{
//    assert(num < 32);
//    switch(num)
//    {
//        case 27: return '\n';
//        case 28: return '.';
//        case 29: return ',';
//        case 30: return '(';
//        case 31: return ')';
//        default: return num + 'a';
//    }
//}



