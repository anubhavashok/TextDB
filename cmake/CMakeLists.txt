# CMakeLists.txt
cmake_minimum_required(VERSION 2.7)
project (TextDB)
include(ExternalProject)
# set(Boost_DEBUG 1)
list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules/")

set(SRC_PATH "${CMAKE_SOURCE_DIR}/../TextDB")
set(TEST_PATH "${CMAKE_SOURCE_DIR}/../test")

file(GLOB TEST_SOURCES
    "${TEST_PATH}/tests.cpp"
    "${TEST_PATH}/tests.h"
    "${SRC_PATH}/*.h"
    "${SRC_PATH}/*.cpp"
)
list(REMOVE_ITEM TEST_SOURCES ${SRC_PATH}/main.cpp)
file(GLOB SOURCES
    "${SRC_PATH}/*.h"
    "${SRC_PATH}/*.cpp"
)

################################
# C++
################################
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
CHECK_CXX_COMPILER_FLAG("-std=c++0x" COMPILER_SUPPORTS_CXX0X)
if(COMPILER_SUPPORTS_CXX11)
    message("c++11 supported")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11 -g")
elseif(COMPILER_SUPPORTS_CXX0X)
	set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++0x -g")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()


################################
# Boost
################################
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

# system, filesystem, thread, program_options, regex, serialization
find_package(Boost 1.45.0 COMPONENTS system system-mt filesystem-mt program_options-mt thread-mt regex-mt serialization-mt) 


################################
# Curl
################################
find_package(CURL REQUIRED)
if (NOT CURL_FOUND)
    message("Curl not found")
endif()

################################
# CurlPP
################################
find_package(CURLpp REQUIRED)
if (NOT CURLPP_FOUND)
    message("CurlPP not found")
endif()

################################
# Snappy
################################
find_package(snappy REQUIRED)
if (NOT snappy_FOUND)
    message("snappy not found")
endif()

################################
# GTest
################################
include("GTest.cmake")
cxx_test(runUnitTests "${TEST_SOURCES}" -L/usr/local/lib ${Boost_LIBRARIES} ${CURLPP_LIBRARIES} ${snappy_LIBRARIES} ${CURL_LIBRARIES} ${GTEST_BOTH_LIBRARIES} gtest_main)

################################
# Unit Tests
################################


# TODO: Revise to include all
if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS} ${CURLPP_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS} ${GTEST_INCLUDE_DIRS}) 
    message(${SOURCES})
    add_executable(TextDB ${SOURCES}) 
    target_link_libraries(TextDB -L/usr/local/lib ${Boost_LIBRARIES} ${CURLPP_LIBRARIES} ${snappy_LIBRARIES} ${CURL_LIBRARIES} ${GTEST_BOTH_LIBRARIES})
endif()


#-L/usr/local/lib -lboost_system-mt -lboost_filesystem-mt -lboost_thread-mt -lsnappy -lboost_program_options -lboost_regex-mt -lboost_serialization -lcurlpp -lcurl -lgtest
