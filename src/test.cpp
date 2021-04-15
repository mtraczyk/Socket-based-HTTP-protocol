#include "function_maxima.h"
#include "gtest/gtest.h"

TEST (SquareTest /*test suite name*/, PosZeroNeg /*test name*/) {
  FunctionMaxima<int, int> a = FunctionMaxima<int, int>();
  FunctionMaxima<int, int> b = FunctionMaxima<int, int>(a);
  FunctionMaxima<int, char> c = FunctionMaxima<int, char>();

  b = a;

  //std::cout << "!!!!" << std::endl;

  b.set_value(3, 5);
  std::cout << b.value_at(3) << std::endl;

  b.set_value(4, 8);
  b.set_value(7, 6);
  b.set_value(1, 2);

  for (const auto &p : b) {
    std::cout << p.arg() << " -> " << p.value() << std::endl;
  }

  std::cout << std::endl;

  for (auto it = b.mx_begin(); it != b.mx_end(); it++) {
    std::cout << (*it).arg() << " -> " << (*it).value() << std::endl;
  }

  b.erase(3);

  std::cout << std::endl;

  b.set_value(3, 4);
  b.set_value(1, 9);
  b.set_value(1, 9);
  b.set_value(1, 8);
  b.set_value(3, 8);
  b.set_value(11, 11);
  b.set_value(10, 10);
  b.set_value(11, 9);

  b.erase(10);

  std::cout << b.value_at(3) << std::endl;

  for (const auto &p : b) {
    std::cout << p.arg() << " -> " << p.value() << std::endl;
  }

  std::cout << std::endl;

  for (auto it = b.mx_begin(); it != b.mx_end(); it++) {
    std::cout << (*it).arg() << " -> " << (*it).value() << std::endl;
  }

  auto aux = b.find(3);

  std::cout << std::endl;
  for (auto it = aux; it != b.end(); it++) {
    std::cout << (*it).arg() << " -> " << (*it).value() << std::endl;
  }

  auto d(b);

  std::cout << std::endl;
  for (const auto &p : d) {
    std::cout << p.arg() << " -> " << p.value() << std::endl;
  }

  //  a.value_at(3);

  //  EXPECT_EQ (9.0, (3.0 * 2.0)); // fail, test continues
  //  ASSERT_EQ (0.0, (0.0));     // success
  //  ASSERT_EQ (9, (3) * (-3.0));  // fail, test interrupts
//    ASSERT_EQ (-9, (-3) * (-3.0));// not executed due to the previous assert
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