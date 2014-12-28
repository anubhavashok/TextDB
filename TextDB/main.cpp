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

using namespace std;

namespace po = boost::program_options;
const char* toolName = "tdb";


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
#include "preformatter.h"
#include <signal.h>

// TODO: VARY WORD INDEX DEPENDING ON NUMBER OF UNIQUE WORDS THERE ARE
// TODO: Make word-text mappings persistent
// TODO: make a preformatter to preformat all text before saving
// TODO: growing index for chars too
// TODO: Save new line chars as idx 27, periods as idx 28, commas as idx 29, 

// Globals in order to handle SIGTERM signals
namespace fs=boost::filesystem;
DB* db = nullptr;
static std::string dbpath;

/*
 * DBSigHandler
 * A function that encodes and saves DB to file on SIGTERM
 * @param signum an int that represents the signal recieved
 * In this case signum corressponds to SIGTERM (15)
 */

void DBSigHandler(int signum);
void DBSigHandler(int signum)
{
    fs::path uncompresseddbpath = fs::path(dbpath).parent_path() / "store.text";
    db->encodeAndSave(dbpath);
    db->saveUncompressed(uncompresseddbpath.string());
    exit(0);
}

int main(int argc, char ** argv) {
    po::options_description desc("Shows the search space");
    Options options;
    po::variables_map vm = options.processCmdLine(argc, argv, desc);
    signal(SIGTERM, DBSigHandler);

    fs::path datapath = options.datapath;
    
    dbpath = options.dbpath;
    db = new DB(datapath);
    assert(db != nullptr);
    if (Options::verbose) {
        cout << "Initializing DB object...";
    }
    if (Options::verbose) {
        cout << "done" << endl;
    }
    
    if (Options::verbose) {
        cout << "Decoding and loading db file...";
    }
    if (boost::filesystem::exists(dbpath)) {
        db->decodeAndLoad(dbpath);
    } else {
        // just create file
        fstream dbfile;
        dbfile.open(dbpath, ios::out);
        dbfile.close();
        cout << "Created new db file at " << dbpath << "..." << endl;
    }
    if (Options::verbose) {
        cout << "done" << endl;
    }
    
    // Backup the stdio streambufs
    streambuf * cin_streambuf  = cin.rdbuf();
    streambuf * cout_streambuf = cout.rdbuf();
    streambuf * cerr_streambuf = cerr.rdbuf();
    
    FCGX_Request request;
    
    FCGX_Init();
    FCGX_InitRequest(&request, 0, 0);
    std::string query_string = "";
    
    while (FCGX_Accept_r(&request) == 0) {
        
        fcgi_streambuf cin_fcgi_streambuf(request.in);
        fcgi_streambuf cout_fcgi_streambuf(request.out);
        fcgi_streambuf cerr_fcgi_streambuf(request.err);
        
        query_string = FCGX_GetParam("REQUEST_URI", request.envp);
        query_string = query_string.substr(1, query_string.size());
        std::vector<std::string> in;
        boost::split(in, query_string, boost::is_any_of("&/"));

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
