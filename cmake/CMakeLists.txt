# CMakeLists.txt
cmake_minimum_required(VERSION 3.0)
project (TextDB)

list(APPEND CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/modules/")

set(SRC_PATH "${CMAKE_SOURCE_DIR}/../TextDB")

file(GLOB SOURCES
    "${SRC_PATH}/*.h"
    "${SRC_PATH}/*.cpp"
)

# CPP 11
include(CheckCXXCompilerFlag)
CHECK_CXX_COMPILER_FLAG("-std=c++11" COMPILER_SUPPORTS_CXX11)
if(COMPILER_SUPPORTS_CXX11)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")
else()
        message(STATUS "The compiler ${CMAKE_CXX_COMPILER} has no C++11 support. Please use a different C++ compiler.")
endif()


# Find boost
set(Boost_USE_STATIC_LIBS OFF) 
set(Boost_USE_MULTITHREADED ON)  
set(Boost_USE_STATIC_RUNTIME OFF) 

# system, filesystem, thread, program_options, regex, serialization
find_package(Boost 1.45.0 COMPONENTS system-mt filesystem-mt program_options-mt thread-mt regex-mt serialization-mt log-mt) 


# Find curlpp, curl
find_package(CURL REQUIRED)
if (NOT CURL_FOUND)
    message("Curl not found")
endif()

find_package(CURLpp REQUIRED)
if (NOT CURLPP_FOUND)
    message("CurlPP not found")
endif()
# Find snappy
find_package(snappy REQUIRED)


if(Boost_FOUND)
    include_directories(${Boost_INCLUDE_DIRS} ${CURLPP_INCLUDE_DIRS} ${CURL_INCLUDE_DIRS}) 
    add_executable(TextDB ${SOURCES}) 
    target_link_libraries(TextDB -L/usr/local/lib ${Boost_LIBRARIES} ${CURLPP_LIBRARIES} ${snappy_LIBRARIES} ${CURL_LIBRARIES})
endif()


#-L/usr/local/lib -lboost_system-mt -lboost_filesystem-mt -lboost_thread-mt -lsnappy -lboost_program_options -lboost_regex-mt -lboost_serialization -lcurlpp -lcurl
