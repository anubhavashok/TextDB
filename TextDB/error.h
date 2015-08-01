//
//  error.h
//  TextDB
//
//  Created by Anubhav on 7/31/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__error__
#define __TextDB__error__

#include <stdio.h>
#include <iostream>

using namespace std;

class error
{
private:
    int code;
    string msg;
public:
    error(string msg, int code);
    string to_string();
    string to_json();
};

#endif /* defined(__TextDB__error__) */
