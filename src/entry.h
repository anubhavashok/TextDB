//
//  entry.h
//  TextDB
//
//  Created by Anubhav on 3/23/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__entry__
#define __TextDB__entry__

#include <stdio.h>
#include "operation.h"
#include <boost/serialization/serialization.hpp>
#include <boost/serialization/vector.hpp>

class Entry
{
public:
    Entry();
    Entry(int _index, int _term, Operation _op);
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & op;
        ar & index;
        ar & term;
    }

    Operation op;
    int index;
    int term;
    
    bool operator==(const Entry& e1) const
    {
        return ((e1.index == index) && (e1.term == term) && (e1.op == op));
    };
    
    bool operator!=(const Entry& e1) const
    {
        return !(e1 == *this);
    };

};

class Entries
{
public:
    Entries();
    Entries(vector<Entry> _entries);
    vector<Entry> entries;
    template<class Archive>
    void serialize(Archive& ar, const unsigned int version)
    {
        ar & entries;
    }
};
#endif /* defined(__TextDB__entry__) */
