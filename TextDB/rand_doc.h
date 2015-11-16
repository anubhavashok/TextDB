
//
//  rand_doc.h
//  TextDB
//
//  Created by Anubhav on 11/4/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__rand_doc__
#define __TextDB__rand_doc__

#include <stdio.h>
#include <algorithm>
#include <random>

using namespace std;

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
#endif /* defined(__TextDB__rand_doc__) */
