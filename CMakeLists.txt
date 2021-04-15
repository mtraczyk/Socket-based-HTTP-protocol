cmake_minimum_required(VERSION 3.16)
project(jnp1FifthTask)

set(CMAKE_CXX_STANDARD 17)

#Add the google test subdirectory
add_subdirectory(/Users/mac/Desktop/projectsInC++/zadanie1SIK/googletest)

#include googletest/include dir
include_directories(/Users/mac/Desktop/projectsInC++/zadanie1SIK/googletest/include)

#include the googlemock/include dir
include_directories(/Users/mac/Desktop/projectsInC++/zadanie1SIK/googletest/googlemock/include)

# Wskazujemy pliki źródłowe.
set(SOURCE_FILES
        parsing_functionalities.h
        parsing_functionalities.cpp
        file_system_functionalities.h
        file_system_functionalities.cpp
        server.h
        server.cpp
        main.cpp
        )

# Wskazujemy plik wykonywalny.
add_executable(zadanie1SIK ${SOURCE_FILES})

#Link with GoogleTest
target_link_libraries(zadanie1SIK gtest gtest_main)

#Link with GoogleMock
target_link_libraries(zadanie1SIK gmock gmock_main)