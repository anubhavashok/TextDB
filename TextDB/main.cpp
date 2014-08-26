//
//  main.cpp
//  TextDB
//
//  Created by Anubhav on 8/24/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//


#include <iostream>
#include <map>
#include <vector>
#include <boost/thread/thread.hpp>


#include <iostream>
#include <boost/program_options.hpp>


using namespace std;

namespace po = boost::program_options;

const char* toolName = "tdb";

namespace {

    /*! @brief Define command line options for this entrypoint */
    struct Options {
        std::vector<std::string> corpusPath;
        std::string dbPath;
        std::string outputDbPath;
        std::string db;
        bool repin = false;

        po::variables_map processCmdLine(int argc, char **argv, po::options_description& desc)
        {
            po::variables_map vm;
            po::options_description inputOpts("Input settings");
            inputOpts.add_options()

            ("corpusPath,c",po::value(&corpusPath), "Path to a corpus file")
            ("dbPath,d", po::value(&dbPath), "Path of database")
            ("outputDbPath,o", po::value(&outputDbPath), "(optional) Path to output db")
            // example to add flags
            // ("repin,p","(optional) flag to repin coastedPath headings")
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
            if (vm.count("repin")) {
                repin = true;
            }
            if (!(vm.count("corpusPath") || (vm.count("dbPath")))) {
                throw po::error("Include corpusPath or dbPath");
            }
            po::notify(vm);
            return vm;
        }
    };
}

// TODO: enable regex
// 1. parse
// 2. run queries

int main(int argc, char ** argv) {

    po::options_description desc("Shows the search space");
    Options options;
    po::variables_map vm = options.processCmdLine(argc, argv, desc);
    // listen for requests
    
    
}
