#include "../integration/IntegrationEnvironment.h"

#include <gtest/gtest.h>

int main(int argc, char **argv) {
    IntegrationEnvironment::initInstance(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
