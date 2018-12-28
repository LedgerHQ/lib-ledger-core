#include <CommonFixtureFunctions.hpp>

using namespace testing;

namespace ledger {
    namespace core {
        namespace tests {
            typedef BlockchainDatabase<BitcoinLikeNetwork::FilledBlock> BlocksDatabase;

            void CommonFixtureFunctions::linkMockDbToFake(std::shared_ptr<::testing::NiceMock<BlocksDBMock>>& mock, BlocksDatabase& fake) {
                ON_CALL(*mock, addBlock(_, _)).WillByDefault(Invoke(&fake, &BlocksDatabase::addBlock));
                ON_CALL(*mock, removeBlocks(_, _)).WillByDefault(Invoke(&fake, &BlocksDatabase::removeBlocks));
                ON_CALL(*mock, removeBlocksUpTo(_)).WillByDefault(Invoke(&fake, &BlocksDatabase::removeBlocksUpTo));
                ON_CALL(*mock, CleanAll()).WillByDefault(Invoke(&fake, &BlocksDatabase::CleanAll));
                ON_CALL(*mock, getBlocks(_, _)).WillByDefault(Invoke(&fake, &BlocksDatabase::getBlocks));
                ON_CALL(*mock, getBlock(_)).WillByDefault(Invoke(&fake, &BlocksDatabase::getBlock));
                ON_CALL(*mock, getLastBlock()).WillByDefault(Invoke(&fake, &BlocksDatabase::getLastBlock));
            }

        }
    }
}
