//
//  preformatter.h
//  TextDB
//
//  Created by Anubhav on 8/30/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#ifndef __TextDB__preformatter__
#define __TextDB__preformatter__

#include <iostream>
#include <vector>
#include <string>

using namespace std;

class Preformatter
{
public:
    static void removePunctuations(std::vector<std::string>& string);
    static void toLower(std::vector<std::string>& string);
};

#endif /* defined(__TextDB__preformatter__) */
