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
#include "preformatter.h"
#include <boost/algorithm/string.hpp>
#include <string.h>
#include <boost/foreach.hpp>
#include "collection.h"
#include <boost/tokenizer.hpp>
#include <boost/range/algorithm/count.hpp>

namespace fs = boost::filesystem;

// index of word
// max value is ~250,000 since there are only that many english words
using widx = boost::dynamic_bitset<>;
const std::string DB::allowed_puncs = " .,!:;\"()/";



DB::DB(fs::path data)
: sentimentAnalysis(data / fs::path("positive.txt"), data / fs::path("negative.txt")), datapath(data)
{
    fs::path d = data / "collections";
    if (!fs::exists(d)) {
        fs::create_directories(d);
    }
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

std::pair<std::string, std::string> DB::parseCollectionsDirName(std::string filename)
{
    // split at ., return both in pair
    std::vector<std::string> args;
    boost::split(args, filename, boost::is_any_of("-"));
    return std::make_pair(args[0], args[1]);
}

/*
 * serializeDoc
 * A function that converts each word in a text document into its equivalent index in the Word Index
 * Adds an entry to the word index on encountering a new unique word
 * @param doc a vector of strings representing a text document
 * @return a vector of widxs representing the right index to each word
 */


/*
 * uint2widx
 * A helper function to generate a bitset of nbits containing input value
 * NOTE: nbits is a member variable to db indicating the current size of a Word Index index
 * @param i an unsigned long that will be the numerical value of the bitset when created
 * @return a widx (boost::dynamic_bitset<>) that is nbits large and holds the value of i
 */

/*
 * addWord
 * A function that adds the word to the Word Index if new and unique OR returns the words appropriate widx
 * @param word a string containing the word that is to be added/looked up
 * @return a widx representing the index of the input word in the Word Index
 */


/*
 * handleQuery
 * A function that takes in a query string and performs the correct query (ADD, ADDDOC, GET), outputting updates to the provided out stream
 * @param in a vector of tokenized strings representing the input command and args
 * @param htmlout an ostream& referring to the appropriate output stream (in this case plaintext html)
 */


void DB::init_query_operations()
{
    
    // disk_size
    queryFunctions["collectionsize"] = [](DB* db, ostream& htmlout, std::vector<std::string> args){
        std::string collection = args[0];
        htmlout << db->collections[collection]->disk_size();
    };
    
    // add
    queryFunctions["add"] = [](DB* db, ostream& htmlout, std::vector<std::string> args){
        std::string collection = args[0];
        std::string name = args[1];
        std::string t = args[2];
        
        htmlout << "ADD " << name << "\n";
        
        // have either text or path to doc
        std::string rawtext = db->urlDecode(t);
        std::vector<std::string> text;
        // TODO: fix this, keep punctuation
        // boost::split(text, rawtext, boost::is_any_of(DB::allowed_puncs));
        
        boost::char_separator<char> sep("", DB::allowed_puncs.c_str()); // specify only the kept separators
        boost::tokenizer<boost::char_separator<char>> tokens(rawtext, sep);
        for (std::string t : tokens) {
            boost::trim(t);
            if (t != "") {
                text.push_back(t);
            }
        }
        
        
        // index word and add to db
        db->add(collection, name, text);
        for (std::string word: text) {
            htmlout << word << " ";
        }
    };
    
    // remove
    queryFunctions["remove"] = [](DB* db, ostream& htmlout, std::vector<std::string> args){
        std::string collection = args[0];
        std::string name = args[1];
        
        // TODO: mark words in deprecated text document in word list and lazy remove
        bool success = db->remove(collection, name);
        htmlout << success << endl;
    };
    
    // get
    queryFunctions["get"] = [](DB* db, ostream& htmlout, std::vector<std::string> args){
        std::string collection = args[0];
        std::string name = args[1];
        
        std::string res;
        res = db->get(collection, name);
        htmlout << res;
    };
    
    // list documents in a collection
    queryFunctions["listdocs"] = [](DB* db, ostream& htmlout, std::vector<std::string> args){
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
    queryFunctions["search"] = [](DB* db, ostream& htmlout, std::vector<std::string> args){
        
    };
    
    // sentiment - gets sentiment of specified document
    queryFunctions["sentiment"] = [](DB* db, ostream& htmlout, std::vector<std::string> args){
        std::string collection = args[0];
        std::string name = args[1];
        
        double score = db->getSentimentScore(collection, name);
        //htmlout << "{\"name\":"<<name<<", \"sentimentScore\": "<<score<<"}";
        htmlout << score;
    };
    
    // size - size of specified document
    queryFunctions["size"] = [](DB* db, ostream& htmlout, std::vector<std::string> args){
        std::string collection = args[0];
        std::string name = args[1];
        
        std::string words = db->collections[collection]->get(name);
        htmlout << boost::count(words, ' ') + 1;
    };
    
    // sentence
    queryFunctions["sentence"] = [](DB* db, ostream& htmlout, std::vector<std::string> args){
        std::string collection = args[0];
        std::string name = args[1];
        std::string s = args[2];
        
        try {
            size_t start = boost::lexical_cast<int>(s);
            htmlout << db->getSentence(collection, name, start);
        } catch (boost::bad_lexical_cast&) {
        }
        
    };
    queryFunctions["drop"] = [](DB* db, ostream& htmlout, std::vector<std::string> args){
        std::string collection = args[0];
        
        if (db->collections.count(collection)) {
            db->drop(collection);
        }
    };
    queryFunctions["create"] = [](DB* db, ostream& htmlout, std::vector<std::string> args){
        std::string collection = args[0];
        std::string name = args[1];
        
        if (!db->collections.count(collection)) {
            db->createCollection(collection, Encoder::str2encoding(name));
        }};

    queryFunctions["listcollections"] = [](DB* db, ostream& htmlout, std::vector<std::string> args){
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
}


void DB::handleQuery(std::vector<std::string> in, ostream& htmlout)
{
    std::string cmd = in[0];
    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);

    std::vector<std::string> args(in.begin() + 1, in.end());
    
    if (queryFunctions.count(cmd)) {
        // call appropriate query
        queryFunctions[cmd](this, htmlout, args);
    } else {
        cout << "(DB): Unknown query" << endl;
    }
}
/*
void DB::handleQuery(std::vector<std::string> in, ostream& htmlout)
{
    
    
    std::string cmd = in[0];
    std::string collection = in[1];
    std::string name = in[2];

    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    if (cmd == "add") {
        assert(in.size() >= 3);
        htmlout << "ADD " << name << "\n";

        // have either text or path to doc
        std::string rawtext = urlDecode(in[3]);
        std::vector<std::string> text;
        // TODO: fix this, keep punctuation
        // boost::split(text, rawtext, boost::is_any_of(DB::allowed_puncs));
        
        boost::char_separator<char> sep("", DB::allowed_puncs.c_str()); // specify only the kept separators
        boost::tokenizer<boost::char_separator<char>> tokens(rawtext, sep);
        for (std::string t : tokens) {
            boost::trim(t);
            if (t != "") {
                text.push_back(t);
            }
        }


        // index word and add to db
        add(collection, name, text);
        for (std::string word: text) {
            htmlout << word << " ";
        }

    } else if (cmd == "remove") {
        // TODO: mark words in deprecated text document in word list and lazy remove
        bool success = remove(collection, name);
        htmlout << success << endl;
    } else if (cmd == "get") {
        std::string res;
        res = get(collection, name);
        htmlout << res;
    } else if (cmd == "listcollections") {
        // get all collectionNames
        std::string json = "[";
        int i = 0;
        for (std::string s: listCollections()) {
            json += ((i == 0) ? "": ",");
            json += '"' + s + '"';
            i++;
        }
        json += "]";
        htmlout << json;
    } else if (cmd == "listdocs") {
        // get all doc names
        Collection* c = collections[collection];
        std::string json = "[";
        int i = 0;
        for (std::string s: c->listFiles()) {
            json += ((i == 0)? "": ",");
            json += '"' + s + '"';
            i++;
        }
        json += "]";
        htmlout << json;
//    }else if (cmd == "search") {
//        std::string queryString = in[1].substr(2) + "+";
//        htmlout << "Search: ";
//        std::vector<std::string> query;
//        boost::split(query, queryString, boost::is_any_of(" +"));
//        for (std::string w: query) {
//            htmlout << w << " ";
//        }
//        htmlout << endl;
//        std::map<std::string, std::vector<std::string> > results = this->search(queryString);
//        std::string resultString = "{\n";
//        for (auto resultPair : results) {
//            resultString += "\"";
//            resultString += resultPair.first;
//            resultString += "\":";
//
//            resultString += "[";
//            for (std::string str : resultPair.second) {
//                resultString += "\"" + str + "\",\n";
//            }
//            resultString += "],\n";
//        }
//        resultString += "}";
//        htmlout << resultString << endl;
    } else if (cmd == "sentiment") {
        double score = getSentimentScore(collection, name);
        //htmlout << "{\"name\":"<<name<<", \"sentimentScore\": "<<score<<"}";
        htmlout << score;
    } else if (cmd == "size") {
        std::string words = collections[collection]->get(name);
        htmlout << boost::count(words, ' ') + 1;
    } else if (cmd == "sentence") {
        try {
            size_t start = boost::lexical_cast<int>(in[3]);
            htmlout << getSentence(collection, name, start);
        } catch (boost::bad_lexical_cast&) {
        }
    } else if (cmd == "drop") {
        if (collections.count(collection)) {
            drop(collection);
        }
    } else if (cmd == "create") {
        if (!collections.count(collection)) {
            createCollection(collection, Encoder::str2encoding(name));
        }
    } else if (cmd == "collectionsize") {
        htmlout << collections[collection]->disk_size();
    } else {
        cout << "(DB): Unknown query" << endl;
    }
}
*/

/*
 * add
 * A function that adds a <string, vector<string> > key value pair to the DB
 * @param name a string that represetnts the key
 * @param text a vector of strings representing the value (text document)
 */

bool DB::add(std::string collection, std::string name, std::vector<std::string> text)
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
        int required = c->size(name);
        
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



bool widxMatch (const widx& a, const widx& b) {
    return (a.to_ulong() == b.to_ulong());
}
//
//std::map<std::string, std::vector<std::string> >  DB::search(std::string queryString) {
//    std::vector<std::string> query;
//    boost::split(query, queryString, boost::is_any_of(" +"));
//    
//    std::map<std::string, std::vector<std::string> > results;
//    std::vector<widx> queryIndexes;
//    // get all widxs
//    for (std::string queryWord: query) {
//        if (word2idx.count(queryWord)) {
//            queryIndexes.push_back(word2idx[queryWord]);
//        }
//    }
//    if (queryIndexes.empty()) {
//        // return results not found;
//        return results;
//    }
//    for (auto docPair : storage) {
//        std::string docName = docPair.first;
//        std::vector<widx> doc = docPair.second;
//        for (size_t i = 0; i < doc.size(); i++) {
//            widx word = doc[i];
//            size_t j = 0;
//            while ((j < queryIndexes.size()) && (i + j < doc.size()) && (widxMatch(doc[i + j], queryIndexes[j]))) {
//                j++;
//            }
//            if (j >= queryIndexes.size() - 1) {
//                // full match
//                // get 5 before start and 5 after start
//                int low = (int)i - std::min((int)i, 5);
//                int high = (int)(i + j) + std::min((int)(doc.size() - i), 5);
//                std::string resString = "";
//                for (size_t k = low; k < high; k++) {
//                    std::string resWord = idx2word[doc[k]];
//                    if (results.count(docName) == 0) {
//                        results[docName] = std::vector<std::string>();
//                        assert(results.count(docName) > 0);
//                    }
//                    resString += resWord + " ";
//                }
//                results[docName].push_back(resString);
//            } else if (j >= queryIndexes.size()/2) {
//                // partial match
//            } else {
//                // TODO: output error or something
//            }
//        }
//    }
//    return results;
//}

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


const char HEX2DEC[256] =
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

std::string DB::urlDecode(std::string & sSrc)
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
    // add collection to db
    Collection* c = new Collection(collectionPath, _encoding);
    collections[_name] = c;
    // create collection file to save
}

std::vector<std::string> DB::listCollections()
{
    std::vector<std::string> collectionNames;
    for (auto p: collections) {
        collectionNames.push_back(p.first);
    }
    return collectionNames;
}

