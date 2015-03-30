//
//  entry.cpp
//  TextDB
//
//  Created by Anubhav on 3/23/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "entry.h"

Entry::Entry(int _index, int _term, Operation _op)
: index(_index), term(_term), op(_op)
{}

Entry::Entry()
{
    index = -1; term = -1;
}

Entries::Entries()
{
}

Entries::Entries(vector<Entry> _entries)
: entries(_entries)
{
}
