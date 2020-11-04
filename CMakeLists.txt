# specify the min cmake required
cmake_minimum_required(VERSION 3.7.2)

message(STATUS "Using compiler ${CMAKE_CXX_COMPILER_ID}")

# project name
project("CppLights")

set (use11 "ON")
if(use11)
    set (CMAKE_CXX_STANDARD 11)
    message(STATUS "using Cpp 11 flag")
else(use11)
    message(STATUS "NOT using Cpp 11 flag")
endif(use11)

file(GLOB SOURCES "src/*.cpp")

# set the output folders
set(CMAKE_BINARY_DIR ${CMAKE_SOURCE_DIR}/bin)
set(EXECUTABLE_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set(LIBRARY_OUTPUT_PATH ${CMAKE_BINARY_DIR})
set (CURL_LIBRARY "-lcurl")
find_package(CURL REQUIRED)
find_package(jsoncpp REQUIRED)

add_executable("CppLights" ${SOURCES}) 

get_target_property(JSON_INC_PATH jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)
include_directories(${CURL_INCLUDE_DIR} ${JSON_INC_PATH})
target_link_libraries("CppLights" ${CURL_LIBRARIES} -ljsoncpp)
