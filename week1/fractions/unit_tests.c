#include "fraction.h"
#include "gtest/gtest.h"

namespace {

    TEST(Fractions, BasicsAdd) {
        Fraction a = (Fraction) { 2, 3 },
                 b = (Fraction) { 4, 5 };
        EXPECT_EQ(add(a,b).num,22);
        EXPECT_EQ(add(a,b).den,15);

    }

    TEST(Fractions, BasicsZero) {
        Fraction a = (Fraction) { 2, 3 },
                 b = (Fraction) { -2, 3 };
        EXPECT_EQ(add(a,b).num,0);
        EXPECT_EQ(add(a,b).den,3);

    }

    TEST(Fractions, BasicsMult) {
        Fraction a = (Fraction) { 2, 3 },
                 b = (Fraction) { 4, 5 };

        EXPECT_EQ(multiply(a,b).num,8);
        EXPECT_EQ(multiply(a,b).den,15);
    }

    TEST(Fractions, BasicsOne) {
        Fraction a = (Fraction) { 2, 3 },
                 b = (Fraction) { 3, 2 };

        EXPECT_EQ(multiply(a,b).num,6);
        EXPECT_EQ(multiply(a,b).den,6);
    }

}
