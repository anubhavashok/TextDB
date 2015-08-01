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

#include <vector>
using namespace std;

vector<query> queries {
    
    // Collection
    query("addCollection", "Creates a new empty collection with unicode encoding", "v1/add/{collectionName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              
          }),
    
    query("addCollectionWithEncoding", "Creates a new empty collection with a specified encoding", "v1/add/{collectionName}/{encoding}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string encoding = args["encoding"];

          }),
    
    query("dropCollection", "Removes an existing collection", "v1/drop/{collectionName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              
          }),

    query("collectionSize", "Returns how many bytes a specified collection occupies on disk", "v1/size/{collectionName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              
          }),
    
    query("listDocs", "Lists all documents in a particular collection", "v1/list/{collectionName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              
          }),


    // Document
    query("addDocument", "Adds a document to a specified collection", "v1/add/{collectionName}/{documentName}/{text}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              string text = args["text"];
              
          }),
    
    query("removeDocument", "Removes a specified document", "v1/remove/{collectionName}/{documentName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              
          }),
    
    query("getDocument", "Gets text of a specified document", "v1/get/{collectionName}/{documentName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              
          }),
    
    query("getSentence", "Returns nth sentence in a specified document", "v1/get/{collectionName}/{documentName}/{n}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              string n = args["n"];
          }),
    
    query("documentSize", "Returns how many bytes a specified document occupies on disk", "v1/size/{collectionName}/{documentName}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              
          }),

    // Science
    query("wordListSentiment", "Calculates sentiment score of a document using a word list", "v1/get/{collectionName}/{documentName}/sentiment",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              
          }),
    
    query("markSentiment", "Mark a document's sentiment for training", "v1/add/{collectionName}/{documentName}/naive-bayes-sentiment/{sentiment}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              string sentiment = args["sentiment"];

          }),

    query("getNaiveBayesSentiment", "Get sentiment of a document based on a trained naive bayes model", "v1/get/{collectionName}/{documentName}/naive-bayes-sentiment",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              
          }),
    
    query("getTermFrequency", "Get term frequency table of a document", "v1/get/{collectionName}/{documentName}/tf",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              
          }),
    
    query("getTermFrequency-InverseDocumentFrequency", "Get tfidf score of document with regards to other documents in the same colletion", "v1/get/{collectionName}/{documentName}/tfidf",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName = args["documentName"];
              
          }),

    query("getSimilarity", "Get cosine similarity of 2 specified documents in a collection", "v1/compare/{collectionName}/{documentName1}/{documentName2}",
          [](DB* db, ostream& out, map<string, string>& args) {
              string collectionName = args["collectionName"];
              string documentName1 = args["documentName1"];
              string documentName2 = args["documentName2"];

              
          }),

    // DB
    query("dbSize", "Returns how many bytes the database occupies on disk", "v1/size",
          [](DB* db, ostream& out, map<string, string>& args) {
              
          }),
    
    query("listCollections", "Lists all collections in the database", "v1/list",
          [](DB* db, ostream& out, map<string, string>& args) {
              
          }),
    
    query("endpoints", "Lists all user facing endpoints", "v1/",
          [](DB* db, ostream& out, map<string, string>& args) {
              
          }),

};

#endif
