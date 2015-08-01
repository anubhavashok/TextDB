//
//  error.cpp
//  TextDB
//
//  Created by Anubhav on 7/31/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "error.h"
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>

error::error(string msg, int code)
: msg(msg), code(code)
{
}
string error::to_string()
{
    return msg;
}
string error::to_json()
{
    boost::property_tree::ptree json;
    json.add("msg", msg);
    json.add("err_code", code);
    stringstream ss;
    boost::property_tree::json_parser::write_json(ss, json);
    return ss.str();
}