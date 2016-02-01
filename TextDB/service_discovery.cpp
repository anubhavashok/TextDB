//
//  service_discovery.cpp
//  TextDB
//
//  Created by Anubhav on 12/30/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "service_discovery.h"
#include <curlpp/Easy.hpp>
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <map>
#include <sstream>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/algorithm/string.hpp>
#include <boost/foreach.hpp>
#include <signal.h>


using namespace std;


map<string, string> discover_services(string kubernetes_addr)
{
    map<string, string> tagger_ips;
    string url = kubernetes_addr + "/api/v1/services/";
    std::stringstream os;
    
    os << curlpp::options::Url(url);
    
    boost::property_tree::ptree pt;
    boost::property_tree::json_parser::read_json(os, pt);
    
    // services['items'][2]['status']['loadBalancer']['ingress'][0]['ip']
    auto services = pt.get_child("items");
    BOOST_FOREACH(boost::property_tree::ptree::value_type& item, services)
    {
        string key = item.second.get_child("metadata.name").get_value<string>();
        vector<string> tokens;
        boost::split(tokens, key, boost::is_any_of("_"));
        if (tokens.back() == "tagger") {
            string name = key.substr(0, key.length() - 7);
            string ip = item.second.get_child("status.loadBalancer.ingress..ip").get_value<string>();
            tagger_ips[name] = ip;
            cout << "name: " << key << " " << ip << endl;
        }
    }
    return tagger_ips;
}
