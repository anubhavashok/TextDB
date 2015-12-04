//
//  db.h
//  TextDB
//
//  Created by Anubhav on 8/26/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#ifndef __TextDB__db__
#define __TextDB__db__

#include <iostream>
#include <fstream>
#include <unordered_map>
#include <vector>
#include <boost/dynamic_bitset.hpp>
#include "sentiment.h"
#include <boost/filesystem.hpp>
#include <boost/optional.hpp>
#include "collection.h"
#include "encoder.h"
#include "LRU.h"
#include "entry.h"
#include "raft.h"
#include "bigram_anomaly.h"
#include <boost/serialization/serialization.hpp>
#include "tagger.h"


using namespace std;
namespace fs = boost::filesystem;

class DB
{
private:
    /* CONSTANTS 
       memory_limit
            max memory usage of TextDB in bytes
            default 2GB
       memory_epsilon 
            memory buffer before caching
            default 10MB
    */
    size_t memory_limit = 2000000000;
    size_t memory_epsilon = 10000;
        
    /* OBJECTS */
    std::unordered_map<std::string, std::function<void(shared_ptr<DB> db, ostream& htmlout, const std::vector<std::string>& args)>> queryFunctions;
    std::unordered_map<std::string, std::string> descriptions;
    std::unordered_map<std::string, std::function<void(shared_ptr<DB> db, ostream& htmlout, const std::vector<std::string>& args)>> metaFunctions;
    SentimentAnalysis sentimentAnalysis;
    LRU lru;


    // index of word
    // max value is ~250,000 since there are only that many english words
    using widx = boost::dynamic_bitset<>;
    
    struct Comparer {
        bool operator() (const boost::dynamic_bitset<> &b1, const boost::dynamic_bitset<> &b2) const {
            return b1.to_ulong() < b2.to_ulong();
        }
    };
    
    void init_query_operations();
    
    bool make_space(size_t required);
    std::pair<std::string, std::string> parseCollectionsDirName(std::string);
    size_t get_occupied_space();
    std::string reassembleText(const std::vector<std::string>& words);

public:
    BigramAnomalyPerceptron bigramAnomalyPerceptron;
    static void gracefulShutdown(int sig)
    {
        // stop accepting requests and handle last accepted request
        // persist un-persisted data
        //
        cout << "Shutting down now..." << endl;
        sleep(2);
        exit(sig);
    }
    static bool ready;
    const static std::string allowed_puncs;
    
    DB(fs::path data, vector<string> replicas, int port, int candidateId, vector<int> replicaIds, vector<Tagger> taggers);
    fs::path datapath;
    std::unordered_map<std::string, Collection*> collections;

    
    void handleQuery(std::vector<std::string> in, ostream& htmlout);
    void commit(const Entry& op);
    
    // Q
    bool add(std::string collection, std::string name, const std::vector<std::string>& text);
    bool add(std::string collection, std::string name, const string& text);
    bool remove(std::string collection, std::string name);
    bool modify(string collection, string name, const vector<string>& text);
    std::string get(std::string collection, std::string name);
    void drop(std::string collection);
    widx uint2widx(unsigned long i);
    std::string getSentence(std::string collection, std::string name, size_t start);

    //std::unordered_map<std::string, const std::vector<std::string>& > search(std::string queryString);

    // Text Mining
    double getSentimentScore(std::string collection, std::string name);
    unordered_map<string, uintmax_t> sentimentDistributionWordList(string collectionName, double granularity);
    void trainNaiveBayes(string collection);
    void markNaiveBayes(string collection, string name, string sentimentClass);
    pair<string, double> testNaiveBayes(string collection, string name);

    
    void printIndex();
    
    void createCollection(string collectionName, string encoding);
    void createCollection(std::string _name, Encoder::CharacterEncoding _encoding);
    void createCollection(string collectionName);
    std::vector<std::string> listCollections();
    
    ofstream log;
    
    Raft raft;
    
    boost::uintmax_t size();
    
    unordered_map<string, uintmax_t> getTermFrequency(string collectionName, string documentName);
    std::unordered_map<std::string, double> getTermFrequencyInverseDocumentFrequency(string collectionName, string documentName);
    
    vector<string> getInterestingDocuments(string collectionName, int n);
    vector<string> getRelatedDocuments(string collectionName, string documentName, int n);
    
    
    
    // These two are here for serialization purposes
    DB()
    :sentimentAnalysis(), raft(), bigramAnomalyPerceptron()
    {};

    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
    }
    
    bool exists(string collectionName, string documentName);
    bool exists(string collectionName);

    unordered_map<string, double> getAllDuplicates(string collectionName, string documentName);
    
    unordered_map<string, Tagger> taggers;

};


#endif /* defined(__TextDB__db__) */
