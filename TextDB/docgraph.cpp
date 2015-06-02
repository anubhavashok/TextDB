//
//  docgraph.cpp
//  TextDB
//
//  Created by Anubhav on 5/9/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "docgraph.h"
#include <iostream>
/*
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


void WNode::addNextEdge(int i, WNode* _next)
{
    next[i] = _next;
}

void WNode::addPrevEdge(int i, WNode* _prev)
{
    prev[i] = _prev;
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
    name2id[doc.name] = doc.id;
    // Add words to doc graph
    // add id to root
    if (doc.d.size() == 0) {
        // throw error
        cerr << "Document cannot be empty" << endl;
        return;
    }
    
    // Create all nodes for words
    for (string w: doc.d) {
        if (!words.count(w)) {
            words[w] = new WNode(w);
        }
    }
    
    // Add document to root
    root[docName] = words[doc.d.front()];
    
    // Create links according to document
    if (doc.d.size() > 1) {
        words[doc.d[0]]->addNextEdge(doc.id, words[doc.d[1]]);
    }
    for (int i = 1; i < words.size() - 1; i++) {
        std:string w = doc.d[i];
        words[w]->addNextEdge(doc.id, words[doc.d[i+1]]);
        words[w]->addPrevEdge(doc.id, words[doc.d[i-1]]);
        
    }
    if (doc.d.size() > 1) {
        words[doc.d.back()]->addPrevEdge(doc.id, words[doc.d[doc.d.size() - 1]]);
    }
    
}
*/