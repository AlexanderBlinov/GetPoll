cmake_minimum_required(VERSION 3.2)
project(GetPoll)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -fPIC -O2")

set(SOURCE_FILES Model/Poll.cpp Model/Poll.h Model/Vote.cpp Model/Vote.h Model/PollOption.cpp Model/PollOption.h CassandraDriver/cassandra.h Storage/StorageClient.cpp Storage/StorageClient.h Storage/Query.cpp Storage/Query.h Network/RequestHandler.cpp)

#set(SOURCE_FILES Network/RequestHandler.cpp)

if(COMMAND cmake_policy)
    cmake_policy(SET CMP0003 NEW)
endif(COMMAND cmake_policy)

add_library(GetPoll SHARED ${SOURCE_FILES})
target_link_libraries(GetPoll ${CMAKE_SOURCE_DIR}/CassandraDriver/libcassandra_static.a)
target_link_libraries(GetPoll uv)
target_link_libraries(GetPoll fastcgi-daemon2)
