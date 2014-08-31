//
//  preformatter.cpp
//  TextDB
//
//  Created by Anubhav on 8/30/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#include "preformatter.h"
#include <algorithm>
void Preformatter::removePunctuations(std::vector<std::string>& doc)
{
    for (std::string& s: doc) {
        std::string fs = "";
        for (size_t i = 0; i < s.size(); i ++) {
            char c = s[i];
            if (!ispunct(c)) {
                fs += c;
            }
        }
        s = fs;
    }
}

void Preformatter::toLower(std::vector<std::string>& doc)
{
    for (std::string s: doc) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    }
}