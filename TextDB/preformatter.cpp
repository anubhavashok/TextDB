//
//  preformatter.cpp
//  TextDB
//
//  Created by Anubhav on 8/30/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#include "preformatter.h"
#include <algorithm>
#include <boost/algorithm/string.hpp>

/*
 * removePunctuations
 * A function that removes all non-alphabet characters except ",.()" for all strings in input vector
 * @param doc a vector of strings representing a text document
 */

void Preformatter::removePunctuations(std::vector<std::string>& doc)
{
    for (std::string& s: doc) {
        std::string fs = "";
        for (size_t i = 0; i < s.size(); i ++) {
            char c = s[i];
            if (isalpha(c) || boost::is_any_of("\n,.()")(c)) {
                fs += c;
            }
        }
        s = fs;
    }
}

/*
 * toLower
 * A function that converts all characters to lowercase for all strings in input vector
 * @param doc a vector of strings representing a text document
 */

void Preformatter::toLower(std::vector<std::string>& doc)
{
    for (std::string s: doc) {
        std::transform(s.begin(), s.end(), s.begin(), ::tolower);
    }
}