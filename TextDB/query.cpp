//
//  query.cpp
//  TextDB
//
//  Created by Anubhav on 7/31/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "query.h"
#include <boost/algorithm/string.hpp>
#include "request.hpp"

query::query(string queryName, string description, string route, QueryFunction _queryfunction, bool visible)
: queryName(queryName), description(description), route(route), visible(visible), queryFunction(_queryfunction)
{
    // trim extraneous /
    boost:trim_if(route, boost::is_any_of(" /"));
    split(names, route, boost::is_any_of("/"));
}

query::query(string queryName, string description, string route, QueryFunction _queryFunction)
: query(queryName, description, route, _queryFunction, true)
{
}

static query valueOf(string req)
{
    // try to parse, if not ok, throw error
    ArgumentValueError e("arg", "expected_type", "provided_type");
    throw e;
}

map<string, string> query::validate(string req)
{
    map<string, string> args;
    vector<string> reqs;
    boost:split(reqs, req, boost::is_any_of("/"));
    assert(reqs.size() == names.size());
    for (int i = 0; i < names.size(); i++) {
        if ((names[i].front() == '{') && (names[i].back() == '}')) {
            string arg_name = names[i].substr(1, names[i].size() - 2);
            args[arg_name] = reqs[i];
            
            // TODO: validate type
            if (i != names.size() -1) {
                // check for existence of collection/document/
            }
        }
    }
    return args;
}
void query::run(DB* db, ostream& out, map<string, string>& args, const http::server::request& req)
{
    db->log << "(TextDB): " << req.method << " " << req.uri << " " << req.ip_address << endl;
    queryFunction(db, out, args);
}