//
//  Doc.cpp
//  TextDB
//
//  Created by Anubhav on 5/16/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "Doc.h"
#include <dtl/dtl.hpp>


using namespace dtl;


Ses<string> Doc::diff(vector<string> old, vector<string> current)
{
    Diff<string> d(old, current);
    d.compose();
    Ses<string> ses = d.getSes();
    return ses;
}

// WARNING - revert is a destructive operation
void Doc::revert(int n)
{
    if (n > diffs.size()) {
        return;
    }
    vector<Diff<string>> patches;
    // Revert to intended revision, n elements back
    for (int i = 0; i < n; i++) {
        patches.push_back(diffs.top());
        diffs.pop();
    }
    if (!loaded) {
        loadDocCache();
    }
    // Apply patches
    vector<string> doc = docCache;
    for (Diff<string> p: patches) {
        docCache = p.patch(docCache);
    }
}

void Doc::update(vector<string> doc)
{
    // create reverse diff, store diff, update document
    if (!loaded) {
        loadDocCache();
    }
    Diff<string> d(doc, docCache);
    d.compose();
    diffs.push(d);
    docCache = doc;
}

void Doc::loadDocCache()
{
}