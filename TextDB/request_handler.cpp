//
// request_handler.cpp
// ~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "request_handler.hpp"
#include <fstream>
#include <sstream>
#include <string>
#include "mime_types.hpp"
#include "reply.hpp"
#include "request.hpp"
#include <boost/algorithm/string.hpp>
#include <iostream>
#include "db.h"
#include "api.h"
using namespace std;

extern DB* db;

namespace http {
namespace server {

API api;

request_handler::request_handler(const std::string& doc_root)
  : doc_root_(doc_root)
{
}


void request_handler::handle_request(const request& req, reply& rep)
{
  // Decode url to path.
  std::string request_path;
  if (!url_decode(req.uri, request_path))
  {
    rep = reply::stock_reply(reply::bad_request);
    return;
  }
    std::vector<std::string> in;
    boost::split(in, req.uri, boost::is_any_of("&/"));
    in.erase(in.begin() + 0);
    
    cout << endl
    << "(TextDB): " << req.method << " " << req.uri << endl;
    stringstream out;
    db->handleQuery(in, out);
    api.accept(req.uri, out, db);

  // Fill out the reply to be sent to the client.
  rep.status = reply::ok;
  char buf[512];
  while (out.read(buf, sizeof(buf)).gcount() > 0)
    rep.content.append(buf, out.gcount());
  rep.headers.resize(3);
  rep.headers[0].name = "Content-Length";
  rep.headers[0].value = std::to_string(rep.content.size());
  rep.headers[1].name = "Content-Type";
  // return as plain text
  rep.headers[1].value = mime_types::extension_to_type("");
  rep.headers[2].name = "Access-Control-Allow-Origin";
  rep.headers[2].value = "*";
}

bool request_handler::url_decode(const std::string& in, std::string& out)
{
  out.clear();
  out.reserve(in.size());
  for (std::size_t i = 0; i < in.size(); ++i)
  {
    if (in[i] == '%')
    {
      if (i + 3 <= in.size())
      {
        int value = 0;
        std::istringstream is(in.substr(i + 1, 2));
        if (is >> std::hex >> value)
        {
          out += static_cast<char>(value);
          i += 2;
        }
        else
        {
          return false;
        }
      }
      else
      {
        return false;
      }
    }
    else if (in[i] == '+')
    {
      out += ' ';
    }
    else
    {
      out += in[i];
    }
  }
  return true;
}

} // namespace server
} // namespace http
