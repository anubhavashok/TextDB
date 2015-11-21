//
//  tagger.cpp
//  TextDB
//
//  Created by Anubhav on 11/19/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "tagger.h"
#include <curlpp/cURLpp.hpp>
#include <curlpp/Options.hpp>
#include <curlpp/Easy.hpp>
#include <sstream>
#include <stdio.h>
#include <cstdlib>

// TODO: use boost async to handle this, especially spawning

unordered_map<string, Tagger::TaggerStatus> Tagger::tsParseMap = unordered_map<string, Tagger::TaggerStatus> {
    {"UNKNOWN", TaggerStatus::UNKNOWN},
    {"DISABLED_REQ", TaggerStatus::DISABLED_REQ},
    {"DISABLED", TaggerStatus::DISABLED},
    {"STARTED", TaggerStatus::STARTED},
    {"TRAINING", TaggerStatus::TRAINING},
    {"TRAINING_REQ", TaggerStatus::TRAINING_REQ},
    {"ACTIVE", TaggerStatus::ACTIVE},
    {"ACTIVE_REQ", TaggerStatus::ACTIVE_REQ}
};

Tagger::Tagger(string name, int port, string spawn_cmd)
{
    endpoint = "http://localhost:"+to_string(port);
}

void Tagger::spawn()
{
    status = TaggerStatus::UNKNOWN;
    // send Tagger ACK endpoint as well
    // So once taggers starts, the first thing it should do is send an ack to the db
    // Custom code goes here, tagger can either be on same machine,
    // In which case, we just spawn process from here
    // If it is on another machine, then send spawn request to client on the machine
    // Let that handle the spawning
    
    // run spawn command on system? Learn how to spawn something like this properly
    std::system(spawn_cmd.c_str());
}

void Tagger::activate()
{
    status = ACTIVE_REQ;
    // send get request to
    string url = endpoint + "/" + "active_ack";
    tagger_message::TagResult tagResult = getTagResult(url);
    if (tagResult.success()) {
        status = ACTIVE;
    }
}

void Tagger::disable()
{
    status = DISABLED_REQ;
    // send get request to
    string url = endpoint + "/" + "disabled_ack";
    tagger_message::TagResult tagResult = getTagResult(url);
    if (tagResult.success()) {
        status = DISABLED_REQ;
    }
}

tagger_message::TagResult Tagger::getTagResult(string addr)
{
    stringstream os;
    tagger_message::TagResult tagResult = tagger_message::TagResult();
    try {
        curlpp::Cleanup myCleanup;
        os << curlpp::options::Url(addr);
        tagResult.ParseFromIstream(&os);
    } catch(curlpp::RuntimeError & e) {
        std::cout << e.what() << std::endl;
    } catch(curlpp::LogicError & e) {
        std::cout << e.what() << std::endl;
    }
    return tagResult;
}