//
//  Doc.h
//  TextDB
//
//  Created by Anubhav on 5/16/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__Doc__
#define __TextDB__Doc__

#include <stdio.h>
#include <vector>
#include <iostream>
#include <map>
#include <stack>
#include <dtl/dtl.hpp>

using namespace std;
using namespace dtl;

class Doc
{
    // acts as a cache and stores meta data about document
    bool loaded = false;
    vector<string> docCache;
    map<string, int> tf;
    // contains ids of previous versions, monotonically increasing
    stack<int> versions;
    Ses<string> diff(vector<string> d1, vector<string> d2);
    int docID;

public:
    Doc(string _name, vector<string> doc)
    : name(_name)
    {
        loaded = true;
        docCache = doc;
        // calculate tf score
        for (string w: docCache) {
            tf[w]++;
        }
    }
    map<string, int> getTF()
    {
        return tf;
    }
    stack<Diff<string>> diffs;

    // on load only load tf
    void loadDocCache();
    
    // revisions
    void update(vector<string> doc);
    void revert(int n);
    void output(ostream& out)
    {
        for (string w: docCache) {
            out << w << " " << endl;
        }
    }
    const string name;
    
    bool setID(int _docID);
    int getID() const;
    size_t size() const;
    const vector<string>& getDocCache();
};


#endif /* defined(__TextDB__Doc__) */
