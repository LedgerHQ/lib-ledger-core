#include <gtest/gtest.h>
#include "../integration/IntegrationEnvironment.h"

int main(int argc, char **argv) {
	IntegrationEnvironment::initInstance(argc, argv);
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
