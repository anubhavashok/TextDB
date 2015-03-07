//
//  clustering.cpp
//  TextDB
//
//  Created by Anubhav on 1/27/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "clustering.h"

template <class Element>
HeirarchicalCluster<Element>::HeirarchicalCluster(const vector<Element>& _elements)
{
    marked = false;
    elements = _elements;
}

template <class Element>
HeirarchicalCluster<Element>::HeirarchicalCluster(HeirarchicalCluster& h1, HeirarchicalCluster& h2)
{
    marked = false;
    for (Element& e: h1.elements) {
        elements.push_back(e);
    }
    for (Element& e: h2.elements) {
        elements.push_back(e);
    }
}

template <class Element>
double HeirarchicalCluster<Element>::dist(Element e)
{
    
}
/*
template <template <typename> class Cluster, class Element>
void Clustering<Cluster<Element>>::cluster()
{
    vector <Cluster<Element>> new_clusters;
    
    for (int i = 0; i < clusters.size()-1; i++) {
        Cluster<Element> c = clusters[i];
        Cluster<Element> nc = clusters[i+1];
        for (int j = 0; j < i; j++) {
            if (!clusters[j].marked) {
            
            }
            // find
            new_clusters.push_back(nc);
            
        }
        // merge nc with c
    }
    // for each cluster, find closest pair, merge, remove cluster from old vector, combine both clusters, add to new vector
    // repeat for each element
}

*/

template <class Element>
void KMeansCluster<Element>::add(const KMeansCluster<Element>& c)
{
    centroid = centroid.insert(centroid.begin(), c.centroid.begin(), c.centroid.end());
}

template <class Element>
double KMeansCluster<Element>::dist(Element e)
{
    // JaccardDistance
}