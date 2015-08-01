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


using namespace std;

// Query Errors
class QueryFormatError: error
{
public:
    QueryFormatError(string msg)
    : error(msg, 4347)
    {}
};

class ArgumentValueError: error
{
public:
    ArgumentValueError(string msg)
    : error(msg, 4348)
    {}
};

typedef std::function<void(DB* db, ostream& htmlout, map<string, string>& args)> QueryFunction;

class query
{
private:
    bool visible = true;
public:
    string description;
    vector<string> names;
    string queryName;
    const string route;
    // lambda that carries out query operation in database
    QueryFunction queryFunction;
    query(string queryName, string description, string route, QueryFunction queryfunction, bool visible);
    query(string queryName, string description, string route, QueryFunction queryfunction);
    static query valueOf(string req);
    map<string, string> validate(string req);
    void run(DB* db, ostream& out, map<string, string> args);
};


#endif /* defined(__TextDB__query__) */
