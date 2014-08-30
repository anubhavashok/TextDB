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

        std::string dbpath;
        
        po::variables_map processCmdLine(int argc, char **argv, po::options_description& desc)
        {
            po::variables_map vm;
            po::options_description inputOpts("Input settings");
            inputOpts.add_options()
            ("dbpath,d",po::value(&dbpath), "Path to a database file")
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
#include "fcgio.h"
#include <ctime>
#include <boost/filesystem.hpp>
#include <fstream>


int main(int argc, char ** argv) {
    std::string dbpath("/Users/anubhav/TextDB/store.bindb");
    DB db;
    if (boost::filesystem::exists(dbpath)) {
        db.decodeAndLoad(dbpath);
    } else {
        // just create file
        ofstream dbfile(dbpath);
        dbfile.close();
    }
    time_t lastSaveTime(time(0));
    // Backup the stdio streambufs
    streambuf * cin_streambuf  = cin.rdbuf();
    streambuf * cout_streambuf = cout.rdbuf();
    streambuf * cerr_streambuf = cerr.rdbuf();
    
    FCGX_Request request;
    
    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);
    const char * query_string = "";
    while (FCGX_Accept_r(&request) == 0) {
        fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);

        cin.rdbuf(&cin_fcgi_streambuf);
        cout.rdbuf(&cout_fcgi_streambuf);
        cerr.rdbuf(&cerr_fcgi_streambuf);
        
        //char** env = request.envp;
        //while (*(++env))
            //cout << *env << endl;
        query_string = FCGX_GetParam("QUERY_STRING", request.envp);
        std::vector<std::string> in;
        boost::split(in, query_string, boost::is_any_of("&"));
        db.handleQuery(in);
        
        cout << "successful query: " << query_string << endl;
        
        cout << "Content-type: text/html\r\n"
        << "\r\n"
        << "<html>\n"
        << "  <head>\n"
        << "    <title>TextDB</title>\n"
        << "  </head>\n"
        << "  <body>\n"
        << "    <h1>Welcome to TextDB</h1>\n"
        << "    <p>To add a document to the database, append '?adddoc&(docname)&(docpath)' to base url</p>\n"
        << "  </body>\n"
        << "</html>\n";
        
        // Note: the fcgi_streambuf destructor will auto flush
        time_t currentTime(time(0));
        if (currentTime - lastSaveTime >= 5000) {
            std::string uncompresseddbpath("/Users/anubhav/TextDB/store.text");
            db.encodeAndSave(dbpath);
            db.saveUncompressed(uncompresseddbpath);
            lastSaveTime = currentTime;
        }
    }
    // restore stdio streambufs
    cin.rdbuf(cin_streambuf);
    cout.rdbuf(cout_streambuf);
    cerr.rdbuf(cerr_streambuf);
    cout << "done " << query_string << endl;
    /*

    po::options_description desc("Shows the search space");
    Options options;
    po::variables_map vm = options.processCmdLine(argc, argv, desc);
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
    cout << endl << "stored data at: " << dbpath << endl;
    
    DB newdb;
    newdb.decodeAndLoad(dbpath);
    cout << "loaded data from: " << dbpath << endl;
     */
}
