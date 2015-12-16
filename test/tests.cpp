//
//  tests.cpp
//  TextDB
//
//  Created by Anubhav on 11/3/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#include "tests.h"
#include "db.h"
#include "rand_doc.h"
#include "tagger.h"
#include <boost/filesystem.hpp>

namespace fs = boost::filesystem;
fs::path datapath = "/Users/anubhav/TextDB/test/data";
vector<string> replicas;
int port = 8001;
int candidateId = 0;
vector<int> replicaIds;
vector<Tagger> taggers

DB tdb(datapath, replicas, port, candidateId, replicaIds, taggers);

// This is here due to the extern requirement
// TODO: Remove extern requirement
// TR: Remove extern requirement
shared_ptr<DB> db(&tdb);

int main(int argc, char** argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}


TEST(CRUD, CREATE_COLLECTION)
{
    string collectionName = "sample";
    tdb.createCollection(collectionName);
    vector<string> collectionNames = tdb.listCollections();
    set<string> testSet(collectionNames.begin(), collectionNames.end());
    ASSERT_GE(collectionNames.size(), 1);
    ASSERT_TRUE(testSet.count(collectionName) > 0);
}

TEST(CRUD, CREATE_AND_READ_DOCUMENT)
{
    string collectionName = "sample";
    string documentName = "sampleDocument";
    string text = "this is a sample text document.";
    ASSERT_TRUE(tdb.add(collectionName, documentName, text));
    string exp = tdb.get(collectionName, documentName);
    ASSERT_EQ(text, exp);
}

TEST(CRUD, DOCUMENT_EXISTS)
{
    string collectionName = "sample";
    string documentName = "sampleDocument";
    ASSERT_TRUE(tdb.exists(collectionName, documentName));
}

TEST(CRUD, REMOVE_DOCUMENT)
{
    string collectionName = "sample";
    string documentName = "sampleDocument";
    tdb.remove(collectionName, documentName);
    ASSERT_FALSE(tdb.exists(collectionName, documentName));
}

TEST(CRUD, COLLECTION_EXISTS)
{
    string collectionName = "sample";
    ASSERT_TRUE(tdb.exists(collectionName));
}

TEST(CRUD, DROP_COLLECTION)
{
    string collectionName = "sample";
    tdb.drop(collectionName);
    ASSERT_FALSE(tdb.exists(collectionName));
}

TEST(CRUD, REMOVE_LSH)
{
    // Might be a long test
    // Generate Text docs
    string doc = gen_doc(15 + rand() % 35);
    // Calculate accuracy
    tdb.add("ddr", "ddrtest1", doc);
    tdb.remove("ddr", "ddrtest1");
    auto m = tdb.collections["ddr"]->localitySensitiveHashing.test(doc, "");
    ASSERT_TRUE(m.empty());
}

TEST(SAMPLE, DEDUPLICATION)
{
    string doc = gen_doc(15 + rand() % 35);
    string unique = gen_doc(15 + rand() % 35);
    // Calculate accuracy
    tdb.add("ddr", "ddrtest1", doc);
    tdb.add("ddr", "ddrtest2", doc);
    tdb.add("ddr", "ddrtest3", unique);
    auto m = tdb.collections["ddr"]->localitySensitiveHashing.test(doc, "");
    ASSERT_TRUE(m.count("ddrtest2"));
    ASSERT_TRUE(!m.count("ddrtest3"));
}

// Sentiment analysis sanity chekcs
TEST(SAMPLE, SENTIMENT_ANALYSIS)
{
    string collectionName = "sentiment";
    string positiveText = "This is a good piece of text";
    string negativeText = "This is a bad piece of text";
    tdb.createCollection(collectionName);
    tdb.add(collectionName, "positive", positiveText);
    tdb.add(collectionName, "negative", negativeText);
    cout << "Positive: " << tdb.getSentimentScore(collectionName, "positive") << endl;
    cout << "Negative: " << tdb.getSentimentScore(collectionName, "negative") << endl;
    ASSERT_GE(tdb.getSentimentScore(collectionName, "positive"), 0);
    ASSERT_LE(tdb.getSentimentScore(collectionName, "negative"), 0);
}

TEST(SAMPLE, TERM_FREQUENCY)
{
    string collectionName = "sentiment";
    unordered_map<string, uintmax_t> tf1 = tdb.getTermFrequency(collectionName, "positive");
    unordered_map<string, uintmax_t> tf2 = tdb.getTermFrequency(collectionName, "negative");
    unordered_map<string, uintmax_t> exp{
        {"this", 1},
        {"is", 1},
        {"a", 1},
        {"good", 1},
        {"piece", 1},
        {"of", 1},
        {"text", 1}
    };
    
    ASSERT_EQ(tf1, exp);
    ASSERT_NE(tf2, exp);
}

TEST(SAMPLE, TERM_FREQUENCY_INVERSE_DOCUMENT_FREQUENCY)
{
    string collectionName = "sentiment";
    unordered_map<string, double> tf1 = tdb.getTermFrequencyInverseDocumentFrequency(collectionName, "positive");
    unordered_map<string, double> exp{
        {"text", 0.693147},
        {"of", 0.693147},
        {"a", 0.693147},
        {"piece", 0.693147},
        {"is", 0.693147},
        {"good", 1.38629},
        {"this", 0.693147}
    };
    for (auto p: exp) {
        string k = p.first;
        ASSERT_NEAR(tf1[k], exp[k], 0.1);
    }
}

TEST(BENCHMARK, ANOMALY)
{
    // Might be a long test
    // Generate Text docs
    string doc = gen_doc(15 + rand() % 35);
    // TODO: Perturb text docs
    cout << "is_anomaly, random" << tdb.bigramAnomalyPerceptron.is_anomaly(doc) << endl;
    ASSERT_TRUE(tdb.bigramAnomalyPerceptron.is_anomaly(doc));
    cout << "is_anomaly, real" << tdb.bigramAnomalyPerceptron.is_anomaly(doc) << endl;
    ASSERT_FALSE(tdb.bigramAnomalyPerceptron.is_anomaly("Hi my name is alice"));
}