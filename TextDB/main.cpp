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
#include "fcgio.h"
#include <ctime>
#include <boost/filesystem.hpp>
#include <fstream>
#include "preformatter.h"
#include <signal.h>

using namespace std;
namespace fs=boost::filesystem;
namespace po = boost::program_options;

DB* db = nullptr;
static std::string dbpath;
const char* toolName = "tdb";


// TODO: VARY WORD INDEX DEPENDING ON NUMBER OF UNIQUE WORDS THERE ARE
// TODO: Make word-text mappings persistent
// TODO: make a preformatter to preformat all text before saving
// TODO: growing index for chars too
// TODO: Save new line chars as idx 27, periods as idx 28, commas as idx 29,
// TODO: if doc is modified, remove cache


/*
 * DBSigHandler
 * A function that encodes and saves DB to file on SIGTERM
 * @param signum an int that represents the signal recieved
 * In this case signum corressponds to SIGTERM (15)
 */

void DBSigHandler(int signum);
void DBSigHandler(int signum)
{
    // safe to exit since stuff is persisted on write
    exit(0);
}

int main(int argc, char ** argv) {
    
    po::options_description desc("Welcome to TexteDB");
    Options options;
    po::variables_map vm = options.processCmdLine(argc, argv, desc);
    signal(SIGTERM, DBSigHandler);
    fs::path datapath = options.datapath;
    dbpath = options.dbpath;
    db = new DB(datapath);
    
    assert(db != nullptr);
    
    if (Options::verbose) {
        cout << "Initializing DB object...";
        cout << "done" << endl;
        cout << "Decoding and loading db file...";
        cout << "done" << endl;
    }
    
    // Backup the stdio streambufs
    streambuf * cin_streambuf  = cin.rdbuf();
    streambuf * cout_streambuf = cout.rdbuf();
    streambuf * cerr_streambuf = cerr.rdbuf();
    
    FCGX_Request request;
    
    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);
    std::string request_uri = "";
    
    while (FCGX_Accept_r(&request) == 0) {
        
        fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);
        
        request_uri = FCGX_GetParam("REQUEST_URI", request.envp);
        request_uri = request_uri.substr(1, request_uri.size());
        std::vector<std::string> in;
        boost::split(in, request_uri, boost::is_any_of("&/"));

        // set buffers
        cin.rdbuf(&cin_fcgi_streambuf);
        cout.rdbuf(&cout_fcgi_streambuf);
        cerr.rdbuf(&cerr_fcgi_streambuf);
        
        std::string cmd = in[0];
        cout << "Content-type: text/plain\r\n"
        << "\r\n";

        db->handleQuery(in, cout);
    }
    // restore stdio streambufs
    cin.rdbuf(cin_streambuf);
    cout.rdbuf(cout_streambuf);
    cerr.rdbuf(cerr_streambuf);

}
