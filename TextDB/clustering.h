//
//  clustering.h
//  TextDB
//
//  Created by Anubhav on 1/27/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__clustering__
#define __TextDB__clustering__

#include <stdio.h>
#include <iostream>
#include <vector>
#include <set>

using namespace std;


template <class Element>
class HeirarchicalCluster
{
    std::vector<Element&> elements;
    HeirarchicalCluster(HeirarchicalCluster& c1, HeirarchicalCluster& c2);
    double dist(Element e);
};

template <class Element>
class KMeansCluster
{
    // define centroid as union of all elements
    std::set<Element> centroid;
    void add(KMeansCluster<Element>& c);
    double dist(Element e);
};



#endif /* defined(__TextDB__clustering__) */
