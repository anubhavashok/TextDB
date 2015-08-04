//
//  encoder.cpp
//  TextDB
//
//  Created by Anubhav on 12/28/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#include "encoder.h"
#include <iostream>
#include <cassert>
#include <string>
#include "error.h"
#include <boost/algorithm/string.hpp>

using namespace std;

class UnknownEncoding: public error
{
public:
    UnknownEncoding(string encoding)
    : error("Unknown encoding: " + encoding, 647)
    {}
};

Encoder* Encoder::createEncoder(Encoder::CharacterEncoding _encoding)
{
    switch (_encoding) {
        Compressed:
            return new CompressedEncoder();
            break;
        default:
        Unicode:
            return new UnicodeEncoder();
            break;
    }
}

unsigned long UnicodeEncoder::encode(char c)
{
    cout << c << " val: " << (int)c << endl;
    unsigned long i = (unsigned long) c;
    if (i > 256) return (unsigned long)'?';
    return i;
}

char UnicodeEncoder::decode(unsigned long n)
{
    char c = (char) n;
    return c;
}

size_t UnicodeEncoder::charSize()
{
    return 16;
}

string UnicodeEncoder::preformat(string s)
{
    // transform non-unicode characters to '?'
    std::transform(s.begin(), s.end(), s.begin(), [](char c){ return ((int)c >= 0 && (int)c <= 255) ? c : '?';});
    return s;
}


unsigned long CompressedEncoder::encode(char c)
{
    if (isalpha(c) && islower(c)) {
        return c - 'a';
    } else if (c == '\n') {
        return 27;
    } else if (c == '.') {
        return 28;
    } else if (c == ',') {
        return 29;
    } else if (c == '(') {
        return 30;
    } else if (c == ')') {
        return 31;
    } else {
        // invalid character
        cout << "char is :" << c << endl;
        assert(false);
    }
}

char CompressedEncoder::decode(unsigned long n)
{
    switch(n)
    {
        case 27: return '\n';
        case 28: return '.';
        case 29: return ',';
        case 30: return '(';
        case 31: return ')';
        default: return n + 'a';
    }
}

size_t CompressedEncoder::charSize()
{
    return 5;
}

string CompressedEncoder::preformat(string s)
{
    std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    return s;
}

Encoder::CharacterEncoding Encoder::str2encoding(std::string encoding)
{
    boost::to_lower(encoding);
    if (encoding == "compressed") {
        return Encoder::CharacterEncoding::Compressed;
    } else if (encoding == "unicode") {
        return Encoder::CharacterEncoding::Unicode;
    } else {
        throw UnknownEncoding(encoding);
    }
}

std::string Encoder::encoding2str(CharacterEncoding _encoding)
{
    switch (_encoding) {
        case Compressed: return "compressed";
        default:
        case Unicode: return "unicode";
    }
}



