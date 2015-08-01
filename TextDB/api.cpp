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


// v1/get/{collectionName}/{docName}/sentiment
// v1/add/{collectionName}
// v1/add/{collectionName}/{docName}

API::API()
{
    // initialize routes from queries
    // trim extraneous /
    for (int i = 0; i < queries.size(); i++) {
        query q = queries[i];
        routes.add(q.route, i);
    }
}

void API::accept(string req, ostream& out, DB* db)
{
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