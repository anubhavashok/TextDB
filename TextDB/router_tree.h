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

class PreexistingRoute: error
{
public:
    PreexistingRoute(string msg)
    : error(msg, 937)
    {}
};

class UnknownQuery: error
{
public:
    UnknownQuery(string msg)
    : error(msg, 938)
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
    public:
        // UNUSED - only exists for map index access
        node()
        {}
        node(string name)
        : name(name)
        {
            if ((name.front() == '{') && (name.back() == '}')) {
                value = true;
            }
        }
        node(string name, int qid)
        : name(name), qid(qid)
        {
            if ((name.front() == '{') && (name.back() == '}')) {
                value = true;
            }
            
        }

        void add_recursive(deque<string> routes, int qid)
        {
            if (routes.size() == 0) {
                return;
            }
            string r = routes.front();
            routes.pop_front();
            if (!children.count(r)) {
                if (routes.empty()) {
                    children[r] =  node(r, qid);
                } else {
                    children[r] =  node(r);
                }
            }
            children[r].add_recursive(routes, qid);
        }
        
        bool exists_recursive(deque<string> routes)
        {
            if (routes.size() == 0) {
                return true;
            }
            string r = routes.front();
            routes.pop_front();
            if (value) {
                // check all
                bool exists = false;
                for (auto p: children) {
                    exists |= p.second.exists_recursive(routes);
                }
                return exists;
            } else if (children.count(r)) {
                return children[r].exists_recursive(routes);
            }
            return false;
        }
        
        int get_recursive(deque<string> routes)
        {
            if (routes.size() == 0) {
                return qid;
            }
            string r = routes.front();
            routes.pop_front();
            if (value) {
                // check all
                int exists = -1;
                for (auto p: children) {
                    int v = p.second.exists_recursive(routes);
                    exists = v != -1? v: exists;
                }
                return exists;
            } else if (children.count(r)) {
                return children[r].exists_recursive(routes);
            }
            return -1;
        }
        
        string get_preexisting_name_recursive(deque<string> routes)
        {
            if (routes.size() == 0) {
                return name;
            }
            string r = routes.front();
            routes.pop_front();
            if (value) {
                // check all
                string exists = "";
                for (auto p: children) {
                    string v = p.second.get_preexisting_name_recursive(routes);
                    exists = v.empty() ? exists: v;
                }
                return exists;
            } else if (children.count(r)) {
                return name + "/" + children[r].get_preexisting_name_recursive(routes);
            }
            return "";
        }

    };
public:
    node root;
    router_tree()
    : root("root")
    {
    }
    void add(string route, int qid)
    {
        deque<string> routes;
        boost::split(routes, route, boost::is_any_of("/"));
        if (root.exists_recursive(routes)) {
            string route_name = root.get_preexisting_name_recursive(routes);
            throw PreexistingRoute("Route: " + route_name + " already exists in router tree. Requested route has to contain a different route.");
        }
        root.add_recursive(routes, qid);
    }
    int get(string route)
    {
        deque<string> routes;
        boost::split(routes, route, boost::is_any_of("/"));
        int qid = root.get_recursive(routes);
        if (qid == -1) {
            throw UnknownQuery("Query: " + route + " does not exist");
        }
        return qid;
    }
    
};

#endif /* defined(__TextDB__router_tree__) */
