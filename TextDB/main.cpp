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
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

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
namespace fs = boost::filesystem;
namespace po = boost::program_options;

DB* db = nullptr;
static std::string dbpath;
const char* toolName = "tdb";


class RequestHandler : public HTTPRequestHandler
{
public:
    virtual void handleRequest(HTTPServerRequest &req, HTTPServerResponse &resp)
    {
        resp.setStatus(HTTPResponse::HTTP_OK);
        resp.setContentType("text/html");
        
        ostream& out = resp.send();
        
        
        std::string request_uri = req.getURI();
        /*
        Parameters
        NameValueCollection::ConstIterator i = req.begin();
        
        while(i!=form.end()){
            
            name=i->first;
            value=i->second;
            cout << name << "=" << value << endl << flush;
            ++i;
        }
         */
        
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
        while (true) {
            // if leader, call appendEntries RPC as heartbeat
            db->raft.leaderLoop();
        }
        
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
    TextDBServer app;
    std::vector<std::string> args;
    args.push_back(to_string(options.port));
    



    return app.run(args);

}
