//
//  rand_doc.cpp
//  TextDB
//
//  Created by Anubhav on 11/4/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "rand_doc.h"

string gen_word(int n)
{
    string s;
    for (int i = 0; i < n; i++) {
        s += (char)('a' + (rand() % 26));
    }
    return s;
}

string gen_doc(int n)
{
    string s = "";
    for (int i = 0; i < n; i++) {
        int wl = 5 + rand() % 10;
        s += gen_word(wl) + " ";
    }
    return s;
}