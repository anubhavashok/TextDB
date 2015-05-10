//
//  docgraph.cpp
//  TextDB
//
//  Created by Anubhav on 5/9/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "docgraph.h"
#include <iostream>

void DGraph::add(string docName, vector<string> doc)
{
    if (doc.size() == 0) {
        // throw error
        cerr << "Document cannot be empty" << endl;
        return;
    }
    
    // Create all nodes for words
    for (string w: doc) {
        if (!words.count(w)) {
            words[w] = new WNode(w);
        }
    }
    
    // Add document to root
    root[docName] = words[doc.front()];
    
    // Create links according to document
    if (doc.size() > 1) {
        words[doc[0]]->addNextEdge(docName, words[doc[1]]);
    }
    for (int i = 1; i < words.size() - 1; i++) {
        std:string w = doc[i];
        words[w]->addNextEdge(docName, words[doc[i+1]]);
        words[w]->addPrevEdge(docName, words[doc[i-1]]);

    }
    if (doc.size() > 1) {
        words[doc.back()]->addPrevEdge(docName, words[doc[doc.size() - 1]]);
    }
}

vector<string> DGraph::get(string docName)
{
    if (!root.count(docName)) {
        cerr << "Document: " << docName << " not found" << endl;
    }
    WNode* node = root[docName];
    vector<string> doc;
    while (node->next.count(docName)) {
        doc.push_back(node->w);
        node = node->next[docName];
    }
    return doc;
}


void WNode::addNextEdge(string doc, WNode* _next)
{
    next[doc] = _next;
}

void WNode::addPrevEdge(string doc, WNode* _prev)
{
    prev[doc] = _prev;
}

double WNode::nextProb(string w)
{
    double c = 0;
    double t = 0;
    for (auto p: next) {
        if (p.second->w == w) {
            c++;
        }
        t++;
    }
    return c/t;
}

double WNode::prevProb(string w)
{
    
    // do not store word in a map because of time-space trade off
    double c = 0;
    double t = 0;
    for (auto p: prev) {
        if (p.second->w == w) {
            c++;
        }
        t++;
    }
    return c/t;
}

WNode::WNode(string _w)
: w(_w)
{
}

void DGraph::add(const Doc& doc)
{
    // Add words to doc graph
    // add id to root
}