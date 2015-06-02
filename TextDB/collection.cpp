//
//  collection.cpp
//  TextDB
//
//  Created by Anubhav on 12/28/14.
//  Copyright (c) 2014 anubhav. All rights reserved.
//

#include "collection.h"
#include "encoder.h"
#include <cmath>
#include <ctime>
#include <boost/foreach.hpp>
#include <boost/algorithm/string.hpp>
#include <set>
#include <algorithm>
#include "Doc.h"

using widx = boost::dynamic_bitset<>;


Collection::Collection(fs::path path, Encoder::CharacterEncoding _encoding)
: collectionPath(path), name(path.stem().string()), bitReader(_encoding), bitWriter(_encoding)
{
    // load from file or create file
    if (!fs::exists(path / "files")) {
        fs::create_directories(path / "files");
    }
    loadWordIndex();
}


/*
 * aow
 * A function that byte encodes the DB and saves it to disk
 * @param path a string containing the absolute path of the store.bindb file
 */
// TODO: make function modifiable

void Collection::aow(fs::path path, const std::vector<widx>& doc)
{
    bitWriter.clear();
    // create file to store all docs
    
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

    while (!bitReader.eof()) {
        size_t len = (size_t)bitReader.getNextBits(8).to_ulong();
        std::string word = bitReader.getNextString(len);
        // add to in memory word index
        if(word != "") {
            words.push_back(word);
        }
    }
    
    // build word index in memory
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
    if (!fs::exists(filePath)) {
        cout << "file cannot be loaded as it does not exist" << endl;
        return false;
    }
    bitReader.read(filePath.string(), true);
    //size_t size = bitReader.getNextBits(32).to_ulong();
    size_t nbits = bitReader.getNextBits(32).to_ulong();
    std::vector<widx> doc;
    while (!bitReader.eof()) {
        widx idx = bitReader.getNextBits(nbits);
        doc.push_back(idx);
        if (bitReader.remainingChars() <= 1) {
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

size_t Collection::size()
{
    size_t sum = 0;
    for (auto p: storage) {
        sum += sizeof(p.first);
        sum += sizeof(p.second);
    }
    return sum;
}


size_t Collection::disk_size()
{
    size_t size=0;
    for(fs::recursive_directory_iterator it(collectionPath);
        it!=fs::recursive_directory_iterator();
        ++it)
    {
        if(!fs::is_directory(*it))
            size+=fs::file_size(*it);
    }
    return size;
}

size_t Collection::size(std::string name)
{
    fs::path path = collectionPath / "files" / (name + ".fyle");
    return fs::file_size(path);
}

bool Collection::add(std::string name, std::vector<std::string> doc)
{
    // TODO: decide where to put this
    if (idx2word.size() >= pow(2, nbits)) {
        nbits++;
    }

    fs::path path = collectionPath / "files" / (name + ".fyle");
    
    // find new words and add them to word index
    std::vector<std::string> new_words = find_new_words(doc);
    aow_words(new_words);
    for (std::string word: new_words) {
        if (word != "")
            addWord(word);
        // add doc to idx->docs mapping
        if (!idx2docs.count(word2idx[word])) {
            idx2docs[word2idx[word]] = std::vector<std::string>();
        }
        idx2docs[word2idx[word]].push_back(name);
    }
    // store in doc object/ docgraph here
    Doc document(name, doc);
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
        // NOTE: current length of word is 32
        bitWriter.write(word.size(), 8);
        bitWriter.write(word);
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
        if (word[0] == '.') {
            break;
        }
    }
    return reassembleText(subset);
}

bool Collection::remove(std::string name)
{
    fs::path files = collectionPath / "files";
    if (!fs::exists(files / (name + ".fyle"))) {
        return false;
    }
    time_t timestamp = time(0);
    std::string newName = name+".remove."+std::to_string(timestamp);
    // mark file for removal with timestamp
    fs::rename(files / (name + ".fyle"), files / newName);
    //removeWordsFromMapping(name);
    storage.erase(name);
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
    return fs::exists(collectionPath / "files" / (name + ".fyle"));
}

std::string Collection::reassembleText(const std::vector<std::string>& words)
{
    std::string text = "";
    int i = 0;
    for (std::string word: words) {
        text += ((!i || ispunct(word[0])) ? "" : " ") + word;
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
        if (!frequency_table.count(w)) {
            frequency_table[w] = 0;
        }
        frequency_table[w]++;
    }
    // convert frequency_table into json
    string res = "{ \"tf\": {";
    for (auto p: frequency_table) {
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
    for (string p: listFiles()) {
        docs.push_back(get_vector(p));
    }
    return docs;
}

bool modify(string name, vector<string> doc)
{
    Doc d = docs[name];
    d.update(doc);
}