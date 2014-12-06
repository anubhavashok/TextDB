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

namespace {
    
    /*! @brief Define command line options for this entrypoint */
    struct Options {
        
        // path to store.bindb
        std::string dbpath;
        
        // path to data - sentiment analysis etc.
        std::string datapath;
        
        // verbose flag
        static bool verbose;
        
        po::variables_map processCmdLine(int argc, char **argv, po::options_description& desc)
        {
            po::variables_map vm;
            po::options_description inputOpts("Input settings");
            inputOpts.add_options()
            ("dbpath,d",po::value(&dbpath), "Path to a database file")
            ("datapath,a",po::value(&datapath), "Path to data files")
            ("verbose,v","(optional) verbose")
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
            if (vm.count("verbose")) {
                Options::verbose = true;
            } else {
                Options::verbose = false;
            }
            po::notify(vm);
            return vm;
        }
    };
    bool Options::verbose;
}


#endif
