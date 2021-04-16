#include "gtest/gtest.h"
#include <regex>

namespace HTTP2RequestPatterns {
  using std::regex;

  regex requestLineGET("(GET)\\s/[a-zA-Z0-9.-/]*\\s(HTTP/1.1)");
  regex requestLineHEAD("(HEAD)\\s/[a-zA-Z0-9.-/]*\\s(HTTP/1.1)");
  regex headerConnectionDefault("(Connection):(\\s)*");
  regex headerConnectionClose("(Connection):(\\s)*(close)(\\s)*");
  regex contentLength("(Content-Length):(\\s)*(0)(\\s)*");
}

TEST (SquareTest /*test suite name*/, PosZeroNeg /*test name*/) {
  assert(std::regex_match("Connection:   close  ", HTTP2RequestPatterns::headerConnectionClose));
}

/*

 class myTestFixture: public ::testing::test {
    public:
         myTestFixture( ) {
             // initialization;
             // can also be done in SetUp()
         }

    void SetUp( ) {
         // initialization or some code to run before each test
    }

    void TearDown( ) {
         // code to run after each test;
         // can be used instead of a destructor,
         // but exceptions can be handled in this function only
     }

    ~myTestFixture( )  {
         //resources cleanup, no exceptions allowed
    }

     // shared user data
};

TEST_F( myTestFixture, TestName) {}

*/