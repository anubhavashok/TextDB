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

namespace fs = boost::filesystem;

// index of word
// max value is ~250,000 since there are only that many english words
using widx = boost::dynamic_bitset<>;


DB::DB(fs::path data)
: sentimentAnaylsis(data / fs::path("negative.txt"), data / fs::path("positive.txt"))
{
}

/*
 * serializeDoc
 * A function that converts each word in a text document into its equivalent index in the Word Index
 * Adds an entry to the word index on encountering a new unique word
 * @param doc a vector of strings representing a text document
 * @return a vector of widxs representing the right index to each word
 */

std::vector<widx> DB::serializeDoc(std::vector<std::string> doc)
{
    std::vector<widx> res;
    for (std::string word: doc) {
        assert(word.length() < 32);
        widx idx = addWord(word);
        res.push_back(idx);
    }
    return res;
}

/*
 * uint2widx
 * A helper function to generate a bitset of nbits containing input value
 * NOTE: nbits is a member variable to db indicating the current size of a Word Index index
 * @param i an unsigned long that will be the numerical value of the bitset when created
 * @return a widx (boost::dynamic_bitset<>) that is nbits large and holds the value of i
 */

widx DB::uint2widx(unsigned long i)
{
    assert(i < pow(2, nbits));
    widx res = boost::dynamic_bitset<>(nbits, i);
    return res;
}

/*
 * addWord
 * A function that adds the word to the Word Index if new and unique OR returns the words appropriate widx
 * @param word a string containing the word that is to be added/looked up
 * @return a widx representing the index of the input word in the Word Index
 */

widx DB::addWord(std::string word)
{
    std::transform(word.begin(), word.end(), word.begin(), ::tolower);
    if (idx2word.size() >= pow(2, nbits)-1) {
        nbits++;
        cout << "increasing index size to " << nbits << endl;
    }
    if (word2idx.count(word)) {
        widx idx = word2idx[word];
        assert(idx.to_ulong() < pow(2, nbits));
        assert(idx2word[idx] == word);
        return idx;
    } else {
        // careful if parallel
        // len might have changed
        size_t len = idx2word.size();
        widx idx = uint2widx((unsigned long)len);
        word2idx[word] = idx;
        idx2word[idx] = word;
        return idx;
    }
}

/*
 * handleQuery
 * A function that takes in a query string and performs the correct query (ADD, ADDDOC, GET), outputting updates to the provided out stream
 * @param in a vector of tokenized strings representing the input command and args
 * @param htmlout an ostream& referring to the appropriate output stream (in this case plaintext html)
 */

void DB::handleQuery(std::vector<std::string> in, ostream& htmlout)
{
    std::string cmd = in[0];
    std::string name = in[1];

    std::transform(cmd.begin(), cmd.end(), cmd.begin(), ::tolower);
    if (cmd == "add") {
        assert(in.size() >= 3);
        htmlout << "ADD " << name << "\n";

        // have either text or path to doc
        std::vector<std::string> text;
        for (size_t i = 2; i < in.size(); i++) {
            text.push_back(in[i]);
        }
        // index word and add to db
        add(name, text);
        for (std::string word: text) {
            htmlout << word << " ";
        }

    } else if (cmd == "adddoc") {
        assert(in.size() >= 3);
        htmlout << "ADDDOC " << name << "\n";
        
        // have either text or path to doc
        fs::path docPath = in[2];
        // read doc and load
        std::vector<std::string> text;
        ifstream fin(docPath.string());
        std::string line;
        std::vector<std::string> words;
        while (!fin.eof()) {
            std::getline(fin, line);
            boost::split(words, line, boost::is_any_of(" "));
            for(std::string word: words) {
                text.push_back(word);
            }
            text.push_back("\n");
        }
        add(name, text);
        htmlout << "Added " << in[2] << endl;
        
    } else if (cmd == "get") {
        assert(in.size() == 2);
        htmlout << "GET " << name << endl;
        double score = getSentimentScore(name);
        htmlout << "Sentiment: " << score << endl;
        
        std::vector<std::string> res;
        res = get(name);
        for (std::string word: res) {
            htmlout << word << " ";
        }
    } else if (cmd == "list") {
        // get all keys
        for (auto entry: storage) {
            htmlout << entry.first << endl;
        }
    } else if (cmd == "search") {
        std::string queryString = in[1].substr(2) + "+";
        htmlout << "Search: ";
        std::vector<std::string> query;
        boost::split(query, queryString, boost::is_any_of(" +"));
        for (std::string w: query) {
            htmlout << w << " ";
        }
        htmlout << endl;
        std::map<std::string, std::vector<std::string> > results = this->search(queryString);
        std::string resultString = "{\n";
        for (auto resultPair : results) {
            resultString += "\"";
            resultString += resultPair.first;
            resultString += "\":";

            resultString += "[";
            for (std::string str : resultPair.second) {
                resultString += "\"" + str + "\",\n";
            }
            resultString += "],\n";
        }
        resultString += "}";
        htmlout << resultString << endl;
    } else if (cmd == "sentiment") {
        double score = getSentimentScore(name);
        htmlout << "{\"name\":"<<name<<", \"sentimentScore\": "<<score<<"}";
    }else {
        cout << "Unknown query" << endl;
    }
}


/*
 * add
 * A function that adds a <string, vector<string> > key value pair to the DB
 * @param name a string that represetnts the key
 * @param text a vector of strings representing the value (text document)
 */

void DB::add(std::string name, std::vector<std::string> text)
{
    Preformatter::removePunctuations(text);
    Preformatter::toLower(text);

    if (idx2word.size() >= pow(2, nbits)) {
        nbits++;
    }
    std::vector<widx> serializedDoc = serializeDoc(text);
    storage[name] = serializedDoc;
}


/*
 * get
 * A function queries the DB and returns a deserialized text doc corresponding to the key provided
 * @param name a string representing the requested key
 * @return a vector of strings representing a deserialized text doc
 */

std::vector<std::string> DB::get(std::string name)
{
    std::vector<std::string> deserializedDoc;
    if (storage.count(name)) {
        std::vector<widx> serializedDoc = storage[name];
        for (widx idx: serializedDoc) {
            std::string word = idx2word.find(idx)->second;
            deserializedDoc.push_back(word);
        }
    }
    return deserializedDoc;
}


/*
 * encodeAndSave
 * A function that byte encodes the DB and saves it to disk
 * @param path a string containing the absolute path of the store.bindb file
 */

void DB::encodeAndSave(std::string path)
{
    BitReader bitReader;
    
    // set num words (max val 2^18 for possible eng words)
    // is set only once at start of the file so it can be 18
    // nbits inferred from this value
    size_t len = idx2word.size();
    assert(len != 0);
    bitReader.setNextBits(len, 18);
    
    assert(idx2word.size() > 0);
    for(std::pair<widx, std::string> wordPair: idx2word) {
        std::string word = wordPair.second;
        assert(word.size() < 32);
        // word should already be in lower case but just in case
        std::transform(word.begin(), word.end(), word.begin(), ::tolower);
        
        // encode wordlen (max val = 32 chars in each word)
        size_t wordlen = word.size();
        bitReader.setNextBits(wordlen, 5);
        
        // encode word (max val = 32 but only 26 used to encode each english lower case char)
        bitReader.setNextString(word);
    }
    // save to file
    bitReader.saveToFile(path);
    
    // ENCODE KEY-DOC mapping
    BitReader docBitReader;
    
    // create file to store all docs
    for (std::pair<std::string, std::vector<widx>> doc: storage) {
        
        // output number of chars in name (max 32)
        std::string name = doc.first;
        assert(name.size() < 32);
        docBitReader.setNextBits(name.size(), 5);
        
        // output name
        docBitReader.setNextString(name);
        
        // output number of words in doc
        docBitReader.setNextBits(doc.second.size(), 32);
        
        // output nbits for each word
        // remember: nbits is inferrred from num words at the start of decoding
        for (widx idx: doc.second) {
            assert(idx.size() <= nbits);
            docBitReader.setNextBits(idx.to_ulong(), nbits);
        }
    }
    
    docBitReader.saveToFile(path + ".kvp");
}

/*
 * decodeAndLoad
 * A function that decodes a byte encoded form of the DB and creates an in memory usable form
 * @param path a string containing the absolute path to the store.bindb file
 */

void DB::decodeAndLoad(std::string path)
{
    BitReader bitReader(path);

    // read num words - first 18 bits
    size_t len = bitReader.getNextBits(18).to_ulong();
    assert(len > 0);
    nbits = ceil(log(len)/log(2));
    nbits = 1;
    std::vector<std::string> words;
    while (!bitReader.eof()) {
        
        // read word len
        boost::dynamic_bitset<> len = bitReader.getNextBits(5);
        
        // read word
        size_t nchars = len.to_ulong();
        std::string word = bitReader.getNextString(nchars);
        words.push_back(word);
    }

    idx2word.empty();
    word2idx.empty();
    for (size_t i = 0; i < words.size(); i ++) {
        if (i >= pow(2, nbits)) {
            nbits++;
        }
        widx idx(nbits, i);
        idx2word[idx] = words[i];
        word2idx[words[i]] = idx;
    }
    // assert(len.to_ulong() == words.size());
    
    // DECODE KEY-DOC STORAGE
    std::string docPath = path + ".kvp";
    BitReader docBitReader(docPath);
    while (!docBitReader.eof()) {
        
        // read size of name
        size_t keyLen = docBitReader.getNextBits(5).to_ulong();

        // read name
        std::string key = docBitReader.getNextString(keyLen);
        cout << "key is: " << key << endl;

        // read length of words (standard 32 bit unsigned int)
        size_t nwords = docBitReader.getNextBits(32).to_ulong();

        // read each word idx
        std::vector<widx> values;
        for (size_t n = 0; n < nwords; n++) {
            widx idx = docBitReader.getNextBits(nbits);
            // TODO: make sure each idx is stored optimally
            values.push_back(idx);
        }
        storage[key] = values;
        if (docBitReader.remainingChars() <= 1) {
            break;
        }
    }
    
}

/*
 * printIndex
 * A debug function that prints the words present in the Word Index
 */

void DB::printIndex()
{
    for (std::pair<widx, std::string> wordPair: idx2word) {
        cout << wordPair.second << endl;
    }
}

/*
 * saveUncompressed
 * A function that saves the DB in a uncompressed format to enable benchmarks and comparisons
 * @param path a string containing the location of the uncompressed text file
 */

void DB::saveUncompressed(std::string path)
{
    ofstream fout(path);
    fout << idx2word.size() << endl;
    
    // Don't output wordIndex for true comparision
    for (std::pair<widx, std::string> wordPair: idx2word) {
        //fout << wordPair.second.length() << "|" << wordPair.second << endl;
    }
    fout << "DOCUMENTS" << endl;
    for (std::pair<std::string, std::vector<widx>> doc: storage) {
        std::string name  = doc.first;
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
        fout << name << endl;
        for (widx idx: doc.second) {
            fout << idx2word[idx] << " ";
        }
        fout << endl;
    }
    fout.close();
}



bool widxMatch (const widx& a, const widx& b) {
    return (a.to_ulong() == b.to_ulong());
}

std::map<std::string, std::vector<std::string> >  DB::search(std::string queryString) {
    std::vector<std::string> query;
    boost::split(query, queryString, boost::is_any_of(" +"));
    
    std::map<std::string, std::vector<std::string> > results;
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

double DB::getSentimentScore(std::string name)
{
    vector<string> text = get(name);
    return sentimentAnaylsis.analyse(text);
}