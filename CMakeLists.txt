cmake_minimum_required(VERSION 3.2)
project(TextDB)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

set(SOURCE_FILES
    build/CMakeFiles/3.0.2/CompilerIdC/CMakeCCompilerId.c
    build/CMakeFiles/3.0.2/CompilerIdCXX/CMakeCXXCompilerId.cpp
    cmake/CMakeFiles/3.0.2/CompilerIdC/CMakeCCompilerId.c
    cmake/CMakeFiles/3.0.2/CompilerIdCXX/CMakeCXXCompilerId.cpp
    dashboard/node_modules/sequest/node_modules/ssh2/util/pagent.c
    dashboard/node_modules/simple-ssh/node_modules/ssh2/util/pagent.c
    dashboard/node_modules/ssh2/util/pagent.c
    snappy/config.h
    snappy/snappy-c.cc
    snappy/snappy-c.h
    snappy/snappy-internal.h
    snappy/snappy-sinksource.cc
    snappy/snappy-sinksource.h
    snappy/snappy-stubs-internal.cc
    snappy/snappy-stubs-internal.h
    snappy/snappy-stubs-public.h
    snappy/snappy-test.cc
    snappy/snappy-test.h
    snappy/snappy.cc
    snappy/snappy.h
    snappy/snappy_unittest.cc
    TextDB/bitreader.cpp
    TextDB/bitreader.h
    TextDB/bitwriter.cpp
    TextDB/bitwriter.h
    TextDB/cache.cpp
    TextDB/cache.h
    TextDB/collection.cpp
    TextDB/collection.h
    TextDB/connection.cpp
    TextDB/connection.hpp
    TextDB/connection_manager.cpp
    TextDB/connection_manager.hpp
    TextDB/db.cpp
    TextDB/db.h
    TextDB/Doc.cpp
    TextDB/Doc.h
    TextDB/docgraph.cpp
    TextDB/docgraph.h
    TextDB/encoder.cpp
    TextDB/encoder.h
    TextDB/entry.cpp
    TextDB/entry.h
    TextDB/header.hpp
    TextDB/LRU.cpp
    TextDB/LRU.h
    TextDB/main.cpp
    TextDB/mime_types.cpp
    TextDB/mime_types.hpp
    TextDB/operation.cpp
    TextDB/operation.h
    TextDB/options.h
    TextDB/raft.cpp
    TextDB/raft.h
    TextDB/reply.cpp
    TextDB/reply.hpp
    TextDB/request.hpp
    TextDB/request_handler.cpp
    TextDB/request_handler.hpp
    TextDB/request_parser.cpp
    TextDB/request_parser.hpp
    TextDB/sentiment.cpp
    TextDB/sentiment.h
    TextDB/server.cpp
    TextDB/server.hpp
    TextDB/similarity.h
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/bson/ext/bson.cc
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/bson/ext/bson.h
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/bson/node_modules/nan/nan.h
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/win32/wrappers/security_buffer.cc
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/win32/wrappers/security_buffer.h
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/win32/wrappers/security_buffer_descriptor.cc
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/win32/wrappers/security_buffer_descriptor.h
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/win32/wrappers/security_context.cc
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/win32/wrappers/security_context.h
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/win32/wrappers/security_credentials.cc
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/win32/wrappers/security_credentials.h
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/win32/base64.c
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/win32/base64.h
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/win32/kerberos.cc
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/win32/kerberos.h
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/win32/kerberos_sspi.c
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/win32/kerberos_sspi.h
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/win32/worker.cc
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/win32/worker.h
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/base64.c
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/base64.h
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/kerberos.cc
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/kerberos.h
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/kerberos_context.cc
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/kerberos_context.h
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/kerberosgss.c
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/kerberosgss.h
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/worker.cc
    webserver/node_modules/mongojs/node_modules/mongodb/node_modules/kerberos/lib/worker.h
    webserver/node_modules/request/node_modules/node-uuid/benchmark/benchmark-native.c)

add_executable(TextDB ${SOURCE_FILES})