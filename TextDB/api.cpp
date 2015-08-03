//
//  api.cpp
//  TextDB
//
//  Created by Anubhav on 7/31/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "api.h"
#include "query.h"
#include "queries.h"
#include "error.h"


// v1/get/{collectionName}/{docName}/sentiment
// v1/add/{collectionName}
// v1/add/{collectionName}/{docName}

API::API()
{
    // initialize routes from queries
    // trim extraneous /
    cout << "API initializing" << endl;
    queries.push_back(query("endpoints", "Lists all user facing endpoints", "v1/endpoints",
          [](DB* db, ostream& out, map<string, string>& args) {
              cout << "in lambda " << endl;
              for (auto p: args) {
                  cout << "args: " << p.first << endl;
              }
              boost::property_tree::ptree arr;
              cout << " reached " << endl;
              for (query q: queries) {
                  boost::property_tree::ptree inner;
                  inner.add("name", q.queryName);
                  inner.add("description", q.description);
                  inner.add("path", q.route);
                  arr.push_back(make_pair("", inner));
              }
              stringstream ss;
              boost::property_tree::ptree pt;
              pt.put_child("endpoints", arr);
              boost::property_tree::write_json(ss, pt);
              out << ss.str();
          }));
    for (int i = 0; i < queries.size(); i++) {
        query q = queries[i];
        try {
            routes.add(q.route, i);
        } catch (error& e) {
            cout << e.to_json() << endl;
        }
    }
    cout << "Added routes" << endl;
}

void API::accept(string req, ostream& out, DB* db)
{
    cout << "req: " << req << endl;
    // trim extraneous / from req
    boost::trim_if(req, boost::is_any_of(" /"));
    try {
        // check if request exists
        int qid = routes.get(req);
        query q = queries[qid];
        // try to parse request, if not output
        map<string, string> args = q.validate(req);
        // run query in request handler
        q.run(db, out, args);
    } catch (error& e) {
        // TODO: all replies should be objects
        // TODO: make e a property tree
        out << e.to_json() << endl;
    }
}


/*
void dynamic_add()
{
    if (routes.add())
}
*/