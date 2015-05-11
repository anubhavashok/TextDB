//
//  cache.cpp
//  TextDB
//
//  Created by Anubhav on 1/8/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "cache.h"


bool Cache::is_cached(std::string attr)
{
    return cache.count(attr);
}

void Cache::add(std::string attr, boost::any val)
{
    cache[attr] = val;
}

boost::any Cache::get(std::string attr)
{
    return cache[attr];
}

bool Cache::clear(std::string attr)
{
    if (! cache.count(attr)) {
        return false;
    }
    cache.erase(attr);
    return true;
}
