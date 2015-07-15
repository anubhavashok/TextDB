//
//  db.cpp
//  TextDB
//
//  Created by Anubhav on 8/26/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#include "db.h"
#include <algorithm>
#include <cmath>
#include <fstream>
#include <iostream>
#include <cassert>
#include "bitreader.h"
#include <snappy.h>
#include <boost/filesystem/path.hpp>
#include "options.h"
#include <boost/algorithm/string.hpp>
#include <string.h>
#include <boost/foreach.hpp>
#include "collection.h"
#include <boost/tokenizer.hpp>
#include <boost/range/algorithm/count.hpp>
#include <boost/regex.hpp>
#include <unordered_map>
#include "similarity.h"
#include "operation.h"
#include <boost/serialization/vector.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <curlpp/cURLpp.hpp>
#include "entry.h"
#include "raft.h"
#include "Doc.h"


/*
 * class DB
 * Main database controller
 * Ideally should not contain any functionality
 */

namespace fs = boost::filesystem;

// ready - flag that indicates when request handler is ready
bool DB::ready = false;

// index of word
// max value is ~250,000 since there are only that many english words
using widx = boost::dynamic_bitset<>;
const std::string DB::allowed_puncs = " _.,-+*:;!?@$&[]()/|`\\\"\n\t\r";


DB::DB(fs::path data, vector<string> replicas, int port, int candidateId, vector<int> replicaIds)
: sentimentAnalysis(data), datapath(data), raft(replicas, replicaIds, candidateId, shared_ptr<DB>(this), data / "replication" / to_string(port))
{
    
    // Create {data_path}/collections folder
    fs::path d = data / "collections";
    if (!fs::exists(d)) {
        fs::create_directories(d);
    }
    
    // Initialize all collections
    fs::directory_iterator it(d), eod;
    std::vector<std::string> collectionNames;
    BOOST_FOREACH(fs::path p, std::make_pair(it, eod))
    {
        if(fs::is_directory(p))
        {
            cout << p.stem().string() << endl;
            collectionNames.push_back(p.stem().string());
        }
    }
    
    for (std::string collectionName: collectionNames) {
        std::string name, type;
        std::tie(name, type) = parseCollectionsDirName(collectionName);
        Encoder::CharacterEncoding encoding = Encoder::str2encoding(type);
        Collection* c = new Collection(datapath / "collections" / collectionName, encoding);
        collections[name] = c;
    }
    init_query_operations();
}

std::pair<std::string, std::string> DB::parseCollectionsDirName(std::string directory)
{
    // split at - return <collection, type>
    std::vector<std::string> args;
    boost::split(args, directory, boost::is_any_of("-"));
    return std::make_pair(args[0], args[1]);
}

Doc d1("test_doc", vector<string>{"hi", "my", "name", "is"});

void DB::init_query_operations()
{
    
    
    /* ---------------------------------------------------------------------------------------- */

    
    // Test endpoint
    queryFunctions["doctest"] = [](DB* db, ostream& htmlout, const vector<string>& args){
        std::string t = args[0];
        std::string rawtext = curlpp::unescape(t);
        
        /* Tokenize words */
        boost::char_separator<char> sep("", DB::allowed_puncs.c_str()); // specify only the kept separators
        boost::tokenizer<boost::char_separator<char>> tokens(rawtext, sep);
        
        std::vector<std::string> text;
        
        for (std::string t : tokens) {
            boost::trim(t);
            if (t != "") {
                text.push_back(t);
            }
        }

        for (string s: text){
            cout << s << " ";
            
        }
        cout << endl;
        d1.update(text);
        d1.diffs.top().printSES();
        d1.output(htmlout);
    };
    
    
    /* ---------------------------------------------------------------------------------------- */

    
    descriptions["revert"] = "Reverts a document to the immediately previous version\nUsage:\\revert\\{collectionName}\\{docName}";

    queryFunctions["revert"] = [](DB* db, ostream& htmlout, const vector<string>& args){
        d1.revert(1);
        d1.output(htmlout);
        d1.output(cout);
    };
    
    
    /* ---------------------------------------------------------------------------------------- */
    
    
    descriptions["collectionsize"] = "Returns how many bytes a specified collection occupies on disk\nUsage:\\collectionsize\\{collectionName}";
    
    queryFunctions["collectionsize"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        int argc = 1;
        if (args.size() < argc) {
            htmlout << "Required number of arguments: " << argc;
            return;
        }
        std::string collection = args[0];
        
        htmlout << db->collections[collection]->disk_size();
    };
    
    
    /* ---------------------------------------------------------------------------------------- */
    
    
    descriptions["size"] = "Returns how many bytes a specified document occupies on disk\nUsage:\\size\\{collectionName}\\{docName}";

    queryFunctions["size"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        int argc = 2;
        if (args.size() < argc) {
            htmlout << "Required number of arguments: " << argc;
            return;
        }

        std::string collection = args[0];
        std::string doc = args[1];
        
        htmlout << db->collections[collection]->size(doc);
    };

    
    /* ---------------------------------------------------------------------------------------- */

    
    descriptions["add"] = "Adds a document to a collection\nUsage:\\add\\{collectionName}\\{docName}\\{text}";

    queryFunctions["add"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        int argc = 3;
        if (args.size() < argc) {
            htmlout << "Required number of arguments: " << argc;
            return;
        }
        
        std::string collection = args[0];
        std::string name = args[1];
        std::string t = args[2];
        
        std::string rawtext = curlpp::unescape(t);
        cout << "Text: " << rawtext << endl;
        
        /* Tokenize words */
        boost::char_separator<char> sep("", DB::allowed_puncs.c_str()); // specify only the kept separators
        boost::tokenizer<boost::char_separator<char>> tokens(rawtext, sep);
        
        std::vector<std::string> text;

        for (std::string t : tokens) {
            // boost::trim(t);
            if (t != "") {
                text.push_back(t);
            }
        }
        
        if(!db->add(collection, name, text)) {
            htmlout << "Add unsuccessful" << endl;
            return;
        }
        for (std::string word: text) {
            htmlout << word;
        }
    };
    
    /* ---------------------------------------------------------------------------------------- */
    
    
    descriptions["modify"] = "Modifies a document that already exists, storing history of previous versions. \nUsage:\\modify\\{collectionName}\\{docName}\\{text}";

    queryFunctions["modify"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        int argc = 3;
        if (args.size() < argc) {
            htmlout << "Required number of arguments: " << argc;
            return;
        }

        std::string collection = args[0];
        std::string name = args[1];
        std::string t = args[2];
        
        std::string rawtext = curlpp::unescape(t);
        
        /* Tokenize words */
        boost::char_separator<char> sep("", DB::allowed_puncs.c_str()); // specify only the kept separators
        boost::tokenizer<boost::char_separator<char>> tokens(rawtext, sep);
        
        std::vector<std::string> text;
        
        for (std::string t : tokens) {
            boost::trim(t);
            if (t != "") {
                text.push_back(t);
            }
        }
        db->modify(collection, name, text);
    };
    
    
    /* ---------------------------------------------------------------------------------------- */
    
    
    descriptions["remove"] = "Removes a particular document\nUsage:\\remove\\{collectionName}\\{docName}";

    queryFunctions["remove"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        int argc = 2;
        if (args.size() < argc) {
            htmlout << "Required number of arguments: " << argc;
            return;
        }

        std::string collection = args[0];
        std::string name = args[1];
        
        // TODO: mark words in deprecated text document in word list and lazy remove
        bool success = db->remove(collection, name);
        htmlout << success << endl;
    };
    
    
    /* ---------------------------------------------------------------------------------------- */
    
    
    descriptions["get"] = "Gets text of a particular document\nUsage:\\get\\{collectionName}\\{docName}";

    queryFunctions["get"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        int argc = 2;
        if (args.size() < argc) {
            htmlout << "Required number of arguments: " << argc;
            return;
        }
        std::string collection = args[0];
        std::string name = args[1];
        
        std::string res;
        res = db->get(collection, name);
        htmlout << res;
    };
    
    
    /* ---------------------------------------------------------------------------------------- */
    
    
    descriptions["listdocs"] = "Lists all documents in a particular collection\nUsage:\\listdocs\\{collectionName}";

    queryFunctions["listdocs"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        int argc = 1;
        if (args.size() < argc) {
            htmlout << "Required number of arguments: " << argc;
            return;
        }

        std::string collection = args[0];
        
        // get all doc names
        Collection* c = db->collections[collection];
        std::string json = "[";
        int i = 0;
        for (std::string s: c->listFiles()) {
            json += ((i == 0)? "": ",");
            json += '"' + s + '"';
            i++;
        }
        json += "]";
        htmlout << json;
        
    };
    
    
    /* ---------------------------------------------------------------------------------------- */
    
    
    descriptions["sentiment"] = "Calculates sentiment score of a document\nUsage:\\sentiment\\{collectionName}\\{docName}";

    queryFunctions["sentiment"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        int argc = 2;
        if (args.size() < argc) {
            htmlout << "Required number of arguments: " << argc;
            return;
        }

        std::string collection = args[0];
        std::string name = args[1];
        
        double score = db->getSentimentScore(collection, name);
        //htmlout << "{\"name\":"<<name<<", \"sentimentScore\": "<<score<<"}";
        htmlout << score;
    };
    
    
    
    /* ---------------------------------------------------------------------------------------- */
    
    
    descriptions["sentence"] = "Returns nth sentence in a specified document\nUsage:\\sentence\\{collectionName}\\{docName}\\{n}";

    queryFunctions["sentence"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        int argc = 3;
        if (args.size() < argc) {
            htmlout << "Required number of arguments: " << argc;
            return;
        }

        std::string collection = args[0];
        std::string name = args[1];
        std::string s = args[2];
        
        try {
            size_t start = boost::lexical_cast<int>(s);
            htmlout << db->getSentence(collection, name, start);
        } catch (boost::bad_lexical_cast&) {
        }
        
    };
    
    
    /* ---------------------------------------------------------------------------------------- */

    
    descriptions["drop"] = "Drops a particular collection and all the documents in it\nUsage:\\drop\\{collectionName}";

    queryFunctions["drop"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        int argc = 1;
        if (args.size() < argc) {
            htmlout << "Required number of arguments: " << argc;
            return;
        }

        std::string collection = args[0];
        
        if (db->collections.count(collection)) {
            db->drop(collection);
        }
    };

    
    /* ---------------------------------------------------------------------------------------- */

    
    descriptions["create"] = "Creates a new collection with specified encoding\nUsage:\\create\\{collectionName}\\{encoding}";

    queryFunctions["create"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        int argc = 2;
        if (args.size() < argc) {
            htmlout << "Required number of arguments: " << argc;
            return;
        }

        std::string collection = args[0];
        std::string encoding = args[1];
        
        if (!db->collections.count(collection)) {
            db->createCollection(collection, Encoder::str2encoding(encoding));
        }};

    
    /* ---------------------------------------------------------------------------------------- */

    
    descriptions["listcollections"] = "Lists all collections in the database\nUsage:\\listcollections";

    queryFunctions["listcollections"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        // get all collectionNames
        std::string json = "[";
        int i = 0;
        for (std::string s: db->listCollections()) {
            json += ((i == 0) ? "": ",");
            json += '"' + s + '"';
            i++;
        }
        json += "]";
        htmlout << json;

    };
    
    
    /* ---------------------------------------------------------------------------------------- */

    
    descriptions["termfrequency"] = "Returns term frequency table of a document in json format\nUsage:\\size\\{collectionName}\\{docName}";

    queryFunctions["termfrequency"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        int argc = 2;
        if (args.size() < argc) {
            htmlout << "Required number of arguments: " << argc;
            return;
        }

        std::string collection = args[0];
        std::string name = args[1];
        if (db->collections.count(collection)) {
            std::string frequency_table = db->collections[collection]->get_frequency_table(name);
            htmlout << frequency_table;
        }
    };
    
    
    /* ---------------------------------------------------------------------------------------- */


    descriptions["tfidf"] = "Returns tfidf score of document with regards to other documents in the same colletion\nUsage:\\tfidf\\{collectionName}\\{docName}";

    queryFunctions["tfidf"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        int argc = 2;
        if (args.size() < argc) {
            htmlout << "Required number of arguments: " << argc;
            return;
        }

        using namespace IDF;
        using namespace TF;
        using namespace TFIDF;
        string collection = args[0];
        string name = args[1];
        // make sure document is loaded 
        db->get(collection, name);
        auto doc = db->collections[collection]->get_vector(name);
        vector<vector<string>> docs = db->collections[collection]->get_all();
        unordered_map<string, double> idf = inverse_document_frequency(db->collections[collection]->get_vector(name), docs);
        auto tf = term_frequency(doc);
        auto tfidf_m = tfidf(tf, idf);
        vector<pair<string, int>> tfidf_v(tfidf_m.begin(), tfidf_m.end());
        sort(tfidf_v.begin(), tfidf_v.end(), [](const pair<string, int>& p1, const pair<string, int> p2){
            return (p2.second > p1.second);
        });
        for (auto p: tfidf_m) {
            htmlout << p.first << " " << p.second << endl;
        }
    };
    

    /* ---------------------------------------------------------------------------------------- */

    
    descriptions["similarity"] = "Calculates cosine similarity of 2 specified documents in a collection\nUsage:\\similarity\\{collectionName}\\{doc1}\\{doc2}";

    queryFunctions["similarity"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        int argc = 3;
        if (args.size() < argc) {
            htmlout << "Required number of arguments: " << argc;
            return;
        }

        string collection = args[0];
        string doc1 = args[1];
        string doc2 = args[2];
        
        using namespace Similarity;
        // NOTE: potential memory overflow
        db->get(collection, doc1);
        db->get(collection, doc2);
        auto doc1_v = db->collections[collection]->get_vector(doc1);
        auto doc2_v = db->collections[collection]->get_vector(doc2);

        vector<vector<string>> docs = db->collections[collection]->get_all();

        double similarity = document_similarity(doc1_v, doc2_v, docs);
        htmlout << similarity << endl;
    };
    
    
    /* ---------------------------------------------------------------------------------------- */

    
    metaFunctions["ping"] = [](DB* db, ostream& out, const std::vector<std::string>& args){
        out << "OK" << endl;
    };
    
    
    /* ---------------------------------------------------------------------------------------- */
    
    
    metaFunctions["appendentries"] = [](DB* db, ostream& out, const std::vector<std::string>& args){
         /*
         Arguments:
         term leader’s term
         leaderId so follower can redirect clients
         prevLogIndex index of log entry immediately preceding
         new ones
         prevLogTerm term of prevLogIndex entry
         entries[] log entries to store (empty for heartbeat;
         may send more than one for efficiency)
         leaderCommit leader’s commitIndex
        */
        
        
        db->raft.role = Raft::Follower;
        db->raft.lastHeartbeat = chrono::system_clock::now();
        
        int leaderId = stoi(args[1]);
        if (!db->raft.leaders.count(leaderId)) {
            db->raft.leaders.insert(leaderId);
            if (db->raft.leaders.size() > 1) {
                cout << "More than 1 leader" << endl;
            }
        }
    
        int term = stoi(args[0]);
        int prevLogIndex = stoi(args[2]);
        int prevLogTerm = stoi(args[3]);
        string entriesString = curlpp::unescape(args[4]);
        int leaderCommit = stoi(args[5]);
        
        
        cout << "currentTerm: " << db->raft.currentTerm << endl;
        cout << "leaderTerm: " << term << endl;
        cout << "prevLogIndex: " << prevLogIndex << endl;
        cout << "prevLogTerm: " << prevLogTerm << endl;
        cout << "leaderCommit: " << leaderCommit << endl;
        
        Entries entries;
        stringstream oss(ios_base::in | ios_base::out);
        oss << entriesString;
        boost::archive::text_iarchive ar(oss);
        ar >> entries;
        
        if (term < db->raft.currentTerm) {
            out << -1;
            return;
        }
        
        // output entries to console
        for (Entry& e: entries.entries) {
            cout << "Command received: " << e.op.cmd <<  endl;
        }
        db->raft.currentTerm = max(db->raft.currentTerm, term);

        
        /*
         Receiver implementation:
         1. Reply false if term < currentTerm (§5.1)
         2. Reply false if log doesn’t contain an entry at prevLogIndex
         whose term matches prevLogTerm (§5.3)
         3. If an existing entry conflicts with a new one (same index
         but different terms), delete the existing entry and all that
         follow it (§5.3)
         4. Append any new entries not already in the log
         5. If leaderCommit > commitIndex, set commitIndex =
         min(leaderCommit, index of last new entry)
         */
        if ((db->raft.log.count(prevLogIndex)) && (db->raft.log[prevLogIndex].term != prevLogTerm))
        {
            cout << "Entry at index: " << prevLogIndex << " exists but doesn't match" << endl;
            cout << "Entry at index: " << prevLogIndex << " has command " << db->raft.log[prevLogIndex].op.cmd << endl;
            cout << "Size of log: " << db->raft.log.size() << endl;
            out << -1;
            return;
        }
        // ensure sorted
        sort(entries.entries.begin(), entries.entries.end(), [](const Entry e1, const Entry e2){ return e1.index - e2.index;});
        cout << "entries.entries.size(): " << entries.entries.size() << endl;
        for (Entry& e: entries.entries) {
            if ((db->raft.log.count(e.index)) && (db->raft.log[e.index].term != e.term)) {
                // remove current entry in log and everything else since
                for (int i = e.index; i < db->raft.lastApplied; i++) {
                    db->raft.log.erase(i);
                }
            }
            // append new entries to log
            db->raft.log[e.index] = e;
            db->raft.lastApplied = e.index;
        }
        cout << "Reached commitIndex point, entries size: " << entries.entries.size() << endl;
        for(int i=db->raft.commitIndex; i <= leaderCommit; i++) {
            if (db->raft.log.count(i)) {
                db->commit(db->raft.log[i]);
            }
        }
        db->raft.commitIndex = leaderCommit;
        out << "OK";
        
    };
    
    
    /* ---------------------------------------------------------------------------------------- */

    
    metaFunctions["requestvote"] = [](DB* db, ostream& out, const std::vector<std::string>& args){
        cout << "requestVote" << endl;
        int term = stoi(args[0]);
        int candidateId = stoi(args[1]);
        int lastLogIndex = stoi(args[2]);
        int lastLogTerm = stoi(args[3]);
        
        
        //||((term == db->raft.currentTerm) && (db->raft.lastApplied > lastLogIndex))
        if (
            (term < db->raft.currentTerm)
            )
        {
            out << db->raft.currentTerm << "/" << 0;
            return;
        }
        switch (db->raft.role) {
            case Raft::Candidate:
                out << db->raft.currentTerm << "/" << 0;
                break;
            case Raft::Follower:
                if (db->raft.votedFor == boost::none) {
                    cout << "Authorizing vote for " << candidateId << endl;
                    out << db->raft.currentTerm << "/" << 1;
                    db->raft.currentTerm = term;
                    db->raft.votedFor = candidateId;
                } else {
                    out << db->raft.currentTerm << "/" << 0;
                }
                break;
            default:
                out << db->raft.currentTerm << "/" << 0;
                break;
        }
    };
    
    
    /* ---------------------------------------------------------------------------------------- */

    
    metaFunctions["getid"] = [](DB* db, ostream& out, const std::vector<std::string>& args){
        out << db->raft.candidateId;
    };
    
    
    /* ---------------------------------------------------------------------------------------- */
    
    
    descriptions["endpoints"] = "Lists all user facing endpoints\nUsage:\\endpoints";

    queryFunctions["endpoints"] = [this](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        htmlout << "LIST OF ENDPOINTS: " << endl << endl;
        for (auto p: queryFunctions) {
            htmlout << p.first << endl;
            htmlout << "----------" << endl;
            if (descriptions.count(p.first)) {
                htmlout << descriptions[p.first];
            }
            htmlout << endl;
            htmlout << endl;

        }
    };
    
    
    /* ---------------------------------------------------------------------------------------- */

    
}


void DB::handleQuery(std::vector<std::string> in, ostream& htmlout)
{
    if (!DB::ready) {
        cout << "(DB) Database is not ready to accept requests" << endl;
        return;
    }
    std::string cmd = in[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

    std::vector<std::string> args(in.begin() + 1, in.end());
    
    Operation op;
    op.cmd = cmd;
    op.args = args;
    
    if (queryFunctions.count(cmd)) {
        // call appropriate query
        cout << "(DB) Query: " << cmd << endl;
        this->raft.addEntry(op);
        //bool success = this->oplog.propose(op);
        //if (!success) {
        //  cout << "(DB) Failed to reach majority consensus" << endl;
        //    htmlout << "ERR (M4J0R17Y)" << endl;
        //}
        queryFunctions[cmd](this, htmlout, args);
    } else if(metaFunctions.count(cmd)) {
        cout << "(DB) Query: " << cmd << endl;
        metaFunctions[cmd](this, htmlout, args);
    } else {
        cout << "(DB) Unknown query" << endl;
    }
}

/*
 * add
 * A function that adds a <string, vector<string> > key value pair to the DB
 * @param name a string that represetnts the key
 * @param text a vector of strings representing the value (text document)
 */

bool DB::add(std::string collection, std::string name, const std::vector<std::string>& text)
{
    if (!collections.count(collection)) {
        cout << "Collection: " << collection << " does not exist!" << endl;
        createCollection(collection, Encoder::CharacterEncoding::Unicode);
        return false;
    }
    Collection* c = collections[collection];
    return c->add(name, text);

    //std::vector<widx> serializedDoc = serializeDoc(text);
    //storage[name] = serializedDoc;
}

/*
 * remove
 * A function that removes a <string, vector<string> > key value pair from the DB
 * @param name a string that represetnts the key
 */

bool DB::remove(std::string collection, std::string name)
{
    Collection* c = collections[collection];
    return c->remove(name);
}

/*
 * get
 * A function queries the DB and returns a deserialized text doc corresponding to the key provided
 * @param name a string representing the requested key
 * @return a vector of strings representing a deserialized text doc
 */

std::string DB::get(std::string collection, std::string name)
{
    // Collection does not exist
    if (!collections.count(collection)) {
        cout << "collection not found" << endl;
        return "";
    }
    
    Collection* c = collections[collection];
    
    // Document does not exist
    if (!c->exists(name)) {
        return "";
    }
    
    // Document not in LRU
    if (!lru.cached(collection+":"+name)) {
        size_t required = c->size(name);
        make_space(required);
    }

    lru.access(collection+":"+name);
    return c->get(name);
}

/*
 * make_space
 * A function that makes space for required
 * @param collection a string representing the name of the collection to be deleted
 */

bool DB::make_space(size_t required)
{
    if (required > memory_limit) {
        cout << "Memory limit exceeded" << endl;
        return false;
    }
    size_t current_size = get_occupied_space();

    // kick till enough space left
    while (memory_limit - (current_size + required) < memory_epsilon) {
        std::vector<std::string> last;
        std::string popped = lru.pop_back();
        if (popped == "") {
            // nothing in cache, this probably means that memory_limit is not enough to store the docs
            return false;
        } else {
            boost::split(last, popped, boost::is_any_of(":"));
            if (collections.count(last[0])) {
                Collection* c = collections[last[0]];
                c->kick(last[1]);
            }
            current_size = get_occupied_space();
        }
    }
    return true;
}

/*
 * drop
 * A function that deletes a collection
 * @param collection a string representing the name of the collection to be deleted
 */

void DB::drop(std::string collection)
{
    fs::path collectionsPath = datapath / "collections";
    if (!fs::exists(collectionsPath)) {
        fs::create_directories(collectionsPath);
    }
    fs::directory_iterator it(collectionsPath), eod;
    std::vector<std::string> collectionNames;
    BOOST_FOREACH(fs::path p, std::make_pair(it, eod))
    {
        if(fs::is_directory(p))
        {
            std::vector<std::string> args;
            boost::split(args, p.stem().string(), boost::is_any_of("-"));
            if (args[0] == collection) {
                // delete pointer contents
                delete collections[args[0]];
                collections.erase(args[0]);
                fs::remove_all(p);
            }
        }
    }
    
}

/*
 * get_occupied_space
 * A function that gets the total in-memory space that all the collections currently occupy
 */

size_t DB::get_occupied_space()
{
    size_t sum = 0;
    for (auto p: collections) {
        sum += p.second->size();
    }
    return sum;
}

double DB::getSentimentScore(std::string collection, std::string name)
{
    // handle case that collection doesnt exist
    if (collections.count(collection)) {
        Collection* c = collections[collection];
        if (c->is_cached(name, "sentiment")) {
            return boost::any_cast<double>(c->get_cached(name, "sentiment"));
        }
    }

    std::string text = get(collection, name);
    double sentiment = sentimentAnalysis.analyse(text);
    collections[collection]->add_to_cache(name, "sentiment", boost::any(sentiment));
    return sentiment;
}

std::string DB::getSentence(std::string collection, std::string name, size_t start)
{
    if (!lru.cached(collection+":"+name)) {
        get(collection, name);
    }
    return collections[collection]->getSentence(name, start);
}


void DB::createCollection(std::string _name, Encoder::CharacterEncoding _encoding)
{
    fs::path collectionPath = datapath / "collections" / (_name+"-"+Encoder::encoding2str(_encoding));
    if (!fs::exists(collectionPath)) {
        fs::create_directories(collectionPath);
    }
    Collection* c = new Collection(collectionPath, _encoding);
    collections[_name] = c;
}

std::vector<std::string> DB::listCollections()
{
    std::vector<std::string> collectionNames;
    for (auto p: collections) {
        collectionNames.push_back(p.first);
    }
    return collectionNames;
}

void DB::commit(const Entry& e)
{
    const Operation& op = e.op;
    string cmd = op.cmd;
    vector<string> args = op.args;
    ostream out(0);
    if (queryFunctions.count(cmd)) {
        this->raft.aow_log(e);
        queryFunctions[cmd](this, out, args);
    } else if (metaFunctions.count(cmd)) {
        metaFunctions[cmd](this, out, args);
    } else {
        cout << "(DB): Unknown query: " << cmd << " , commit failed" << endl;
    }
}

bool DB::modify(string collection, string name, const vector<string>& doc)
{
    collections[collection]->modify(name, doc);
    return true;
}
