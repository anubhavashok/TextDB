//
//  html.h
//  TextDB
//
//  Created by Anubhav on 1/25/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__html__
#define __TextDB__html__

#include <iostream>
#include <Poco/Net/HTTPRequest.h>
#include <Poco/Net/HTTPClientSession.h>
#include <Poco/Net/HTTPResponse.h>

class HTML
{
public:
    static std::string get(std::string uri);
    static std::string parseText(std::string html);
};


#endif /* defined(__TextDB__html__) */
