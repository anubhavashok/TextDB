
//
//  pos.h
//  TextDB
//
//  Created by Anubhav on 8/9/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__pos__
#define __TextDB__pos__

#include <stdio.h>
#include <vector>

using namespace std;

class POSTagger
{
private:
    // Depends on model
public:
    vector<pair<string, string>> tag(vector<string> doc);
};

#endif /* defined(__TextDB__pos__) */
