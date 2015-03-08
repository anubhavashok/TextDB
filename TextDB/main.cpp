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
#include <signal.h>

#include "Poco/Net/HTTPServer.h"
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPRequestHandlerFactory.h"
#include "Poco/Net/HTTPResponse.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerRequest.h"
#include "Poco/Util/ServerApplication.h"

using namespace Poco::Net;
using namespace Poco::Util;
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

class RequestHandler : public HTTPRequestHandler
{
public:
    virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp)
    {
        resp.setStatus(HTTPResponse::HTTP_OK);
        resp.setContentType("text/html");
        
        ostream& out = resp.send();
        
        
        std::string request_uri = req.getURI();
        
        std::vector<std::string> in;
        boost::split(in, request_uri, boost::is_any_of("&/"));
        in.erase(in.begin() + 0);

        cout << endl
        << "(TextDB): " << req.getMethod() << " " << req.getURI() << endl;

        db->handleQuery(in, out);
        out.flush();
        
    }
};

class RequestHandlerFactory : public HTTPRequestHandlerFactory
{
public:
    virtual HTTPRequestHandler* createRequestHandler(const HTTPServerRequest &)
    {
        return new RequestHandler;
    }
};

class TextDBServer : public ServerApplication
{
protected:
    int main(const vector<string> & args)
    {
        HTTPServer s(new RequestHandlerFactory, ServerSocket(stoi(args[0])), new HTTPServerParams);
        
        s.start();
        cout << endl << "Welcome to TextDB! [" << s.port() << "]" << endl;
        
        waitForTerminationRequest();  // wait for CTRL-C or kill
        
        cout << endl << "Shutting down..." << endl;
        s.stop();
        
        return Application::EXIT_OK;
    }
};


int main(int argc, char ** argv) {
    
    po::options_description desc("Welcome to TexteDB");
    Options options;
    po::variables_map vm = options.processCmdLine(argc, argv, desc);

    fs::path datapath = options.datapath;
    dbpath = options.dbpath;
    cout << "Building DB: " << datapath << endl;;
    db = new DB(datapath, options.replicas);

    assert(db != nullptr);
    TextDBServer app;
    std::vector<std::string> args;
    args.push_back(to_string(options.port));
    
    return app.run(args);

}
