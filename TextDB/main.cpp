//
//  main.cpp
//  TextDB
//
//  Created by Anubhav on 8/24/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//


#include <iostream>
#include <vector>

#include <boost/program_options.hpp>

using namespace std;

namespace po = boost::program_options;

const char* toolName = "tdb";

namespace {

    /*! @brief Define command line options for this entrypoint */
    struct Options {

        po::variables_map processCmdLine(int argc, char **argv, po::options_description& desc)
        {
            po::variables_map vm;
            po::options_description inputOpts("Input settings");
            inputOpts.add_options()

            //("corpusPath,c",po::value(&corpusPath), "Path to a corpus file")
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
            po::notify(vm);
            return vm;
        }
    };
}

// TODO: enable regex
// 1. parse
// 2. run queries


#include <bitset>
#include <boost/algorithm/string.hpp>
#include "db.h"

int main(int argc, char ** argv) {

    po::options_description desc("Shows the search space");
    Options options;
    po::variables_map vm = options.processCmdLine(argc, argv, desc);
    DB db;
    // listen for requests
    while (!cin.eof()) {
        cout << "Query: ";
        std::string raw;
        std::getline(std::cin, raw);
        std::vector<std::string> in;
        boost::split(in, raw, boost::is_any_of(" "));
        db.handleQuery(in);
    }
    std::string dbpath("/Users/anubhav/TextDB/store.bindb");
    std::string uncompresseddbpath("/Users/anubhav/TextDB/store.text");
    db.encodeAndSave(dbpath);
    db.saveUncompressed(uncompresseddbpath);
    cout << "stored data at: " << dbpath << endl;
    
    db.printIndex();
    
    DB newdb;
    newdb.decodeAndLoad(dbpath);
    cout << "loaded data from: " << dbpath << endl;
    newdb.printIndex();
}
