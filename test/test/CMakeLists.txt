cmake_minimum_required(VERSION 3.15)
project(tinyxml2tests)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(tinyxml2tests
        StrPairTest.cpp
        ../../tinyxml2.cpp
        ../../tinyxml2.h
        ../../src/StrPair.cpp
        ../../src/StrPair.h
        ../../src/XMLUtil.cpp
        ../../src/XMLUtil.h
        )

target_link_libraries(
        tinyxml2tests
        gtest
        gtest_main
        )

