#define BOOST_LOG_DYN_LINK 1

//
//  main.cpp
//  TextDB
//
//  Created by Anubhav on 8/24/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#include "options.h"
#include <iostream>
#include <vector>
#include <boost/program_options.hpp>
#include <bitset>
#include <boost/algorithm/string.hpp>
#include "db.h"
#include <ctime>
#include <boost/filesystem.hpp>
#include <fstream>
#include <signal.h>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <cstdlib>
#include "mutex_unordered_map.h"
#include "tagger.h"

#include "server.hpp"
using namespace std;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

shared_ptr<DB> db = nullptr;
static shared_ptr<DB> sdb(db);
static std::string dbpath;
const char* toolName = "tdb";

#define LOG_VAR_I(v) { printf( "%s: %i\n", #v, v); }
#define LOG_VAR_S(v) { printf( "%s: %s\n", #v, v.c_str()); }


int main(int argc, char ** argv) {
    // run_tests(&argc, argv);

    /* Program Options */
    Options options;
    po::options_description desc("Welcome to TexteDB");
    po::variables_map vm = options.processCmdLine(argc, argv, desc);

    /* Read config file */
    string config = options.config;
    boost::property_tree::ptree pt;
    fstream inconf(config);
    boost::property_tree::read_json(inconf, pt);
    
    /* Initialize variables */
    int port = pt.get<int>("port");
    fs::path datapath = pt.get<string>("data") + "/" + to_string(port);
    int candidateId = pt.get<int>("id");
    
    LOG_VAR_I(port);
    LOG_VAR_S(datapath);
    LOG_VAR_I(candidateId);
    
    /* RAFT variables */
    vector<string> replicas;
    vector<int> replicaIds;
    for (auto& item: pt.get_child("replicas")) {
        string replicaAddress = item.second.get<string>("hostname");
        int replicaId = item.second.get<int>("id");
        replicas.push_back(replicaAddress);
        replicaIds.push_back(replicaId);
        LOG_VAR_S(replicaAddress);
        LOG_VAR_I(replicaId);
    }
    vector<Tagger> taggers;
    for (auto& item: pt.get_child("taggers")) {
        string name = item.second.get<string>("name");
        int taggerPort = item.second.get<int>("port");
        string start = item.second.get<string>("start");
        Tagger t(name, taggerPort, start);
        taggers.push_back(t);
    }
    
    /* Initialize database object */
    db = shared_ptr<DB>(new DB(datapath, replicas, port, candidateId, replicaIds, taggers));
    assert(db != nullptr);

    try
    {
        // Initialise the server.
        http::server::server s("0.0.0.0", to_string(port), datapath.string());
        
        // Run the server until stopped.
        s.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << "\n";
    }
    
    // TR: Could possibly remove this
    cout << sdb.unique() << endl;
}
