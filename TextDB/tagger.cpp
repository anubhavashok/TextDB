//
//  tagger.cpp
//  TextDB
//
//  Created by Anubhav on 11/19/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "tagger.h"
#include <curlpp/cURLpp.hpp>

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


void Tagger::activate()
{
    // send get request to
    string url = endpoint + "/" + "active_ack";
    
}

void Tagger::disable()
{
    
}