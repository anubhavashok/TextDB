
//
//  api.h
//  TextDB
//
//  Created by Anubhav on 7/31/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__api__
#define __TextDB__api__

#include <stdio.h>
#include <iostream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include "db.h"
#include "collection.h"
#include "error.h"
#include "router_tree.h"
#include "request.hpp"

using namespace std;

class API
{
private:
    string version;
    router_tree routes;
public:
    API();
    void accept(const http::server::request& req, ostream& out, DB* db);
    const error access_denied();
};

#endif /* defined(__TextDB__api__) */
