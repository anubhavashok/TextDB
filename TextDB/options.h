//
//  options.h
//  TextDB
//
//  Created by Anubhav on 8/29/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#ifndef TextDB_options_h
#define TextDB_options_h


#include "options.h"
#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

namespace po = boost::program_options;
using namespace std;

namespace {
    
    /*! @brief Define command line options for this entrypoint */
    struct Options {
        
        std::string datapath;         // path to data - sentiment analysis etc.
        static bool verbose;          // verbose flag
        size_t memory_limit;          // memory_limit
        int port;
        vector<string> replicas;      // ip addresses of replicas
        int candidateId;
        string config;
        
        po::variables_map processCmdLine(int argc, char **argv, po::options_description& desc)
        {
            po::variables_map vm;
            po::options_description inputOpts("Input settings");
            inputOpts.add_options()
            ("id,i", po::value(&candidateId)->required(), "Id of candidate (REQUIRED)")
            ("datapath,a", po::value(&datapath)->required(), "Path to data files (REQUIRED)")
            ("mlimit,m", po::value(&memory_limit), "Memory limit in bytes")
            ("replicas,r", po::value(&replicas), "Ip addresses of replicas")
            ("port,p", po::value(&port)->required(), "Port for TextDB IO (REQUIRED)")
            ("verbose,v", "Verbose")
            ("config,c", po::value(&config)->required(), "Configuration file (REQUIRED)")
            ;
            
            desc.add_options()
            ("help,h", "Print this help")
            ;
            desc.add(inputOpts);

            po::store(po::parse_command_line(argc, argv, desc), vm);
            
            // handle command-line options
            if (vm.count("help")) {
                std::cout << desc << std::endl;
                exit(0);
            }
            Options::verbose = vm.count("verbose");
            try {
                po::notify(vm);
            } catch (exception& e) {
                std::cout << desc << std::endl;
                exit(0);
                
            }
            return vm;
        }
    };
    bool Options::verbose;
}


#endif
