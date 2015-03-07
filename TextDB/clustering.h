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
    vector<Element> elements;
    double dist(Element e);
public:
    HeirarchicalCluster(const vector<Element>& elements);
    HeirarchicalCluster(HeirarchicalCluster& c1, HeirarchicalCluster& c2);
    bool marked;
};

template <template <typename> class Cluster, class Element>
class Clustering
{
private:
    vector <Cluster<Element>> clusters;
    vector <Element> elements;
public:
    Clustering(vector<Element> elements);
    void cluster();
    // level corressponds to the number of elements in each cluster or the number of total clusters
    // number of elements in each cluster = 2^level, number of total clusters = N/2^level
    // where level <= log2(N)
    vector <Cluster<Element>> getCusters(int level);
};

template <class Element>
class KMeansCluster
{
    // define centroid as union of all elements
    set<Element> centroid;
    void add(const KMeansCluster<Element>& c);
    void cluster();
    double dist(Element e);
    bool marked;
};


#endif /* defined(__TextDB__clustering__) */
