//
//  cache.h
//  TextDB
//
//  Created by Anubhav on 1/8/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__cache__
#define __TextDB__cache__

#include <stdio.h>
#include <map>
#include <string>
#include <boost/any.hpp>

class Cache
{
public:
    bool is_cached(std::string attr);
    void add(std::string attr, boost::any val);
    boost::any get(std::string attr);
    bool clear(std::string attr);
    
private:
    std::map<std::string, boost::any> cache;
};

#endif /* defined(__TextDB__cache__) */
