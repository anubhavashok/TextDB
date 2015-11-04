//
//  mutex_unordered_map.h
//  TextDB
//
//  Created by Anubhav on 10/31/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__mutex_unordered_map__
#define __TextDB__mutex_unordered_map__

#include <boost/thread/shared_mutex.hpp>
#include <boost/thread/locks.hpp>
#include "error.h"
#include <unordered_map>

class bad_access_error: public error
{
public:
    bad_access_error()
    : error("Map key does not exist", 823)
    {}
};

template<typename K, typename V>
class mutex_unordered_map
{
    std::unordered_map<K, V> m;
    boost::shared_mutex _access;
    
public:
    void write(K key, V val)
    {
        boost::unique_lock< boost::shared_mutex > lock(_access);
        m[key] = val;
    }
    
    V read(K key)
    {
        boost::shared_lock< boost::shared_mutex > lock(_access);
        // do read
        if (m.count(key)) {
            return m[key];
        } else {
            // throw bad access error
            throw bad_access_error();
        }
    }
    
};
#endif /* defined(__TextDB__mutex_unordered_map__) */
