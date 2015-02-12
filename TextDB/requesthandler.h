//
//  requesthandler.h
//  TextDB
//
//  Created by Anubhav on 1/24/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__requesthandler__
#define __TextDB__requesthandler__

#include <iostream>
#include "Poco/Net/HTTPRequestHandler.h"
#include "Poco/Net/HTTPServerResponse.h"
#include "Poco/Net/HTTPServerRequest.h"
#include <boost/algorithm/string.hpp>


using namespace Poco::Net;
using namespace std;

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
        
        // db->handleQuery(args, out)
        out.flush();
        
        cout << endl
        << " and URI=" << req.getURI() << endl;
    }
};

#endif /* defined(__TextDB__requesthandler__) */
