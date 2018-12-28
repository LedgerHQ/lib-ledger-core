#pragma once
#include <gtest/gtest.h>
#include <Helpers.hpp>
#include <Mocks.hpp>
#include <memory>
#include <wallet/BlockchainDatabase.hpp>

namespace ledger {
    namespace core {
        namespace tests {
            class CommonFixtureFunctions : public virtual ::testing::Test {
            public:
                void linkMockDbToFake(std::shared_ptr<::testing::NiceMock<BlocksDBMock>>& mock, BlockchainDatabase<BitcoinLikeNetwork::FilledBlock>& fake);
            };
        }
    }
}
