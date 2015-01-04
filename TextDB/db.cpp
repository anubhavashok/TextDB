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

namespace fs = boost::filesystem;

// index of word
// max value is ~250,000 since there are only that many english words
using widx = boost::dynamic_bitset<>;


DB::DB(fs::path data)
: sentimentAnaylsis(data / fs::path("negative.txt"), data / fs::path("positive.txt")), datapath(data)
{
    // load collections
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
    
    // get all collection names
    // load all collections
    for (std::string collectionName: collectionNames) {
        std::string name, type;
        std::tie(name, type) = parseCollectionsDirName(collectionName);
        Encoder::CharacterEncoding encoding = Encoder::str2encoding(type);
        Collection* c = new Collection(datapath / "collections" / collectionName, encoding);
        collections[name] = c;
    }
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
        boost::split(text, rawtext, boost::is_any_of(" .,!:;\"()/"));

        // index word and add to db
        add(collection, name, text);
        for (std::string word: text) {
            htmlout << word << " ";
        }

    } else if (cmd == "remove") {
        // TODO: mark words in deprecated text document in word list and lazy remove
        bool success = remove(collection, name);
        htmlout << success << endl;
    } else if (cmd == "adddoc") {
        assert(in.size() >= 3);
        htmlout << "ADDDOC " << name << "\n";
        
        // have either text or path to doc
        fs::path docPath = in[3];
        // read doc and load
        std::vector<std::string> text;
        ifstream fin(docPath.string());
        std::string line;
        std::vector<std::string> words;
        while (!fin.eof()) {
            std::getline(fin, line);
            boost::split(words, line, boost::is_any_of(" .,!:;\"()/"));
            for(std::string word: words) {
                text.push_back(word);
            }
            text.push_back("\n");
        }
        add(collection, name, text);
        htmlout << "Added " << in[3] << endl;
        
    } else if (cmd == "get") {
        //assert(in.size() == 2);
        std::vector<std::string> res;
        res = get(collection, name);
        for (std::string word: res) {
            htmlout << word << " ";
        }
    } else if (cmd == "listcollections") {
        // get all collectionNames
        std::string json = "[";
        for (std::string s: listCollections()) {
            json += "'" + s + "'";
        }
        json += "]";
        htmlout << json;
    } else if (cmd == "listdocs") {
        // get all doc names
        Collection* c = collections[collection];
        std::string json = "[";
        for (std::string s: c->listFiles()) {
            json += "'" + s + "'";
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
        htmlout << collections[collection]->size(name);
    } else if (cmd == "sentence"){
        size_t start = std::stoi(in[3], nullptr, 10);
        htmlout << getSentence(collection, name, start);
    } else {
        cout << "Unknown query" << endl;
    }
}


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

std::vector<std::string> DB::get(std::string collection, std::string name)
{
    if (!collections.count(collection)) {
        cout << "collection not found" << endl;
        return std::vector<std::string>();
    }
    Collection* c = collections[collection];
    if (!c->exists(name)) {
        return std::vector<std::string>();
    }
    if (!lru.cached(collection+":"+name)) {
        size_t total_size = get_occupied_space();
        size_t required = c->size(name);
        
        while (memory_limit - (total_size + required) < memory_epsilon) {
            // kick till enough space left
            std::vector<std::string> last;
            std::string popped = lru.pop();
            boost::split(last, popped, boost::is_any_of(":"));
            Collection* c = collections[last[0]];
            c->kick(last[1]);
        }
    }

    lru.access(collection+":"+name);
    return c->get(name);
}

size_t DB::get_occupied_space()
{
    size_t sum = 0;
    for (auto p: collections) {
        sum += p.second->size();
    }
    return sum;
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

///*
// * saveUncompressed
// * A function that saves the DB in a uncompressed format to enable benchmarks and comparisons
// * @param path a string containing the location of the uncompressed text file
// */
//
//void DB::saveUncompressed(std::string path)
//{
//    ofstream fout(path);
//    fout << idx2word.size() << endl;
//    
//    // Don't output wordIndex for true comparision
//    for (std::pair<widx, std::string> wordPair: idx2word) {
//        //fout << wordPair.second.length() << "|" << wordPair.second << endl;
//    }
//    fout << "DOCUMENTS" << endl;
//    for (std::pair<std::string, std::vector<widx>> doc: storage) {
//        std::string name  = doc.first;
//        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
//        fout << name << endl;
//        for (widx idx: doc.second) {
//            fout << idx2word[idx] << " ";
//        }
//        fout << endl;
//    }
//    fout.close();
//}



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
    vector<string> text = get(collection, name);
    return sentimentAnaylsis.analyse(text);
}

std::string DB::getSentence(std::string collection, std::string name, size_t start)
{
    return collections[collection]->getSentence(name, start);
}

// URL decode function stolen from: http://stackoverflow.com/questions/154536/encode-decode-urls-in-c
/*std::string DB::urlDecode(string &SRC)
{
    std::string ret;
    char ch;
    int i, ii;
    for (i=0; i<SRC.length(); i++) {
        if (int(SRC[i])==37) {
            sscanf(SRC.substr(i+1,2).c_str(), "%x", &ii);
            ch=static_cast<char>(ii);
            ret+=ch;
            i=i+2;
        } else {
            ret+=SRC[i];
        }
    }
    return (ret);
}
*/

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
    cout << collectionPath;
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

