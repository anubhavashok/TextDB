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


Collection::Collection(fs::path path, Encoder::CharacterEncoding _encoding)
: collectionPath(path), name(path.root_name().string()), bitReader(_encoding), bitWriter(_encoding)
{
    // load from file or create file
}


/*
 * aow
 * A function that byte encodes the DB and saves it to disk
 * @param path a string containing the absolute path of the store.bindb file
 */
// TODO: make function modifiable

void Collection::aow(fs::path path, std::vector<widx> doc)
{
    bitWriter.clear();
    // create file to store all docs
    for (std::pair<std::string, std::vector<widx>> doc: storage) {
        
        std::string name = doc.first;
        fs::path filePath = collectionPath / "files" / (name + ".fyle");

        // output number of words in doc
        bitWriter.write(doc.second.size(), 32);
        
        // output nbits in case nbits changes
        bitWriter.write(nbits, 32);
        
        // output nbits for each word
        for (widx idx: doc.second) {
            assert(idx.size() <= nbits);
            bitWriter.write(idx.to_ulong(), nbits);
        }
        bitWriter.saveToFile(filePath.string(), true);
    }
    bitWriter.clear();
    
}

void Collection::loadWordIndex()
{
    fs::path widx_path = collectionPath / "word.idx";

    bitReader.clear();
    // read uncompressed
    bitReader.read(widx_path.string(), false);
    
    size_t size = bitReader.getNextBits(32).to_ulong();
    
    std::vector<std::string> words;

    while (!bitReader.eof()) {
        size_t len = (size_t)bitReader.getNextBits(5).to_ulong();
        std::string word = bitReader.getNextString(len);
        // add to in memory word index
        words.push_back(word);

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


void Collection::load(std::string name)
{

    fs::path filePath = collectionPath / "files" / (name + ".fyle");
    size_t size = bitReader.getNextBits(32).to_ulong();
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
}


void Collection::kick()
{
    // assert no outstanding writes
    storage.clear();
}


size_t Collection::size()
{
    return sizeof(storage);
}

bool Collection::add(std::string name, std::vector<std::string> doc)
{
    fs::path path = collectionPath / (name + ".fyle");
    aow(path, doc);
    // get new words
    // add new words
    std::vector<std::string> new_words = find_new_words(doc);
    aow_words(new_words);
    storage[name] = serialize(doc);
}


std::vector<std::string> Collection::find_new_words(std::vector<std::string> doc)
{
    std::vector<std::string> new_words;
    for (std::string word: doc) {
        if(word2idx.count(word) == 0) {
            new_words.push_back(word);
        }
    }
    return new_words;
}

void Collection::aow_words(std::vector<std::string> new_words)
{
    if (new_words.size() == 0) {
        return;
    }
    bitWriter.clear();
    // ensure aow_words is called prior to updating idx2word with new words
    fs::path widx_path = collectionPath / "word.idx";
    int size = (int)idx2word.size() + (int)new_words.size();
    
    // output nbits
    fstream fout(widx_path.string(), fstream::in | fstream::out | fstream::binary);
    fout.seekp(0);
    fout.write((char*)&size, 4);
    fout.close();
    
    // [len|chars]
    for (std::string word: new_words) {
        // output len in bytes
        // NOTE: current length of word is 32
        bitWriter.write(word.size() * bitWriter.encoder->charSize(), 5);
        bitWriter.write(word);
    }
    bitWriter.appendToFile(widx_path.string(), false);
    bitWriter.clear();
}
