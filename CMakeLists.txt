cmake_minimum_required(VERSION 3.16)
project(zadanie1SIK)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_C_FLAGS "-g -Wall -Wextra")
set(CMAKE_CXX_FLAGS "-g -Wall -Wextra")
set(DEFAULT_BUILD_TYPE "RELEASE")

#Add the google test subdirectory
add_subdirectory(/Users/mac/Desktop/projectsInC++/zadanie1SIK/googletest)

#include googletest/include dir
include_directories(/Users/mac/Desktop/projectsInC++/zadanie1SIK/googletest/include)

#include the googlemock/include dir
include_directories(/Users/mac/Desktop/projectsInC++/zadanie1SIK/googletest/googlemock/include)

# Wskazujemy pliki źródłowe.
set(SOURCE_FILES
        src/parsing_functionalities.h
        src/parsing_functionalities.cpp
        src/file_system_functionalities.h
        src/file_system_functionalities.cpp
        src/server.h
        src/server.cpp
        src/err.h
        src/err.c
        src/request_answer.h
        src/request_answer.cpp
        src/main.cpp
        )

set(SOURCE_FILES2
        src/err.h
        src/err.c
        src/echo-client.cpp
        )

# Wskazujemy plik wykonywalny.
add_executable(serwer ${SOURCE_FILES})

add_executable(echo-client ${SOURCE_FILES2})

#Link with GoogleTest
target_link_libraries(serwer gtest gtest_main)

#Link with GoogleMock
target_link_libraries(serwer gmock gmock_main)