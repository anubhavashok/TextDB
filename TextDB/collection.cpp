//
//  collection.cpp
//  TextDB
//
//  Created by Anubhav on 12/28/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#include "collection.h"
#include "encoder.h"
#include <stdio.h>
#include <cmath>
#include <ctime>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <set>
#include <algorithm>
#include "Doc.h"
#include "naive_bayes_sentiment.h"

using widx = boost::dynamic_bitset<>;


Collection::Collection(fs::path path, Encoder::CharacterEncoding _encoding)
: collectionPath(path), name(path.stem().string()), bitReader(_encoding), bitWriter(_encoding)
{
    if (!fs::exists(path / "files")) {
        fs::create_directories(path / "files");
    }
    
    loadWordIndex();
    
    // Initialize documentNames
    vector<string> filenames = listFiles();
    documentNames = set<string>(filenames.begin(), filenames.end());
    
    initializeLocalitySensitiveHashing(); // TODO: serialize and load hashes instead of filling up cache with documents each time
    naiveBayesSentiment = NaiveBayesSentiment();
}

void Collection::initializeLocalitySensitiveHashing()
{
    for (string name: documentNames) {
        localitySensitiveHashing.add(get(name), name);
    }
}

unordered_map<string, double> Collection::getAllDuplicates(string name)
{
    if (!documentNames.count(name)) {
        return unordered_map<string, double>();
    }
    string s = get(name);
    return localitySensitiveHashing.test(s, name);
}

/*
 * aow - Append on Write
 */

void Collection::aow(fs::path path, const std::vector<widx>& doc)
{
    bitWriter.clear();
    
    assert(doc.size() < pow(2, 32));
    
    // output number of words in doc
    bitWriter.write(doc.size(), 32);
    
    // output nbits in case nbits changes
    bitWriter.write(nbits, 32);
    
    // output nbits for each word
    for (widx idx: doc) {
        assert(idx.size() <= nbits);
        bitWriter.write(idx.to_ulong(), nbits);
    }
    
    bitWriter.saveToFile(path.string(), true);
    bitWriter.clear();
}

void Collection::loadWordIndex()
{

    fs::path widx_path = collectionPath / "word.idx";
    
    if (!fs::exists(widx_path)) {
        cout << "no widx yet " << endl;
        return;
    }
    
    bitReader.clear();
    
    // read uncompressed
    bitReader.read(widx_path.string(), false);
    
    ifstream fin(widx_path.string(), ios::binary | ios::in);
    
    unsigned int size;
    fin.read((char*)&size, sizeof(int));
    
    bitReader.getNextBits(32).to_ulong();
    
    std::vector<std::string> words;

    /* Read words */
    while (!bitReader.eof()) {
        size_t len = (size_t)bitReader.getNextBits(8).to_ulong();
        std::string word = bitReader.getNextString(len);
        // add to in memory word index
        if(word != "") {
            words.push_back(word);
        }
    }
    
    /* Construct in-memory word index */
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
    
    bitReader.clear();
}


bool Collection::load(std::string name)
{
    bitReader.clear();
    
    fs::path filePath = collectionPath / "files" / (name + ".fyle");
    if (!documentNames.count(name)) {
        cout << "file cannot be loaded as it does not exist" << endl;
        return false;
    }
    bitReader.read(filePath.string(), true);
    size_t num_words = bitReader.getNextBits(32).to_ulong();
    cout << "size: " << num_words << endl;
    size_t num_bits = bitReader.getNextBits(32).to_ulong();
    std::vector<widx> doc;
    for(int i = 0; i < num_words; i++) {
        widx idx = bitReader.getNextBits(num_bits);
        doc.push_back(idx);
        if (bitReader.remainingChars() < 1) {
            break;
        }
    }
    storage[name] = doc;
    return true;
}


void Collection::kick()
{
    // assert no outstanding writes
    storage.clear();
}

void Collection::kick(std::string name)
{
    if (storage.count(name) > 0) {
        storage.erase(storage.find(name));
    }
}

boost::uintmax_t Collection::size()
{
    boost::uintmax_t sum = 0;
    for (auto p: storage) {
        sum += sizeof(p.first);
        sum += sizeof(p.second);
    }
    return sum;
}


boost::uintmax_t Collection::disk_size()
{
    boost::uintmax_t size=0;
    for(fs::recursive_directory_iterator it(collectionPath);
        it!=fs::recursive_directory_iterator();
        ++it)
    {
        if(!fs::is_directory(*it))
            size+=fs::file_size(*it);
    }
    return size;
}

boost::uintmax_t Collection::size(std::string name)
{
    fs::path path = collectionPath / "files" / (name + ".fyle");
    return fs::file_size(path);
}

bool Collection::add(std::string name, std::vector<std::string> doc)
{
    if (idx2word.size() >= pow(2, nbits)) {
        nbits++;
    }


    
    // TODO: change to take a doc
    localitySensitiveHashing.add(reassembleText(doc), name);
    
    fs::path path = collectionPath / "files" / (name + ".fyle");
    // find new words and add them to word index
    std::vector<std::string> new_words = find_new_words(doc);
    aow_words(new_words);
    for (std::string word: new_words) {
        if ((word != "")) {
            addWord(word);
            // add doc to idx->docs mapping
            if (!idx2docs.count(word2idx[word])) {
                idx2docs[word2idx[word]] = std::vector<std::string>();
            }
            idx2docs[word2idx[word]].push_back(name);
        }
    }
    // store in doc object/ docgraph here
    //Doc document(name, doc);
    documentNames.insert(name);
    storage[name] = serialize(doc);
    aow(path, storage[name]);
    return true;
}

widx Collection::addWord(std::string word)
{
    if (idx2word.size() >= pow(2, nbits)-1) {
        nbits++;
        //cout << "increasing index size to " << nbits << endl;
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

widx Collection::uint2widx(unsigned long i)
{
    assert(i < pow(2, nbits));
    widx res = boost::dynamic_bitset<>(nbits, i);
    return res;
}

std::vector<std::string> Collection::find_new_words(const std::vector<std::string>& doc)
{
    std::set<std::string> new_words;
    for (std::string word: doc) {
        if((word != "")&&(word2idx.count(word) == 0)) {
            new_words.insert(word);
        }
    }
    return vector<std::string>(new_words.begin(), new_words.end());
}


void Collection::aow_words(const std::vector<std::string>& new_words)
{
    if (new_words.size() == 0) {
        return;
    }
    bitWriter.clear();
    // ensure aow_words is called prior to updating idx2word with new words
    fs::path widx_path = collectionPath / "word.idx";
    size_t size = (int)idx2word.size() + (int)new_words.size();
    
    // output size of word index
    fstream fout;
    if (!fs::exists(widx_path)) {
        fout.open(widx_path.string(), fstream::out | fstream::binary);
    } else {
        fout.open(widx_path.string(), fstream::in|fstream::out | fstream::binary);
    }
    fout.seekp(0);
    fout.write((char*)&size, sizeof(int));
    fout.close();
    
    // [len|chars]
    for (std::string word: new_words) {
        // output len in bytes
        // NOTE: current length of word is
        if ((unsigned long)word.size() < 256) {
            bitWriter.write(word.size(), 8);
            bitWriter.write(word);
        } else {
            bitWriter.write((unsigned long)255, 8);
            bitWriter.write(word.substr(0, 255));
        }
    }
    bitWriter.appendToFile(widx_path.string(), false);
    bitWriter.clear();
}

std::vector<widx> Collection::serialize(const std::vector<std::string>& doc)
{
    std::vector<widx> idxs;
    for(std::string s: doc) {
        if(s != "") {
            assert(word2idx.count(s) > 0);
            widx idx = word2idx[s];
            idxs.push_back(idx);
        }
    }
    return idxs;
}

std::vector<std::string> Collection::deserialize(const std::vector<widx>& doc)
{
    // convert from widx to std::string by looking up in wordIndex
    std::vector<std::string> stringdoc;
    for (widx idx: doc) {
        if (!idx2word.count(idx)) cout << "BUG in deserialize " << idx.to_ulong() << endl;
        else {
        std::string s = idx2word.find(idx)->second;
        //cout << "Word is: " << s << endl;
        stringdoc.push_back(s);
        }
    }
    return stringdoc;
}

std::string Collection::get(std::string name)
{
    if (storage.count(name))
    {
        return reassembleText(deserialize(storage[name]));
    }
    if(load(name)) {
        return reassembleText(deserialize(storage[name]));
    } else {
        return "";
    }
}

vector<string> Collection::get_vector(std::string name)
{
    if (storage.count(name))
    {
        return deserialize(storage[name]);
    }
    if(load(name)) {
        return deserialize(storage[name]);
    } else {
        return vector<string>();
    }
}

std::string Collection::getSentence(std::string name, size_t start)
{
    std::vector<widx> doc = storage[name];
    std::vector<std::string> subset;
    for (size_t i = start; i < doc.size(); i++) {
        std::string word = idx2word[doc[i]];
        subset.push_back(word);
        if (word.back() == '.') {
            break;
        }
    }
    return reassembleText(subset);
}

bool Collection::remove(std::string name)
{
    if (!documentNames.count(name)) {
        return false;
    }
    // remove from lsh
    localitySensitiveHashing.remove(get(name), name);
    fs::path files = collectionPath / "files";
    time_t timestamp = time(0);
    std::string newName = name+".remove."+std::to_string(timestamp);
    // mark file for removal with timestamp
    fs::rename(files / (name + ".fyle"), files / newName);
    //removeWordsFromMapping(name);
    storage.erase(name);
    documentNames.erase(name);
    // let cron take care of remove
    // remove doc name from idx2docs
    return true;
}

void Collection::removeWordsFromMapping(std::string name)
{
    const vector<widx>& doc = storage[name];
    for (widx idx: doc) {
        idx2docs[idx].erase(std::remove(idx2docs[idx].begin(), idx2docs[idx].end(), name), idx2docs[idx].end());
    }
}

vector<string> Collection::getDocumentNames()
{
    vector<string> names(documentNames.begin(), documentNames.end());
    return names;
}

std::vector<std::string> Collection::listFiles()
{
    fs::path files = collectionPath / "files";
    fs::directory_iterator it(files), eod;
    std::vector<std::string> fileNames;
    BOOST_FOREACH(fs::path p, std::make_pair(it, eod))
    {
        if(is_regular_file(p))
        {
            if (p.extension() == ".fyle") {
                fileNames.push_back(p.stem().string());
            }
        }
    }
    return fileNames;
}

std::vector<std::string> Collection::getWords()
{
    std::vector<std::string> words;
    for (auto p: word2idx) {
        words.push_back(p.first);
    }
    return words;
}

bool Collection::exists(std::string name)
{
    return documentNames.count(name) > 0;
}

std::string Collection::reassembleText(const std::vector<std::string>& words)
{
    std::string text = "";
    int i = 0;
    for (std::string word: words) {
        text += /*((!i || ispunct(word[0])) ? "" : " ")*/ word;
        i++;
    }
    return text;
}


bool Collection::is_cached(std::string name, std::string attr)
{
    if (cache.count(name)) {
        return cache[name].is_cached(attr);
    } else {
        return false;
    }
}

boost::any Collection::get_cached(std::string name, std::string attr)
{
    if (is_cached(name, attr))
        return cache[name].get(attr);
    else
        return NULL;
}

void Collection::clear_cache(std::string name)
{
    cache.erase(name);
}

void Collection::add_to_cache(std::string name, std::string attr, boost::any val)
{
    cache[name].add(attr, val);
}

string Collection::get_frequency_table(string name)
{
    unordered_map<string, int> frequency_table;
    vector<string> words = get_vector(name);
    for (string w: words) {
        transform(w.begin(), w.end(), w.begin(), ::tolower);
        if (!frequency_table.count(w)) {
            frequency_table[w] = 0;
        }
        frequency_table[w]++;
    }
    // sort frequency table by frequency
    vector<pair<string, int>> frequency_table_v(frequency_table.begin(), frequency_table.end());
    sort(frequency_table_v.begin(), frequency_table_v.end(), [](const pair<string, int>& p1, const pair<string, int>& p2){
        return p2.second > p1.second;
    });
    // convert frequency_table into json
    string res = "{ \"tf\": {";
    for (auto p: frequency_table_v) {
        res += "\"" + p.first + "\": " + to_string(p.second) + ", ";
    }
    // remove last ,
    res.pop_back();
    res.pop_back();

    res += "}}";
    return res;
}

vector<vector<string>> Collection::get_all()
{
    vector<vector<string>> docs;
    for (string p: documentNames) {
        docs.push_back(get_vector(p));
    }
    return docs;
}

vector<string> Collection::get_all_string()
{
    vector<string> docs;
    for(string p: documentNames) {
        docs.push_back(get(p));
    }
    return docs;
}

bool Collection::modify(string name, vector<string> doc)
{
    return true;
    /*
    Doc d = docs[name];
    d.update(doc);
     */
}


void Collection::mark(string name, string sentimentClass)
{
    naiveBayesSentiment.mark(name, sentimentClass);
}

void Collection::train()
{
    naiveBayesSentiment.train();
}

pair<string, double> Collection::test(string name)
{
    if (!exists(name)) {
        return make_pair("", -2);
    }
    return naiveBayesSentiment.test(get(name));
}

vector<string> Collection::getInterestingDocuments(int n)
{
    // TODO: actually implement useful functionality
    // E.g. top freq words (removed stopwords), find n documents for top n, score top n^2 docs, according to relevance, return top n
    return randomSampleDocuments(n);
}

vector<string> Collection::getRelatedDocuments(string documentName, int n)
{
    // TODO: actually implement useful functionality
    // Should be fairly straightforward
    return randomSampleDocuments(n);
}

vector<string> Collection::randomSampleDocuments(int k)
{
    vector<string> sampledDocuments(k);
    vector<string> list(documentNames.begin(), documentNames.end());
    // Resevoir sampling
    for (int i = 0; i < list.size(); i++) {
        if (i < k) {
            sampledDocuments.push_back(list[i]);
        } else {
            int r = rand() & i;
            // replace random document
            if (r < k) {
                sampledDocuments[r] = sampledDocuments[i];
            }
        }
    }
    return sampledDocuments;
}
