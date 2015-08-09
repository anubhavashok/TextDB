//
//  query.h
//  TextDB
//
//  Created by Anubhav on 7/31/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__query__
#define __TextDB__query__

#include <stdio.h>
#include <vector>
#include <iostream>
#include "db.h"
#include "error.h"
#include "request.hpp"


using namespace std;

// Query Errors
class QueryFormatError: public error
{
public:
    QueryFormatError(string msg)
    : error(msg, 4347)
    {}
};

class ArgumentValueError: public error
{
public:
    ArgumentValueError(string arg, string expected, string provided)
    : error("Format of arg: " + arg + " is supposed to be " + expected + " but is " + provided, 4348)
    {}
};

typedef std::function<void(DB* db, ostream& out, map<string, string>& args)> QueryFunction;

class query
{
private:
    bool visible = true;
public:
    string description;
    vector<string> names;
    string queryName;
    string route;
    // lambda that carries out query operation in database
    QueryFunction queryFunction;
    query(string queryName, string description, string route, QueryFunction queryfunction, bool visible);
    query(string queryName, string description, string route, QueryFunction queryfunction);
    static query valueOf(string req);
    map<string, string> validate(string req);
    void run(DB* db, ostream& out, map<string, string>& args, const http::server::request& req);
};


#endif /* defined(__TextDB__query__) */
