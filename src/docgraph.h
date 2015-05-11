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

using namespace std;

// TODO: convert doc identifier from doc name string to doc id
class WNode
{
public:
    WNode(string _w);
    void addNextEdge(int i, WNode* next);
    void addPrevEdge(int i, WNode* prev);
    double nextProb(string w);
    double prevProb(string w);
    string w;
    map<int, WNode*> next;
    map<int, WNode*> prev;
};

class DGraph
{
    // perhaps include string to widx inside here
public:
    DGraph();
    void add(Doc doc); // TODO: create Doc class
    void add(string docName, vector<string> doc);
    vector<string> get(string docName);
private:
    map<string, WNode*> words;
    map<int, WNode*> root;
    map<string, int> tf;
    map<string, int> doc2id;
};


#endif /* defined(__TextDB__docgraph__) */
