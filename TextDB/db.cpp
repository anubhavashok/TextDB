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
#include "html.h"
#include <unordered_map>
#include "similarity.h"
#include "operation.h"
#include <boost/archive/text_iarchive.hpp>
#include <boost/archive/text_oarchive.hpp>
#include "acceptor.h"
#include "proposer.h"
#include "oplog.h"
#include "acceptor.cpp"
#include "proposer.cpp"




namespace fs = boost::filesystem;

// index of word
// max value is ~250,000 since there are only that many english words
using widx = boost::dynamic_bitset<>;
const std::string DB::allowed_puncs = " .,!:;\"()/";


DB::DB(fs::path data, vector<string> replicas)
: sentimentAnalysis(data), datapath(data), oplog(replicas)
{
    fs::path d = data / "collections";
    cout << "Entered TDB Constructor" <<endl;
    if (!fs::exists(d)) {
        fs::create_directories(d);
    }
    cout << "Initialized directories" << endl;
    
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


void DB::init_query_operations()
{
    
    // disk_size
    queryFunctions["collectionsize"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        std::string collection = args[0];
        
        htmlout << db->collections[collection]->disk_size();
    };
    
    // add
    queryFunctions["add"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        std::string collection = args[0];
        std::string name = args[1];
        std::string t = args[2];
        
        std::string rawtext = db->urlDecode(t);
        
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
        
        db->add(collection, name, text);
        for (std::string word: text) {
            htmlout << word << " ";
        }
    };
    
    // remove
    queryFunctions["remove"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        std::string collection = args[0];
        std::string name = args[1];
        
        // TODO: mark words in deprecated text document in word list and lazy remove
        bool success = db->remove(collection, name);
        htmlout << success << endl;
    };
    
    // get
    queryFunctions["get"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        std::string collection = args[0];
        std::string name = args[1];
        
        std::string res;
        res = db->get(collection, name);
        htmlout << res;
    };
    
    // list documents in a collection
    queryFunctions["listdocs"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
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
    
    // search - full text search for a term
    queryFunctions["search"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        
    };
    
    // sentiment - gets sentiment of specified document
    queryFunctions["sentiment"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        std::string collection = args[0];
        std::string name = args[1];
        
        double score = db->getSentimentScore(collection, name);
        //htmlout << "{\"name\":"<<name<<", \"sentimentScore\": "<<score<<"}";
        htmlout << score;
    };
    
    // size - size of specified document
    queryFunctions["size"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        std::string collection = args[0];
        std::string name = args[1];
        
        std::string words = db->collections[collection]->get(name);
        htmlout << boost::count(words, ' ') + 1;
    };
    
    // sentence
    queryFunctions["sentence"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        std::string collection = args[0];
        std::string name = args[1];
        std::string s = args[2];
        
        try {
            size_t start = boost::lexical_cast<int>(s);
            htmlout << db->getSentence(collection, name, start);
        } catch (boost::bad_lexical_cast&) {
        }
        
    };
    queryFunctions["drop"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        std::string collection = args[0];
        
        if (db->collections.count(collection)) {
            db->drop(collection);
        }
    };
    queryFunctions["create"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        std::string collection = args[0];
        std::string name = args[1];
        
        if (!db->collections.count(collection)) {
            db->createCollection(collection, Encoder::str2encoding(name));
        }};

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
    
    queryFunctions["searchdoc"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        std::string collection = args[0];
        std::string name = args[1];
        std::string search_expression = args[2];
        std::string doc = db->get(collection, name);
        boost::regex regex(search_expression);
        boost::sregex_iterator it(doc.begin(), doc.end(), regex);
        boost::sregex_iterator end;
        htmlout << "[" << endl;
        bool start = true;
        for (; it != end; ++it) {
            if (!start) {
                htmlout << ", " << endl;
            } else {
                start = false;
            }
            htmlout <<"\"" << it->str() << "\" ";
        }
        htmlout << "]" << endl;

    };
    
    queryFunctions["htmldoc"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        std::string uri = "http://" + urlDecode(args[0]);
        cout << uri << endl;
        std::string page = HTML::get(uri);
        cout << page << endl;
        std::string text = HTML::parseText(page);
        cout << text << endl;
        htmlout << text << endl;
    };
    
    queryFunctions["termfrequency"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        std::string collection = args[0];
        std::string name = args[1];
        if (db->collections.count(collection)) {
            std::string frequency_table = db->collections[collection]->get_frequency_table(name);
            htmlout << frequency_table;
        }
    };
    
    queryFunctions["tfidf"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
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
        for (auto p: tfidf_m) {
            htmlout << p.first << " " << p.second << endl;
        }
    };
    
    queryFunctions["similarity"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
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
    
    queryFunctions["replog"] = [](DB* db, ostream& htmlout, const std::vector<std::string>& args){
        string operation = args[0];
        Operation op = db->oplog.insert(operation);
        // tcp guarantees ordering and consistency
        assert(op.cmd != "replog");
        // perform operation
        db->queryFunctions[op.cmd](db, htmlout, op.args);
    };
    
    queryFunctions["paxos"] = [](DB* db, ostream& out, const std::vector<std::string>& args){
        string stage = args[0];
        long long n = stoll(args[1]);
        if (stage == "accept") {
            stringstream instream(ios_base::in | ios_base::out);
            instream << curlpp::unescape(args[2]);
            boost::archive::text_iarchive ar(instream);
            Operation op;
            ar >> op;
            
            bool ok = db->oplog.acceptor.accept(n, op);
            if (ok) {
                out << 1;
            } else {
                out << -1;
            }
            
        } else if (stage == "prepare") {
            bool ok = db->oplog.acceptor.promise(n);
            if (ok) {
                long long acceptedProposal = db->oplog.acceptor.acceptedProposal;
                Operation op = db->oplog.acceptor.acceptedValue;
                stringstream outstream(ios_base::out);
                boost::archive::text_oarchive ar(outstream);
                ar << op;
                string outstring = outstream.str();
                string res =  to_string(acceptedProposal) + "|" + curlpp::escape(outstring);
                out << res;
            } else {
                out << -1;
            }
        }
        
    };
    queryFunctions["propose"] = [](DB* db, ostream& out, const std::vector<std::string>& args){
        string cmd = args[0];
        Operation op;
        op.cmd = cmd;
        op.args = vector<string>(args.begin()+1, args.end());
        db->oplog.proposer.propose(op);
    };
}


void DB::handleQuery(std::vector<std::string> in, ostream& htmlout)
{
    std::string cmd = in[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

    std::vector<std::string> args(in.begin() + 1, in.end());
    
    if (queryFunctions.count(cmd)) {
        // call appropriate query
        cout << "(DB) Query: " << cmd << endl;
        queryFunctions[cmd](this, htmlout, args);
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
    if (!collections.count(collection)) {
        cout << "collection not found" << endl;
        return "";
    }
    Collection* c = collections[collection];
    if (!c->exists(name)) {
        return "";
    }
    if (!lru.cached(collection+":"+name)) {
        int total_size = get_occupied_space();
        size_t required = c->size(name);
        
        while ((int)memory_limit - (total_size + required) < (int)memory_epsilon) {
            // kick till enough space left
            std::vector<std::string> last;
            std::string popped = lru.pop();
            if (popped == "") {
                // nothing in cache yet, but this probably means that memory_limit is not enough to store the docs
                break;
            } else {
                boost::split(last, popped, boost::is_any_of(":"));
                Collection* c = collections[last[0]];
                c->kick(last[1]);
                total_size = get_occupied_space();
            }
        }
    }

    lru.access(collection+":"+name);
    return c->get(name);
}

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

int DB::get_occupied_space()
{
    size_t sum = 0;
    for (auto p: collections) {
        sum += p.second->size();
    }
    return (int) sum;
}
/*
 * printIndex
 * A debug function that prints the words present in the Word Index
 */

void DB::printIndex()
{
    std::set<std::string> uniqueWords;
    for (auto p: collections) {
        Collection* c = p.second;
        std::vector<std::string> words = c->getWords();
        uniqueWords.insert(words.begin(), words.end());
    }
    for (std::string word: uniqueWords) {
        cout << word << endl;
    }
}


/*
std::unordered_map<std::string, std::vector<std::string> >  DB::search(std::string queryString) {
    std::vector<std::string> query;
    boost::split(query, queryString, boost::is_any_of(" +"));
    
    std::unordered_map<std::string, std::vector<std::string> > results;
    std::vector<widx> queryIndexes;
    // get all widxs
    for (std::string queryWord: query) {
        if (word2idx.count(queryWord)) {
            queryIndexes.push_back(word2idx[queryWord]);
        }
    }
    if (queryIndexes.empty()) {
        // return results not found;
        return results;
    }
    for (auto docPair : storage) {
        std::string docName = docPair.first;
        std::vector<widx> doc = docPair.second;
        for (size_t i = 0; i < doc.size(); i++) {
            widx word = doc[i];
            size_t j = 0;
            while ((j < queryIndexes.size()) && (i + j < doc.size()) && (widxMatch(doc[i + j], queryIndexes[j]))) {
                j++;
            }
            if (j >= queryIndexes.size() - 1) {
                // full match
                // get 5 before start and 5 after start
                int low = (int)i - std::min((int)i, 5);
                int high = (int)(i + j) + std::min((int)(doc.size() - i), 5);
                std::string resString = "";
                for (size_t k = low; k < high; k++) {
                    std::string resWord = idx2word[doc[k]];
                    if (results.count(docName) == 0) {
                        results[docName] = std::vector<std::string>();
                        assert(results.count(docName) > 0);
                    }
                    resString += resWord + " ";
                }
                results[docName].push_back(resString);
            } else if (j >= queryIndexes.size()/2) {
                // partial match
            } else {
                // TODO: output error or something
            }
        }
    }
    return results;
}
*/
double DB::getSentimentScore(std::string collection, std::string name)
{
    // handle case that collection doesnt exist
    if (collections.count(collection)) {
        Collection* c = collections[collection];
        if (c->is_cached(name, "sentiment")) {
            return boost::any_cast<double>(c->get_cached(name, "sentiment"));
        }
    }
//    if (!lru.cached(collection+":"+name)) {
//        get(collection, name);
//    }
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


static const char HEX2DEC[256] =
{
    /*       0  1  2  3   4  5  6  7   8  9  A  B   C  D  E  F */
    /* 0 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 1 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 2 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 3 */  0, 1, 2, 3,  4, 5, 6, 7,  8, 9,-1,-1, -1,-1,-1,-1,
    
    /* 4 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 5 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 6 */ -1,10,11,12, 13,14,15,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 7 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    
    /* 8 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* 9 */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* A */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* B */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    
    /* C */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* D */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* E */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1,
    /* F */ -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1, -1,-1,-1,-1
};

std::string DB::urlDecode(const std::string & sSrc)
{
    // Note from RFC1630: "Sequences which start with a percent
    // sign but are not followed by two hexadecimal characters
    // (0-9, A-F) are reserved for future extension"
    
    const unsigned char * pSrc = (const unsigned char *)sSrc.c_str();
    const size_t SRC_LEN = sSrc.length();
    const unsigned char * const SRC_END = pSrc + SRC_LEN;
    // last decodable '%'
    const unsigned char * const SRC_LAST_DEC = SRC_END - 2;
    
    char * const pStart = new char[SRC_LEN];
    char * pEnd = pStart;
    
    while (pSrc < SRC_LAST_DEC)
    {
        if (*pSrc == '%')
        {
            char dec1, dec2;
            if (-1 != (dec1 = HEX2DEC[*(pSrc + 1)])
                && -1 != (dec2 = HEX2DEC[*(pSrc + 2)]))
            {
                *pEnd++ = (dec1 << 4) + dec2;
                pSrc += 3;
                continue;
            }
        }
        
        *pEnd++ = *pSrc++;
    }
    
    // the last 2- chars
    while (pSrc < SRC_END)
        *pEnd++ = *pSrc++;
    
    std::string sResult(pStart, pEnd);
    delete [] pStart;
    return sResult;
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

