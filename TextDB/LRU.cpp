//
//  LRU.cpp
//  TextDB
//
//  Created by Anubhav on 1/2/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "LRU.h"
#include <string>

void LRU::access(std::string name)
{
    _lru.remove(name);
    _lru.push_front(name);
}

std::string LRU::pop()
{
    if (_lru.size() > 0) {
        std::string name = _lru.back();
        _lru.pop_back();
        return name;
    } else {
        return "";
    }
}

bool LRU::cached(std::string name)
{
    for (std::string n: _lru) {
        if (name == n) {
            return true;
        }
    }
    return false;
}
