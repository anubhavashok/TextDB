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




vector<query> queries {
    
    // Collection
    query("addCollection", "Creates a new empty collection with unicode encoding", "v1/add/{collectionName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              ensureCollectionDoesntExist(db, collectionName);
              
          }),
    
    query("addCollectionWithEncoding", "Creates a new empty collection with a specified encoding", "v1/add/{collectionName}/{encoding}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string encoding = args["encoding"];
              ensureCollectionDoesntExist(db, collectionName);
              
          }),
    
    query("dropCollection", "Removes an existing collection", "v1/drop/{collectionName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              ensureCollectionExists(db, collectionName);
              
          }),

    query("collectionSize", "Returns how many bytes a specified collection occupies on disk", "v1/size/{collectionName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              ensureCollectionExists(db, collectionName);

              
          }),
    
    query("listDocs", "Lists all documents in a particular collection", "v1/list/{collectionName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              ensureCollectionExists(db, collectionName);
              
          }),


    // Document
    query("addDocument", "Adds a document to a specified collection", "v1/add/{collectionName}/{documentName}/{text}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              string text = args["text"];
              ensureDocumentDoesntExist(db, collectionName, documentName);
          }),
    
    query("removeDocument", "Removes a specified document", "v1/remove/{collectionName}/{documentName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);

          }),
    
    query("getDocument", "Gets text of a specified document", "v1/get/{collectionName}/{documentName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);

          }),
    
    query("getSentence", "Returns nth sentence in a specified document", "v1/get/{collectionName}/{documentName}/sentence/{n}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              string n = args["n"];
              ensureDocumentExists(db, collectionName, documentName);

          }),
    
    query("documentSize", "Returns how many bytes a specified document occupies on disk", "v1/size/{collectionName}/{documentName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);

          }),

    // Science
    query("wordListSentiment", "Calculates sentiment score of a document using a word list", "v1/get/{collectionName}/{documentName}/sentiment",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);

          }),
    
    query("markSentiment", "Mark a document's sentiment for training", "v1/add/{collectionName}/{documentName}/naive-bayes-sentiment/{sentiment}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              string sentiment = args["sentiment"];
              ensureDocumentExists(db, collectionName, documentName);

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

          }),
    
    query("getTermFrequency-InverseDocumentFrequency", "Get tfidf score of document with regards to other documents in the same colletion", "v1/get/{collectionName}/{documentName}/tfidf",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              ensureDocumentExists(db, collectionName, documentName);

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
              
          }),
    
    query("listCollections", "Lists all collections in the database", "v1/list",
          [](DB* db, ostream& out, map<string, string>& args) {
              vector<string> collectionNames = db->listCollections();
              boost::property_tree::ptree arr;
              for (string collectionName: collectionNames) {
                  boost::property_tree::ptree val;
                  val.put("", collectionName);
                  arr.push_back(make_pair("", val));
              }
              boost::property_tree::ptree json;
              json.add_child("collections", arr);
              stringstream ss;
              boost::property_tree::write_json(ss, json);
              out << ss.str();
          }),
    

};

#endif
