//
//  docgraph.cpp
//  TextDB
//
//  Created by Anubhav on 5/9/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "docgraph.h"
#include <iostream>

bool DGraph::add(string docName, vector<string> doc)
{
    if (doc.size() == 0) {
        // throw error
        cerr << "Document cannot be empty" << endl;
        return false;
    }
    
    if (doc2id.count(docName)) {
        cerr << "Document already exists" << endl;
        return false;
    }
    long docID = doc2id.size();
    doc2id[docName] = docID;
    id2doc[docID] = docName;
    
    // Create all nodes for words
    for (string w: doc) {
        if (!words.count(w)) {
            words[w] = new WNode(w);
        }
    }
    
    // Add document to root
    // root takes in an id
    root[docID] = words[doc.front()];
    
    // Create links according to document
    if (doc.size() > 1) {
        words[doc[0]]->addNextEdge(docID, words[doc[1]]);
    }
    for (int i = 1; i < words.size() - 1; i++) {
        std:string w = doc[i];
        words[w]->addNextEdge(docID, words[doc[i+1]]);
        words[w]->addPrevEdge(docID, words[doc[i-1]]);

    }
    if (doc.size() > 1) {
        words[doc.back()]->addPrevEdge(docID, words[doc[doc.size() - 1]]);
    }
    return true;
}

vector<string> DGraph::get(string docName)
{
    if (!doc2id.count(docName)) {
        return vector<string>();
    }
    size_t docID = doc2id[docName];
    if (!root.count(docID)) {
        cerr << "Document: " << docName << " not found" << endl;
    }
    WNode* node = root[docID];
    vector<string> doc;
    while (node->next.count(docID)) {
        doc.push_back(node->w);
        node = node->next[docID];
    }
    return doc;
}


void WNode::addNextEdge(size_t i, WNode* _next)
{
    next[i] = _next;
}

void WNode::addPrevEdge(size_t i, WNode* _prev)
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

bool DGraph::add(Doc& doc)
{
    int docID = doc.getID();
    doc2id[doc.name] = docID;
    // Add words to doc graph
    // add id to root
    if (doc.size() <= 0) {
        // throw error
        cerr << "Document cannot be empty" << endl;
        return false;
    }
    
    // Create all nodes for words
    const vector<string>& docCache = doc.getDocCache();
    for (string w: docCache) {
        if (!words.count(w)) {
            words[w] = new WNode(w);
        }
    }
    
    // Add document to root
    root[docID] = words[docCache.front()];
    
    // Create links according to document
    if (docCache.size() > 1) {
        words[docCache[0]]->addNextEdge(docID, words[docCache[1]]);
    }
    for (int i = 1; i < words.size() - 1; i++) {
        std:string w = docCache[i];
        words[w]->addNextEdge(docID, words[docCache[i+1]]);
        words[w]->addPrevEdge(docID, words[docCache[i-1]]);
        
    }
    if (docCache.size() > 1) {
        words[docCache.back()]->addPrevEdge(docID, words[docCache[docCache.size() - 1]]);
    }
    return true;
}
