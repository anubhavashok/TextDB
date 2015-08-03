//
//  router_tree.h
//  TextDB
//
//  Created by Anubhav on 7/31/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__router_tree__
#define __TextDB__router_tree__

#include <stdio.h>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <unordered_map>
#include "query.h"
#include "error.h"

using namespace std;

class PreexistingRoute: public error
{
public:
    PreexistingRoute(string route)
    : error("Route: " + route + " already exists", 937)
    {}
};

class UnknownQuery: public error
{
public:
    UnknownQuery(string queryName)
    : error("Unknown query: " + queryName, 938)
    {}
};


class router_tree
{
private:
    class node
    {
    private:
        string name;
        unordered_map<string, node> children;
        bool value = false;
        int qid = -1;
        
        bool is_value_label(string name);
    public:
        // UNUSED - only exists for map index access
        node()
        {}
        node(string name);
        node(string name, int qid);

        void add_recursive(deque<string> routes, int _qid);
        // Check only when adding
        bool exists_recursive(deque<string> routes);
        
        int get_recursive(deque<string> routes);
        
        string get_preexisting_name_recursive(deque<string> routes);

    };
public:
    node root;
    router_tree();
    void add(string route, int qid);
    int get(string route);
    
};

#endif /* defined(__TextDB__router_tree__) */
