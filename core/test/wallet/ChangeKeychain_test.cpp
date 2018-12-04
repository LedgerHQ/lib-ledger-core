#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>
#include <vector>
#include <wallet/bitcoin/keychains/ChangeKeychain.hpp>

#include "Helpers.hpp"
#include "Mocks.hpp"

using namespace testing;

namespace ledger {
    namespace core {
        namespace tests {

            class ChangeKeychainTest : public ::testing::Test {
            public:
               
            };

            TEST_F(ChangeKeychainTest, ConstuctorAskDB) {
                bitcoin::ChangeKeychain chain();
            }
        }
    }
}