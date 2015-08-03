//
//  router_tree.cpp
//  TextDB
//
//  Created by Anubhav on 7/31/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "router_tree.h"
#include <stdio.h>
#include <iostream>
#include <boost/algorithm/string.hpp>
#include <unordered_map>
#include "query.h"
#include "error.h"

using namespace std;



bool router_tree::node::is_value_label(string name)
{
    return ((name.front() == '{') && (name.back() == '}'));
}

router_tree::node::node(string name)
: name(name)
{
    if ((name.front() == '{') && (name.back() == '}')) {
        value = true;
    }
}

router_tree::node::node(string name, int qid)
: name(name), qid(qid)
{
    if ((name.front() == '{') && (name.back() == '}')) {
        value = true;
    }
    
}
        
void router_tree::node::add_recursive(deque<string> routes, int _qid)
{
    if (routes.size() == 0) {
        qid = _qid;
        return;
    }
    string r = routes.front();
    routes.pop_front();
    if (!children.count(r)) {
        if (routes.empty()) {
            children[r] =  node(r, _qid);
        } else {
            children[r] =  node(r);
        }
    }
    children[r].add_recursive(routes, _qid);
}
        // Check only when adding
bool router_tree::node::exists_recursive(deque<string> routes)
{
    if (routes.size() == 0) {
        return qid != -1;
    }
    string r = routes.front();
    routes.pop_front();
    if (children.count(r)) {
        return children[r].exists_recursive(routes);
    } else {
        // check all, assume that any of the labels could be used as the value of the placeholder
        bool exists = false;
        for (auto p: children) {
            if (p.second.value || is_value_label(r)) {
                exists |= p.second.exists_recursive(routes);
            }
        }
        return exists;
    }
    return false;
}

int router_tree::node::get_recursive(deque<string> routes)
{
    if (routes.size() == 0) {
        return qid;
    }
    string r = routes.front();
    routes.pop_front();
    if (children.count(r)) {
        return children[r].get_recursive(routes);
    } else {
        // check all
        int exists = -1;
        for (auto p: children) {
            if (p.second.value) {
                int v = p.second.get_recursive(routes);
                exists = (v != -1)? v: exists;
            }
        }
        return exists;
    }
    return -1;
}

string router_tree::node::get_preexisting_name_recursive(deque<string> routes)
{
    if (routes.size() == 0) {
        return (qid == -1) ? "": name;
    }
    string r = routes.front();
    routes.pop_front();
    if (children.count(r)) {
        string exists = children[r].get_preexisting_name_recursive(routes);
        return exists.empty()? exists: (name + "/" + exists);
    } else {
        string exists = "";
        for (auto p: children) {
            if (p.second.value) {
                string v = p.second.get_preexisting_name_recursive(routes);
                exists = exists.empty() ? v: exists;
            }
        }
        return exists.empty()? exists: (name + "/" + exists);
    }
    return "";
}


router_tree::router_tree()
: root("")
{
}
void router_tree::add(string route, int qid)
{
    deque<string> routes;
    boost::split(routes, route, boost::is_any_of("/"));
    cout << "Adding route: " << route << endl;
    // check if route already exists
    if (root.exists_recursive(routes)) {
        string route_name = root.get_preexisting_name_recursive(routes);
        throw PreexistingRoute(route_name);
    }
    root.add_recursive(routes, qid);
}
int router_tree::get(string route)
{
    deque<string> routes;
    boost::split(routes, route, boost::is_any_of("/"));
    int qid = root.get_recursive(routes);
    if (qid == -1) {
        throw UnknownQuery(route);
    }
    return qid;
}
