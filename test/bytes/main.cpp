//
// Created by PIERRE POLLASTRI on 14/09/2016.
//

#include <gtest/gtest.h>

TEST(FactorialTest, HandlesZeroInput) {
    printf("Hello world!\n");
    EXPECT_EQ(1, 1);
}


int main(int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}