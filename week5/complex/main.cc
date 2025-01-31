#include <stdio.h>
#include "gtest/gtest.h"

GTEST_API_ int main(int argc, char **argv)
{
  testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}

Complex a, b(1.0, 2.0), c(2.0);
Complex c = (3.0, 4.0); //this compiles