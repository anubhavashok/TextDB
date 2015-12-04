//
//  queries.h
//  TextDB
//
//  Created by Anubhav on 7/31/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef TextDB_queries_h
#define TextDB_queries_h
#include "query.h"
#include "db.h"
#include "error.h"

#include <curlpp/cURLpp.hpp>
#include <boost/tokenizer.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <vector>
#include <sstream>
//#include "custom_lambda.h"

using namespace std;

// Validation helpers, move to utils

static bool collectionExists(shared_ptr<DB> db, string collectionName)
{
    return db->collections.count(collectionName) > 0;
}

static bool documentExists(shared_ptr<DB> db, string collectionName, string documentName)
{
    return collectionExists(db, collectionName) && db->collections[collectionName]->exists(documentName);
}

class PreexistingCollection : public error
{
public:
    PreexistingCollection(string collectionName)
    : error("Collection: " + collectionName + " already exists", 607736)
    {
    }
};

class NonexistingCollection : public error
{
public:
    NonexistingCollection(string collectionName)
    : error("Collection: " + collectionName + " does not exist", 607737)
    {
    }
};

class PreexistingDocument : public error
{
public:
    PreexistingDocument(string collectionName, string documentName)
    : error("Document: " + documentName + " from collection: " + collectionName + " already exists", 607738)
    {
    }
};

class NonexistingDocument : public error
{
public:
    NonexistingDocument(string collectionName, string documentName)
    : error("Document: " + documentName + " from collection: " + collectionName + " does not exists", 607739)
    {
    }
};

class BadAuthorization : public error
{
public:
    BadAuthorization(string authorization)
    : error("Authorization: " + authorization + " is invalid", 467705)
    {
    }
};

static void ensureCollectionExists(shared_ptr<DB> db, string collectionName)
{
    if (!collectionExists(db, collectionName)) {
        throw NonexistingCollection(collectionName);
    }
}

static void ensureCollectionDoesntExist(shared_ptr<DB> db, string collectionName)
{
    if (collectionExists(db, collectionName)) {
        throw PreexistingCollection(collectionName);
    }
}

static void ensureDocumentExists(shared_ptr<DB> db, string collectionName, string documentName)
{
    ensureCollectionExists(db, collectionName);
    if (!documentExists(db, collectionName, documentName)) {
        throw NonexistingDocument(collectionName, documentName);
    }
}

static void ensureDocumentDoesntExist(shared_ptr<DB> db, string collectionName, string documentName)
{
    ensureCollectionExists(db, collectionName);
    if (documentExists(db, collectionName, documentName)) {
        throw PreexistingDocument(collectionName, documentName);
    }
}

static void validateAuthorization(string auth, string truth)
{
    if (auth != truth) {
        throw BadAuthorization(auth);
    }
}

static void successfulReply(ostream& out, std::initializer_list<pair<string, string>> args)
{
    boost::property_tree::ptree json;
    json.put("code", 200);
    json.put("msg", "ok");
    for (auto arg: args) {
        json.put(arg.first, arg.second);
    }
    
    stringstream ss;
    boost::property_tree::write_json(ss, json);
    out << ss.str();
}

static void successfulReply(ostream& out, std::initializer_list<pair<string, string>> args, std::initializer_list<pair<string, vector<string>>> vargs)
{
    boost::property_tree::ptree json;
    json.put("code", 200);
    json.put("msg", "ok");
    for (auto arg: args) {
        json.put(arg.first, arg.second);
    }
    
    for (auto varg: vargs) {
        boost::property_tree::ptree arr;
        for (string arg: varg.second) {
            boost::property_tree::ptree elem;
            elem.put("", arg);
            arr.push_back(make_pair("", elem));
        }
        json.add_child(varg.first, arr);
    }
    
    stringstream ss;
    boost::property_tree::write_json(ss, json);
    out << ss.str();
}


static void successfulReply(ostream& out, std::initializer_list<pair<string, string>> args, std::initializer_list<pair<string, unordered_map<string, boost::uintmax_t>>> vargs)
{
    boost::property_tree::ptree json;
    json.put("code", 200);
    json.put("msg", "ok");
    for (auto arg: args) {
        json.put(arg.first, arg.second);
    }
    
    for (auto varg: vargs) {
        boost::property_tree::ptree arr;
        unordered_map<string, boost::uintmax_t> arg = varg.second;
        boost::property_tree::ptree elem;
        for (auto p: arg) {
            if (!p.first.empty()) {
                elem.put(p.first, to_string(p.second));
            }
        }
        json.add_child(varg.first, elem);
    }
    
    stringstream ss;
    boost::property_tree::write_json(ss, json);
    out << ss.str();
}

static void successfulReply(ostream& out, std::initializer_list<pair<string, string>> args, std::initializer_list<pair<string, unordered_map<string, double>>> vargs)
{
    boost::property_tree::ptree json;
    json.put("code", 200);
    json.put("msg", "ok");
    for (auto arg: args) {
        json.put(arg.first, arg.second);
    }
    
    for (auto varg: vargs) {
        boost::property_tree::ptree arr;
        unordered_map<string, double> arg = varg.second;
        boost::property_tree::ptree elem;
        for (auto p: arg) {
            if (!p.first.empty()) {
                elem.put(p.first, to_string(p.second));
            }
        }
        json.add_child(varg.first, elem);
    }
    
    stringstream ss;
    boost::property_tree::write_json(ss, json);
    out << ss.str();
}

vector<query> queries {
    
    // Collection
    query("addCollection", "Creates a new empty collection with unicode encoding", "v1/add/{collectionName}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              ensureCollectionDoesntExist(db, collectionName);
             
              db->createCollection(collectionName);
              successfulReply(out, {{"op", "addCollection"}, {"collectionName", collectionName}});
          }),
    
    query("addCollectionWithEncoding", "Creates a new empty collection with a specified encoding", "v1/add/{collectionName}/{encoding}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string encoding = args["encoding"];
              ensureCollectionDoesntExist(db, collectionName);
              
              db->createCollection(collectionName, encoding);
              
              successfulReply(out, {{"op", "addCollectionWithEncoding"}, {"collectionName", collectionName}, {"encoding", encoding}});
          }),
    
    query("dropCollection", "Removes an existing collection", "v1/drop/{collectionName}/{auth}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string auth = args["auth"];
              ensureCollectionExists(db, collectionName);
              validateAuthorization(auth, "scu2447k");
              
              db->drop(collectionName);
              
              successfulReply(out, {{"op", "dropCollection"}, {"collectionName", collectionName}});
          }),

    query("collectionSize", "Returns how many bytes a specified collection occupies on disk", "v1/size/{collectionName}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              ensureCollectionExists(db, collectionName);
              
              boost::uintmax_t size = db->collections[collectionName]->size();
              
              successfulReply(out, {{"op", "collectionSize"}, {"collectionName", collectionName}, {"size", to_string(size)}});
          }),
    
    query("listDocs", "Lists all documents in a particular collection", "v1/list/{collectionName}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              ensureCollectionExists(db, collectionName);
              
              vector<string> documentNames = db->collections[collectionName]->getDocumentNames();
              
              successfulReply(out, {{"op", "listDocs"}, {"collectionName", collectionName}}, {{"documentNames", documentNames}});
          }),
    
    query("getInterestingDocuments", "Gets top n interesting documents", "v1/interesting/{collectionName}/{n}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              int n = stoi(args["n"]);
              ensureCollectionExists(db, collectionName);
              
              vector<string> documentNames = db->getInterestingDocuments(collectionName, n);
              successfulReply(out, {{"op", "addCollection"}, {"collectionName", collectionName}}, {{"documentNames", documentNames}});
          }),
    
    query("getRelatedDocuments", "Gets top n related documents", "v1/related/{collectionName}/{documentName}/{n}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              int n = stoi(args["n"]);
              ensureCollectionExists(db, collectionName);
              ensureDocumentExists(db, collectionName, documentName);

              vector<string> documentNames = db->getRelatedDocuments(collectionName, documentName, n);
              successfulReply(out, {{"op", "addCollection"}, {"collectionName", collectionName}}, {{"documentNames", documentNames}});
          }),


    // Document
    query("addDocument", "Adds a document to a specified collection", "v1/add/{collectionName}/{documentName}/{text}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              string text = args["text"];
              ensureDocumentDoesntExist(db, collectionName, documentName);
              
              std::string rawtext = curlpp::unescape(text);
              cout << "Text: " << rawtext << endl;
              
              db->add(collectionName, documentName, rawtext);
              
              successfulReply(out, {{"op", "addDocument"}, {"collectionName", collectionName}, {"documentName", documentName}});
          }),
    
    query("removeDocument", "Removes a specified document", "v1/remove/{collectionName}/{documentName}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);
              
              db->remove(collectionName, documentName);
              
              successfulReply(out, {{"op", "removeDocument"}, {"collectionName", collectionName}, {"documentName", documentName}});
          }),
    
    query("getDocument", "Gets text of a specified document", "v1/get/{collectionName}/{documentName}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);

              string text = db->get(collectionName, documentName);
              successfulReply(out, {{"op", "getDocument"}, {"collectionName", collectionName}, {"documentName", documentName}, {"text", text}});
          }),
    
    query("getSentence", "Returns nth sentence in a specified document", "v1/get/{collectionName}/{documentName}/sentence/{n}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              string n_str = args["n"];
              ensureDocumentExists(db, collectionName, documentName);
              try {
                  boost::uintmax_t n = boost::lexical_cast<boost::uintmax_t>(n_str);
                  string sentence = db->getSentence(collectionName, documentName, n);
                  
                  successfulReply(out, {{"op", "getSentence"}, {"collectionName", collectionName}, {"documentName", documentName}, {"n", to_string(n)}, {"sentence", sentence}});
              } catch (boost::bad_lexical_cast const& ) {
                  throw ArgumentValueError("n", "integer", "string");
              }

          }),
    
    query("documentSize", "Returns how many bytes a specified document occupies on disk", "v1/size/{collectionName}/{documentName}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);

              boost::uintmax_t size = db->collections[collectionName]->size(documentName);

              successfulReply(out, {{"op", "documentSize"}, {"collectionName", collectionName}, {"documentName", documentName}, {"documentSize", to_string(size)}});
          }),

    // Science
    query("wordListSentiment", "Calculates sentiment score of a document using a word list", "v1/get/{collectionName}/{documentName}/sentiment",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);

              double sentiment = db->getSentimentScore(collectionName, documentName);
              
              successfulReply(out, {{"op", "getSentence"}, {"collectionName", collectionName}, {"documentName", documentName}, {"sentiment", to_string(sentiment)}});
          }),
    
    query("markSentiment", "Mark a document's sentiment for training", "v1/add/{collectionName}/{documentName}/naive-bayes-sentiment/{sentiment}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              string sentiment = args["sentiment"];
              ensureDocumentExists(db, collectionName, documentName);
              
              db->markNaiveBayes(collectionName, documentName, sentiment);

              successfulReply(out, {{"op", "markSentiment"}, {"collectionName", collectionName}, {"documentName", documentName}, {"sentiment", sentiment}});
          }),

    query("getNaiveBayesSentiment", "Get sentiment of a document based on a trained naive bayes model", "v1/get/{collectionName}/{documentName}/naive-bayes-sentiment",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);

          }),
    
    query("getTermFrequency", "Get term frequency table of a document", "v1/get/{collectionName}/{documentName}/tf",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);
              
              unordered_map<string, boost::uintmax_t> tf = db->getTermFrequency(collectionName, documentName);
              
              successfulReply(out, {{"op", "getTermFrequency"}, {"collectionName", collectionName}, {"documentName", documentName}}, {{"tf", tf}});

          }),
    
    query("getTermFrequency-InverseDocumentFrequency", "Get tfidf score of document with regards to other documents in the same colletion", "v1/get/{collectionName}/{documentName}/tfidf",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);

              unordered_map<string, double> tfidf = db->getTermFrequencyInverseDocumentFrequency(collectionName, documentName);
              
              successfulReply(out, {{"op", "getTermFrequency-InverseDocumentFrequency"}, {"collectionName", collectionName}, {"documentName", documentName}}, {{"tfidf", tfidf}});
          }),

    query("getSimilarity", "Get cosine similarity of 2 specified documents in a collection", "v1/compare/{collectionName}/{documentName1}/{documentName2}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName1 = args["documentName1"];
              string documentName2 = args["documentName2"];
              ensureDocumentExists(db, collectionName, documentName1);
              ensureDocumentExists(db, collectionName, documentName2);
          }),

    // DB
    query("dbSize", "Returns how many bytes the database occupies on disk", "v1/size",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              // TODO: add db name
              boost::uintmax_t size = db->size();
              
              successfulReply(out, {{"op", "dbSize"}, {"dbSize", to_string(size)}});
          }),
    
    query("listCollections", "Lists all collections in the database", "v1/list",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              vector<string> collectionNames = db->listCollections();
              
              successfulReply(out, {{"op", "listCollections"}}, {{"collectionNames", collectionNames}});
          }),
    /*
    query("lambda", "Allows input of serialized lambda", "v1/lambda/{lambda}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string lambda = curlpp::unescape(args["lambda"]);
              cout << lambda << endl;
              auto f = Extractor::extractLambda(lambda);
              cout << "Lambda has been extracted" << endl;
              f(db, out, args);
              successfulReply(out, {{"op", "lambda"}});
          }),
    */
    query("sentimentDistribution", "Returns distribution of sentiment", "v1/sentimentDistribution/{collectionName}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              double granularity = stod(args["granularity"]);
              
              unordered_map<string, uintmax_t> distribution = db->sentimentDistributionWordList(collectionName, granularity);
              
              successfulReply(out, {{"op", "sentimentDistribution"}}, {{"distribution", distribution}});
          }),
    query("taggers", "Returns details of custom tagger", "v1/taggers/{taggerName}/{text}",
          [](shared_ptr<DB> db, ostream& out, map<string, string>& args) {
              string taggerName = args["taggerName"];
              string text = args["text"];
              // tagger is spawned on this thread
              db->taggers[taggerName].spawn();
              db->taggers[taggerName].activate();
              string result = "";
              successfulReply(out, {{"op", "taggers"}, {"res", result}});
          })
};

#endif
