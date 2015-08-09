//
//  user.h
//  TextDB
//
//  Created by Anubhav on 8/5/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__user__
#define __TextDB__user__

#include <stdio.h>
#include <iostream>
#include <map>
#include <vector>

using namespace std;

class User
{
    string name;
    vector<string> labels;
    
public:
    User(string name);
    void addLabel();
};

#endif /* defined(__TextDB__user__) */
