//
//  encoder.h
//  TextDB
//
//  Created by Anubhav on 12/28/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#ifndef __TextDB__encoder__
#define __TextDB__encoder__

#include <stdio.h>
#include <string>
using namespace std;

class Encoder
{
public:
    enum CharacterEncoding
    {
        Compressed,
        Unicode
    };
    static Encoder* createEncoder(CharacterEncoding _encoding);
    virtual unsigned long encode(char c) = 0;
    virtual char decode(unsigned long i) = 0;
    virtual string preformat(string s) = 0;
    virtual size_t charSize() = 0;
    static CharacterEncoding str2encoding(std::string type);
    static std::string encoding2str(CharacterEncoding _encoding);
};

class UnicodeEncoder: public Encoder
{
public:
    unsigned long encode(char c);
    char decode(unsigned long i);
    string preformat(string s);
    size_t charSize();
};


class CompressedEncoder: public Encoder
{
public:
    unsigned long encode(char c);
    char decode(unsigned long i);
    string preformat(string s);
    size_t charSize();
};

#endif /* defined(__TextDB__encoder__) */
