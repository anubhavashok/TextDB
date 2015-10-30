//
//  custom_lambda.h
//  TextDB
//
//  Created by Anubhav on 10/21/15.
//  Copyright (c) 2015 anubhav. All rights reserved.
//

#ifndef __TextDB__custom_lambda__
#define __TextDB__custom_lambda__

#include <iostream>
#include <queue>
#include <map>
#include <boost/serialization/serialization.hpp>
#include <boost/archive/text_oarchive.hpp>
#include <boost/archive/text_iarchive.hpp>
#include <boost/serialization/utility.hpp>
#include <boost/serialization/map.hpp>
#include <boost/serialization/string.hpp>
#include <boost/serialization/access.hpp>
#include <boost/serialization/force_include.hpp>
#include <boost/serialization/void_cast_fwd.hpp>
#include <boost/serialization/vector.hpp>
#include <boost/serialization/priority_queue.hpp>
#include <boost/serialization/shared_ptr.hpp>
#include <sstream>
#include "db.h"

using namespace std;

typedef std::pair<std::shared_ptr<DB>, map<string, string>> InputTuple;
typedef std::function<string(InputTuple real, InputTuple fake)> ComparatorType;


class LambdaObject
{
public:
    priority_queue<InputTuple, vector<InputTuple>, ComparatorType> pq;
    
    LambdaObject()
    {
        auto comp = [](InputTuple real, InputTuple fake)->string{return "Works";};
        pq = priority_queue<InputTuple, vector<InputTuple>, ComparatorType> (comp);
        //auto comp = [](int a, int b)->int{ return a + b; };
        //pq = std::priority_queue<int, std::vector<int>, std::function<int(int, int)> >(comp);
    }
    
    //std::priority_queue<int, std::vector<int>, std::function<int(int, int)> > pq;
    friend class boost::serialization::access;
    template<class Archive>
    void serialize(Archive & ar, const unsigned int version)
    {
        ar & pq;
    }
};

class Extractor: public priority_queue<InputTuple, std::vector<InputTuple>, ComparatorType>
{
public:
    Extractor(const priority_queue<InputTuple, std::vector<InputTuple>, ComparatorType>& pq)
    : priority_queue<InputTuple, std::vector<InputTuple>, ComparatorType>(pq)
    {
        
    }
    ComparatorType getComparator()
    {
        return comp;
    }
    // void(ostream& out, map<string, string>& args)
    static function<void(DB*, ostream&, map<string, string>&)> extractLambda(string s) {
        LambdaObject retrievedLambdaObject;
        std::stringstream ifs(s);
        boost::archive::text_iarchive ia(ifs);
        
        // restore the schedule from the archive
        ia >> retrievedLambdaObject;
        
        Extractor e(retrievedLambdaObject.pq);
        auto f = e.getComparator();
        
        
        function<void(DB*, ostream&, map<string, string>&)> res = [f](DB* db, ostream& out, map<string, string>& args) {
            auto real = make_tuple(shared_ptr<DB>(db), args);
            DB* fakeDB = new DB();
            map<string, string> fakeArgs;
            auto fake = make_tuple(shared_ptr<DB>(fakeDB), fakeArgs);
            out << f(real, fake);
        };
        return res;
    }

};





#endif /* defined(__TextDB__custom_lambda__) */
