//
//  service_discovery.h
//  TextDB
//
//  Created by Anubhav on 12/30/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__service_discovery__
#define __TextDB__service_discovery__

#include <stdio.h>
#include <map>
using namespace std;

map<string, string> discover_services(string kubernetes_addr);

#endif /* defined(__TextDB__service_discovery__) */
