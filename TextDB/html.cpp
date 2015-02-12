//
//  html.cpp
//  TextDB
//
//  Created by Anubhav on 1/25/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "html.h"
#include <stack>
#include <Poco/URI.h>
#include <sstream>

using namespace Poco::Net;
using namespace std;

std::string HTML::get(std::string uristring)
{
    std::string page = "";
    Poco::URI uri(uristring);
    HTTPClientSession session(uri.getHost(), uri.getPort());
    HTTPRequest request(HTTPRequest::HTTP_GET, uri.getPathAndQuery(), HTTPRequest::HTTP_1_1);
    HTTPResponse response;

    if(!session.sendRequest(request)) cout << "Error!" << endl;
    std::istream& rs = session.receiveResponse(response);
    std::cout << response.getStatus() << " " << response.getReason() << std::endl;
    if (response.getStatus() != Poco::Net::HTTPResponse::HTTP_UNAUTHORIZED)
    {
        cout << "OK page has been retreived" << endl;
        stringstream s;
        s << rs.rdbuf();
        page = s.str();
    }
    else
    {
        cout << "(TextDB): GET request failed!" << endl;
    }
    return page;
}

bool textTag(std::string tagText)
{
    std::string tag = "";
    int i = 0;
    while (i < tagText.size() && tagText[i] != ' ') {
        tag += tagText[i];
        i++;
    }
    bool isText = (tag != "script") && (tag != "style");
    if (isText) cout << tag << endl;
    return isText;
}

// Highly experimental, do not use in production
std::string HTML::parseText(std::string html)
{
    std::stack<std::string> tagStack;
    std::stack<bool> captureStack;
    captureStack.push(true);
    std::string text;
    bool startTag = false;
    std::string tag = "";
    
    for (char c: html) {
        // start of a tag
        if (c == '<') {
            tag = "";
            startTag = true;
        } else if (startTag) {
            if (c == '>') {
                startTag = false;
                if (tag.back() == '\\' || tag.front() == '/') {
                    captureStack.pop();
                    tagStack.pop();
                } else {
                    // TODO: closing tag expected
                    tagStack.push(tag);
                    if (textTag(tag)) {
                        captureStack.push(true);
                        text += ' ';
                    }
                    else captureStack.push(false);
                }
            } else {
                tag += c;
            }
        } else if (captureStack.top()) {
            text += c;
        }
    }
    return text;
}
