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

#include "server.hpp"
using namespace std;
namespace fs = boost::filesystem;
namespace po = boost::program_options;

DB* db = nullptr;
static std::string dbpath;
const char* toolName = "tdb";


int main(int argc, char ** argv) {

    po::options_description desc("Welcome to TexteDB");
    Options options;
    po::variables_map vm = options.processCmdLine(argc, argv, desc);

    string config = options.config;
    fstream inconf(config);
    boost::property_tree::ptree pt;
    boost::property_tree::read_json(inconf, pt);
    
    fs::path datapth = pt.get<string>("data");
    cout << "datapath: " << datapth << endl;
    int candidateId = pt.get<int>("id");
    cout << "candidateId: " << candidateId << endl;
    int port = pt.get<int>("port");
    cout << "port: " << port << endl;
    
    vector<string> replicas;
    vector<int> replicaIds;
    for (auto& item: pt.get_child("replicas")) {
        string replica = item.second.get<string>("hostname");
        int replicaId = item.second.get<int>("id");
        replicas.push_back(replica);
        replicaIds.push_back(replicaId);
        cout << "replica: " << replica << endl;
        cout << "id: " << replicaId << endl;
    }
    
    //fs::path datapath = options.datapath;
    //cout << "Building DB: " << datapath << endl;;
    db = new DB(datapth, replicas, port, candidateId, replicaIds);

    assert(db != nullptr);

    try
    {
        // Initialise the server.
        http::server::server s("0.0.0.0", to_string(port), "");
        
        // Run the server until stopped.
        s.run();
    }
    catch (std::exception& e)
    {
        std::cerr << "exception: " << e.what() << "\n";
    }
}
