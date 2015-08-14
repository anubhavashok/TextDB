//
//  similarity.h
//  TextDB
//
//  Created by Anubhav on 1/29/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__similarity__
#define __TextDB__similarity__

#include <stdio.h>



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
#include <boost/algorithm/string.hpp>

namespace TF {
    /*
     Term Frequency
     ----------------------
     
     Builds a frequency table based on words in document
     
     Input:
     document = {"Hi", "welcome", "to", "text", "db", "welcome", "to", "this"};
     Output:
     ________________
     Hi      |   1
     welcome |   2
     to      |   2
     text    |   1
     db      |   1
     this    |   1
     */
    
    auto term_frequency = [](const std::vector<std::string>& document) {
        std::unordered_map<std::string, boost::uintmax_t> tf;
        for (std::string w: document) {
            boost::to_lower(w);
            if (!tf.count(w)) tf[w] = 0;
            tf[w]++;
        }
        return tf;
    };
    
};

namespace TFIDF {
    /*
     Term Frequency - Inverse Document Frequency
     ----------------------
     
     Calculates TF-IDF score based on tf and idf table
     
     */
    
    static auto tfidf = [](const std::unordered_map<std::string, boost::uintmax_t>& tf, const std::unordered_map<std::string, double>& idf) {
        std::unordered_map<std::string, double> tf_idf;
        for (auto p: tf) {
            std::string w = p.first;
            boost::uintmax_t tf_w = p.second;
            double idf_w = idf.at(w);
            double tf_idf_w = tf_w * idf_w;
            tf_idf[w] = tf_idf_w;
        }
        return tf_idf;
    };
};

namespace CosineSimilarity {
    static std::function<double(std::vector<double>&, std::vector<double>&)> similarity = [](std::vector<double>& v1, std::vector<double>& v2){
        /*
         Cosine similarity
         ----------------------
         
         */
        boost::uintmax_t cardinality = v1.size();
        
        /* Step 1: Calculate dot-product of vectors */
        double dot_product = 0;
        for (boost::uintmax_t i = 0; i < cardinality; i++) {
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
};

namespace Stopwords {
    /*
     Stopwords
     ----------------------
     
     */
    
    using namespace std;
    static vector<string> stopwords;
    //static vector<string> remove_stopwords(vector<string> document);
    static auto remove_stopwords = [](vector<string> document)
    {
        /* Step 1: Iterate through each stopword */
        for (string s: stopwords) {
            /* Step 2: Remove stopword s from document */
            document.erase(remove(document.begin(), document.end(), s), end(document));
        }
        return document;
    };
}

namespace IDF {
    /*
     Inverse Document Frequency
     ----------------------
     
     */
    using namespace std;
    static function<unordered_map<string, double>(vector<string>, const vector<vector<string>>&)> inverse_document_frequency = [](vector<string> document, const vector<vector<string>>& docs)
    {
        unordered_map<string, double> idf;
        size_t N = docs.size();
        /* Step 1: Find number of documents with term t */
        for (string t: document) {
            boost::to_lower(t);
            if (!idf.count(t)) {
                boost::uintmax_t dft = 0;
                for (const vector<string>& d: docs) {
                    auto e = d.end();
                    if(find(begin(d), end(d), t) != e) {
                        dft++;
                    }
                }
                /* Step 2: Calculate idf for given term */
                cout << "Term: " << t << " Number of documents:" << N << "," << " DFT: " << dft << endl;
                idf[t] = log((double)N/dft);
            }
        }
        return idf;
    };
};

namespace Similarity {
    using namespace IDF;
    using namespace TFIDF;
    using namespace TF;
    using namespace IDF;
    using namespace Stopwords;
    using namespace CosineSimilarity;


    std::function<double(std::vector<std::string>, std::vector<std::string>, const std::vector<std::vector<std::string>>&)> document_similarity = [](std::vector<std::string> document1, std::vector<std::string> document2, const std::vector<std::vector<std::string>>& docs) {
        /*
         Similarity scoring of documents
         -----------------------------------
         Alternative implementations go here
         
         */
        
        /* Step 1: Remove stop words */
        document1 = Stopwords::remove_stopwords(document1);
        document2 = Stopwords::remove_stopwords(document2);
        
        /* Step 2: Calculate Term-Frequency */
        std::unordered_map<std::string, boost::uintmax_t> tf1 = TF::term_frequency(document1);
        std::unordered_map<std::string, boost::uintmax_t> tf2 = TF::term_frequency(document2);
        
        /* Step 3: Calculate Inverse Document Frequency */
        std::unordered_map<std::string, double> idf1 = IDF::inverse_document_frequency(document1, docs);
        std::unordered_map<std::string, double> idf2 = IDF::inverse_document_frequency(document2, docs);
        
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
            double s1 = tf_idf1.count(w)? tf_idf1[w]: 0;
            double s2 = tf_idf2.count(w)? tf_idf2[w]: 0;
            v1.push_back(s1);
            v2.push_back(s2);
        }
        
        double similarity = CosineSimilarity::similarity(v1, v2);
        return similarity;
    };
}
#endif /* defined(__TextDB__similarity__) */
