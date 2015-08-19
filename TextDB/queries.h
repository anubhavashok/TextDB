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

using namespace std;

// Validation helpers, move to utils

static bool collectionExists(DB* db, string collectionName)
{
    return db->collections.count(collectionName) > 0;
}

static bool documentExists(DB* db, string collectionName, string documentName)
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

static void ensureCollectionExists(DB* db, string collectionName)
{
    if (!collectionExists(db, collectionName)) {
        throw NonexistingCollection(collectionName);
    }
}

static void ensureCollectionDoesntExist(DB* db, string collectionName)
{
    if (collectionExists(db, collectionName)) {
        throw PreexistingCollection(collectionName);
    }
}

static void ensureDocumentExists(DB* db, string collectionName, string documentName)
{
    ensureCollectionExists(db, collectionName);
    if (!documentExists(db, collectionName, documentName)) {
        throw NonexistingDocument(collectionName, documentName);
    }
}

static void ensureDocumentDoesntExist(DB* db, string collectionName, string documentName)
{
    ensureCollectionExists(db, collectionName);
    if (documentExists(db, collectionName, documentName)) {
        throw PreexistingDocument(collectionName, documentName);
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
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              ensureCollectionDoesntExist(db, collectionName);
             
              db->createCollection(collectionName);
              successfulReply(out, {{"op", "addCollection"}, {"collectionName", collectionName}});
          }),
    
    query("addCollectionWithEncoding", "Creates a new empty collection with a specified encoding", "v1/add/{collectionName}/{encoding}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string encoding = args["encoding"];
              ensureCollectionDoesntExist(db, collectionName);
              
              db->createCollection(collectionName, encoding);
              
              successfulReply(out, {{"op", "addCollectionWithEncoding"}, {"collectionName", collectionName}, {"encoding", encoding}});
          }),
    
    query("dropCollection", "Removes an existing collection", "v1/drop/{collectionName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              ensureCollectionExists(db, collectionName);
              
              // db->drop(collectionName);
              
              successfulReply(out, {{"op", "dropCollection"}, {"collectionName", collectionName}});
          }),

    query("collectionSize", "Returns how many bytes a specified collection occupies on disk", "v1/size/{collectionName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              ensureCollectionExists(db, collectionName);
              
              boost::uintmax_t size = db->collections[collectionName]->size();
              
              successfulReply(out, {{"op", "collectionSize"}, {"collectionName", collectionName}, {"size", to_string(size)}});
          }),
    
    query("listDocs", "Lists all documents in a particular collection", "v1/list/{collectionName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              ensureCollectionExists(db, collectionName);
              
              vector<string> documentNames = db->collections[collectionName]->listFiles();
              
              successfulReply(out, {{"op", "listDocs"}, {"collectionName", collectionName}}, {{"documentNames", documentNames}});
          }),


    // Document
    query("addDocument", "Adds a document to a specified collection", "v1/add/{collectionName}/{documentName}/{text}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              string text = args["text"];
              ensureDocumentDoesntExist(db, collectionName, documentName);
              
              std::string rawtext = curlpp::unescape(text);
              cout << "Text: " << rawtext << endl;
              
              /* Tokenize words */
              boost::char_separator<char> sep("", DB::allowed_puncs.c_str()); // specify only the kept separators
              boost::tokenizer<boost::char_separator<char>> tokens(rawtext, sep);
              
              std::vector<std::string> t;
              
              for (std::string _t : tokens) {
                  // boost::trim(t);
                  if (_t != "") {
                      t.push_back(_t);
                  }
              }
              
              db->add(collectionName, documentName, t);
              
              successfulReply(out, {{"op", "addDocument"}, {"collectionName", collectionName}, {"documentName", documentName}});
          }),
    
    query("removeDocument", "Removes a specified document", "v1/remove/{collectionName}/{documentName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);
              
              db->remove(collectionName, documentName);
              
              successfulReply(out, {{"op", "removeDocument"}, {"collectionName", collectionName}, {"documentName", documentName}});
          }),
    
    query("getDocument", "Gets text of a specified document", "v1/get/{collectionName}/{documentName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);

              string text = db->get(collectionName, documentName);
              successfulReply(out, {{"op", "getDocument"}, {"collectionName", collectionName}, {"documentName", documentName}, {"text", text}});
          }),
    
    query("getSentence", "Returns nth sentence in a specified document", "v1/get/{collectionName}/{documentName}/sentence/{n}",
          [](DB* db, ostream& out, map<string, string>& args) {
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
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);

              boost::uintmax_t size = db->collections[collectionName]->size(documentName);

              successfulReply(out, {{"op", "documentSize"}, {"collectionName", collectionName}, {"documentName", documentName}, {"documentSize", to_string(size)}});
          }),

    // Science
    query("wordListSentiment", "Calculates sentiment score of a document using a word list", "v1/get/{collectionName}/{documentName}/sentiment",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);

              double sentiment = db->getSentimentScore(collectionName, documentName);
              
              successfulReply(out, {{"op", "getSentence"}, {"collectionName", collectionName}, {"documentName", documentName}, {"sentiment", to_string(sentiment)}});
          }),
    
    query("markSentiment", "Mark a document's sentiment for training", "v1/add/{collectionName}/{documentName}/naive-bayes-sentiment/{sentiment}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              string sentiment = args["sentiment"];
              ensureDocumentExists(db, collectionName, documentName);
              
              db->markNaiveBayes(collectionName, documentName, sentiment);

              successfulReply(out, {{"op", "markSentiment"}, {"collectionName", collectionName}, {"documentName", documentName}, {"sentiment", sentiment}});
          }),

    query("getNaiveBayesSentiment", "Get sentiment of a document based on a trained naive bayes model", "v1/get/{collectionName}/{documentName}/naive-bayes-sentiment",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);

          }),
    
    query("getTermFrequency", "Get term frequency table of a document", "v1/get/{collectionName}/{documentName}/tf",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);
              
              unordered_map<string, boost::uintmax_t> tf = db->getTermFrequency(collectionName, documentName);
              
              successfulReply(out, {{"op", "getTermFrequency"}, {"collectionName", collectionName}, {"documentName", documentName}}, {{"tf", tf}});

          }),
    
    query("getTermFrequency-InverseDocumentFrequency", "Get tfidf score of document with regards to other documents in the same colletion", "v1/get/{collectionName}/{documentName}/tfidf",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);

              unordered_map<string, double> tfidf = db->getTermFrequencyInverseDocumentFrequency(collectionName, documentName);
              
              successfulReply(out, {{"op", "getTermFrequency-InverseDocumentFrequency"}, {"collectionName", collectionName}, {"documentName", documentName}}, {{"tfidf", tfidf}});
          }),

    query("getSimilarity", "Get cosine similarity of 2 specified documents in a collection", "v1/compare/{collectionName}/{documentName1}/{documentName2}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName1 = args["documentName1"];
              string documentName2 = args["documentName2"];
              ensureDocumentExists(db, collectionName, documentName1);
              ensureDocumentExists(db, collectionName, documentName2);
          }),

    // DB
    query("dbSize", "Returns how many bytes the database occupies on disk", "v1/size",
          [](DB* db, ostream& out, map<string, string>& args) {
              // TODO: add db name
              boost::uintmax_t size = db->size();
              
              successfulReply(out, {{"op", "dbSize"}, {"dbSize", to_string(size)}});
          }),
    
    query("listCollections", "Lists all collections in the database", "v1/list",
          [](DB* db, ostream& out, map<string, string>& args) {
              vector<string> collectionNames = db->listCollections();
              
              successfulReply(out, {{"op", "listCollections"}}, {{"collectionNames", collectionNames}});
          }),
    

};

#endif
