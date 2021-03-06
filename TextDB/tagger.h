//
//  tagger.h
//  TextDB
//
//  Created by Anubhav on 11/19/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__tagger__
#define __TextDB__tagger__

#include <stdio.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include "tagger_message.pb.h"

using namespace std;

class Tagger
{
    enum TaggerStatus
    {
        UNKNOWN,
        DISABLED_REQ,
        DISABLED,
        STARTED,
        TRAINING,
        TRAINING_REQ, // Intermediate status set locally
        ACTIVE,
        ACTIVE_REQ
    };
    static unordered_map<string, TaggerStatus> tsParseMap;
    tagger_message::TagResult getTagResult(string addr);
public:
    // performs algorithm, returns tag
    int id;
    string name;
    string spawn_cmd;
    string endpoint;
    // loopback to htttp://localhost:port for now
    int port;
    TaggerStatus status = TaggerStatus::DISABLED;
    // Change status to STARTED once spawn is called
    Tagger(){}
    Tagger(string name, int port, string spawn_cmd);
    void spawn();
    void activate();
    void disable();
    vector<pair<string, size_t>> tag(tagger_message::DocumentFeature df);
    // If tagger is stateful, then train tagger
    // If tagger is static e.g. tfidf, then train is irrelevant
    // options may contain things like (overwrite old model, etc.)
    void train(vector<string> docs, vector<string> options)
    {
        // ensure tagger is of status=STARTED before starting training
        status = TaggerStatus::TRAINING_REQ;
        
    }
    // Only 1 instance of Tagger for now since train requires a consistent state
    static TaggerStatus parseStatus(string status)
    {
        
        return tsParseMap.count(status)? tsParseMap[status] : TaggerStatus::DISABLED;
    }
    string getDetails()
    {
        // custom information about tagger, can be anything as long as it is json
        // Has to be synchronous
        return "";
    }
};


#endif /* defined(__TextDB__tagger__) */
