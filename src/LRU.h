//
//  LRU.h
//  TextDB
//
//  Created by Anubhav on 1/2/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__LRU__
#define __TextDB__LRU__

#include <stdio.h>
#include <unordered_map>
#include <list>

class LRU
{
    std::list<std::string> _lru;
public:
    void access(std::string name);
    std::string pop();
    bool cached(std::string name);
};

#endif /* defined(__TextDB__LRU__) */
