//
//  docgraph.h
//  TextDB
//
//  Created by Anubhav on 5/9/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__docgraph__
#define __TextDB__docgraph__

#include <stdio.h>
#include <map>
#include <vector>
#include <iostream>
#include "Doc.h"

using namespace std;

// TODO: convert doc identifier from doc name string to doc id
class WNode
{
public:
    WNode(string _w);
    void addNextEdge(size_t i, WNode* next);
    void addPrevEdge(size_t i, WNode* prev);
    double nextProb(string w);
    double prevProb(string w);
    string w;
    map<size_t, WNode*> next;
    map<size_t, WNode*> prev;
};

class DGraph
{
    // perhaps include string to widx inside here
public:
    DGraph();
    bool add(Doc& doc);
    bool add(string docName, vector<string> doc);
    vector<string> get(string docName);
private:
    map<string, WNode*> words;
    map<size_t, WNode*> root;
    map<string, int> tf;
    map<string, size_t> doc2id;
    map<size_t, string> id2doc;
};


#endif /* defined(__TextDB__docgraph__) */
