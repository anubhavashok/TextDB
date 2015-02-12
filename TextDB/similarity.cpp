//
//  similarity.cpp
//  TextDB
//
//  Created by Anubhav on 1/29/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "similarity.h"
#include <iostream>
#include <vector>
#include <unordered_map>
#include <set>
#include <cmath>

namespace TF {
    std::unordered_map<std::string, int> term_frequency(const std::vector<std::string>& document) {
        std::unordered_map<std::string, int> tf;
        for (std::string w: document) {
            tf[w]++;
        }
        return tf;
    }
}

namespace TFIDF {
    std::unordered_map<std::string, double> tfidf(std::unordered_map<std::string, int> tf, std::unordered_map<std::string, double> idf) {
        std::unordered_map<std::string, double> tf_idf;
        for (auto p: tf) {
            std::string w = p.first;
            int tf_w = p.second;
            double idf_w = idf[w];
            double tf_idf_w = tf_w * idf_w;
            tf_idf[w] = tf_idf_w;
        }
        return tf_idf;
    }
}

namespace CosineSimilarity {
    std::function<double(std::vector<double>&, std::vector<double>&)> similarity = [](std::vector<double>& v1, std::vector<double>& v2){
        /*
         Cosine similarity
         ----------------------
         
         */
        int cardinality = (int) v1.size();
        
        /* Step 1: Calculate dot-product of vectors */
        int dot_product = 0;
        for (int i = 0; i < cardinality; i++) {
            dot_product += v1[i] * v2[i];
        }
        
        /* Step 2: Calculate norm of both vectors*/
        double ss1 = 0;
        for (double d: v1) {
            ss1 += d*d;
        }
        double norm1 = sqrt(ss1);
        
        double ss2 = 0;
        for (double d: v2) {
            ss2 += d*d;
        }
        double norm2 = sqrt(ss2);
        
        double score = dot_product/(norm1*norm2);
        return score;
    };
}

std::function<double(std::vector<std::string>, std::vector<std::string>)> document_similarity = [](std::vector<std::string> document1, std::vector<std::string> document2) {
    /*
     Similarity scoring of documents
     -----------------------------------
     Alternative implementations go here
     
     */
    
    /* Step 1: Remove stop words */
    document1 = Stopwords::remove_stopwords(document1);
    document2 = Stopwords::remove_stopwords(document2);
    
    /* Step 2: Calculate Term-Frequency */
    std::unordered_map<std::string, int> tf1 = TF::term_frequency(document1);
    std::unordered_map<std::string, int> tf2 = TF::term_frequency(document2);
    
    /* Step 3: Calculate Inverse Document Frequency */
    std::unordered_map<std::string, double> idf1 = IDF::inverse_document_frequency(document1);
    std::unordered_map<std::string, double> idf2 = IDF::inverse_document_frequency(document2);
    
    /* Step 4: Calculate Inverse Document Frequency */
    std::unordered_map<std::string, double> tf_idf1 = TFIDF::tfidf(tf1, idf1);
    std::unordered_map<std::string, double> tf_idf2 = TFIDF::tfidf(tf2, idf2);
    
    /* Step 5: Calculate Cosine Similarity */
    std::set<std::string> u;
    for (std::string w: document1) u.insert(w);
    for (std::string w: document2) u.insert(w);
    
    std::vector<double> v1;
    std::vector<double> v2;
    
    for (std::string w: u) {
        int s1 = tf_idf1.count(w)? tf_idf1[w]: 0;
        int s2 = tf_idf2.count(w)? tf_idf2[w]: 0;
        v1.push_back(s1);
        v2.push_back(s2);
    }
    
    double similarity = CosineSimilarity::similarity(v1, v2);
    return similarity;
};