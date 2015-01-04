//
//  bitwriter.cpp
//  TextDB
//
//  Created by Anubhav on 12/29/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#include "bitwriter.h"
#include <cassert>
#include <fstream>
#include <snappy.h>
#include <cmath>
using namespace std;


BitWriter::BitWriter(Encoder::CharacterEncoding _encoder)
: encoder(Encoder::createEncoder(_encoder))
{}


/*
 * setNextBit
 * A function that sets the next bit to true or false
 * @param bit a bool that the next bit is set to
 */

void BitWriter::setNextBit(bool bit)
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

/*
 * setNextBits
 * A function that sets the next nbits to match mydata
 * @param mydata an unsigned long representing the value to set the next nbits tos
 * @param nbits a size_t representing the number of bits to set
 */

void BitWriter::write(unsigned long mydata, size_t nbits)
{
    assert(mydata < pow(2,nbits));
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

/*
 * setNextString
 * A function that sets the next bits to word (where each character is 5bits)
 * @param word a string that contains the value to set the next bits to
 */

void BitWriter::write(std::string word)
{
    write(word, encoder->charSize());
}

/*
 * setNextString
 * A function that sets the next bits to word (where each character is ncharbits)
 * @param word a string that contains the value to set the next bits to
 * @param ncharbits a size_t representing the number of bits to encode a character in
 */

void BitWriter::write(std::string word, size_t ncharbits)
{
    for (char c: word) {
        size_t charnum = encoder->encode(c);
        // we can still accomodate for 6 more characters
        // to be decided
        //assert(charnum < 32);
        write(charnum, ncharbits);
    }
}

/*
 * saveToFile
 * A function that compresses and saves the current data to file
 * @param path a string that contains the absolute path to the file the data is to be saved to
 */

void BitWriter::saveToFile(std::string path, bool compress)
{
    cout << path << endl;
    // assert(data.size() >= 5);
    ofstream fout(path, ios::out | ios::binary);
    std::string datastring(data.begin(), data.end());
    if (compress) {
        std::string compresseddatastring;
        snappy::Compress(datastring.data(), datastring.size(), &compresseddatastring);
        for (char c: compresseddatastring) {
            fout.put(c);
        }
    } else {
        for (char c: data) {
            fout.put(c);
        }
    }

    fout.close();
}

/*
 * appendToFile
 * A function that compresses and appends the current data to file
 * @param path a string that contains the absolute path to the file the data is to be saved to
 */

void BitWriter::appendToFile(std::string path, bool compress)
{
    //assert(data.size() >= 5);
    ofstream fout(path, ios::app | ios::out | ios::binary);
    std::string datastring(data.begin(), data.end());
    if (compress) {
        std::string compresseddatastring;
        snappy::Compress(datastring.data(), datastring.size(), &compresseddatastring);
        for (char c: compresseddatastring) {
            fout.put(c);
        }
    } else {
        for (char c: data)
            fout.put(c);
    }

    fout.close();
}



char BitWriter::generateNewLastByte(char lastByte, char firstByte, int offset)
{
    // create mask
    char mask;
    for (int i = 0; i < offset; i++) {
        mask &= 1;
        mask <<= 1;
    }
    char newHalf = (mask & firstByte) << (8 - offset);
    assert(((mask << (8 - offset)) & lastByte) == 0);
    return lastByte & newHalf;
}

//void BitWriter::shift(int offset)
//{
//    // convert data into bit set
//    // starting at offset bit till end, shift all to pos - offset
//    // reconvert into array of chars
//    boost::dynamic_bitset<> bs = boost::dynamic_bitset<char>(data.begin(), data.end());
//    
//    for (int i = offset; i < bs.size(); i++) {
//        bs[i-offset] = bs[i];
//    }
//    for (int i = (int)bs.size()-offset; i < bs.size(); i++) {
//        bs[i] = 0;
//    }
//    data.clear();
//    boost::to_block_range(bs, std::back_inserter(data));
//}

//void BitWriter::appendToFile(std::string path, bool compress, int pos)
//{
//    //assert(data.size() >= 5);
//    fstream fout(path, ios::in | ios::out | ios::binary);
//    int offset = pos%8;
//    int nbytes = pos/8;
//    fout.seekg(nbytes);
//    char lastByte;
//    fout.get(lastByte);
//    char firstByte = data[0];
//    char newLastByte = generateNewLastByte(lastByte, firstByte, offset);
//
//    // write newLastByte to pos
//    fout.seekp(nbytes);
//    fout.put(newLastByte);
//    fout.close();
//    shift(offset);
//    fout.open(path, ios::app | ios::out);
//    for (char c: data) {
//        fout.put(c);
//    }
//    
//    std::string datastring(data.begin(), data.end());
//    if (compress) {
//        std::string compresseddatastring;
//        snappy::Compress(datastring.data(), datastring.size(), &compresseddatastring);
//        for (char c: compresseddatastring) {
//            fout.put(c);
//        }
//    } else {
//        for (char c: data)
//            fout.put(c);
//    }
//    
//    fout.close();
//}



void BitWriter::clear()
{
    pos = 0;
    end = false;
    data.clear();
}

void BitWriter::print()
{
    for (char c: data) {
        cout << c << endl;
    }
}