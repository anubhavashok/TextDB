//
// connection_manager.cpp
// ~~~~~~~~~~~~~~~~~~~~~~
//
// Copyright (c) 2003-2014 Christopher M. Kohlhoff (chris at kohlhoff dot com)
//
// Distributed under the Boost Software License, Version 1.0. (See accompanying
// file LICENSE_1_0.txt or copy at http://www.boost.org/LICENSE_1_0.txt)
//

#include "connection_manager.hpp"
#include <boost/thread.hpp>
#include <iostream>
using namespace std;

namespace http {
namespace server {

connection_manager::connection_manager()
{
}

static void start_connection(connection_ptr& c)
{
    
    cout << boost::this_thread::get_id() << endl;
    c->start();
    //cout << "Exiting thread: " << boost::this_thread::get_id() << endl;
}

void connection_manager::start(connection_ptr c)
{
  connections_.insert(c);
    //cout << "main_thread: " << boost::this_thread::get_id() << endl;
    boost::thread t(start_connection, c);
    t.detach();
    //thread_map[c] = boost::thread(start_connection, c);
  //c->start();
}

void connection_manager::stop(connection_ptr c)
{
    //cout << "in: " << boost::this_thread::get_id() << " joining thread: " << thread_map[c].get_id()<< endl;
    //thread_map[c].join();
    //thread_map.erase(c);
  connections_.erase(c);
  c->stop();
}

void connection_manager::stop_all()
{
    for (auto c: connections_) {
        //thread_map[c].join();
        c->stop();
    }
    // thread_map.clear();
  connections_.clear();
}


} // namespace server
} // namespace http
