#pragma once

#include <wallet/common/PersistentBlockchainDatabase.hpp>
#include <async/Future.hpp>
#include <vector>
#include <wallet/NetworkTypes.hpp>
#include <wallet/BlockchainDatabase.hpp>
#include <database/BlockchainDB.hpp>
#include <cereal/archives/binary.hpp>

namespace ledger {
    namespace core {
        namespace common {
            using RawBlock = db::BlockchainDB::RawBlock;

            template<typename NetworkType>
            std::vector<uint8_t> serialize(const typename NetworkType::FilledBlock& filledBlock) {
                std::ostringstream  ss;
                {
                    cereal::BinaryOutputArchive oarchive(ss);
                    oarchive(filledBlock);
                }
                auto data = ss.str();
                return std::vector<uint8_t>(data.begin(), data.end());
            }

            template <typename NetworkType>
            void deserialize(const std::vector<uint8_t>& buf, typename NetworkType::FilledBlock& filledBlock) {
                std::istringstream  ss(std::string(buf.begin(), buf.end()));
                {
                    cereal::BinaryInputArchive iarchive(ss);
                    iarchive(filledBlock);
                }
            }

            template <typename NetworkType>
            PersistentBlockchainDatabase<NetworkType>::PersistentBlockchainDatabase(
                const std::shared_ptr<api::ExecutionContext>& context,
                const std::shared_ptr<db::BlockchainDB>& persistentDB)
            : _context(context)
            , _persistentDB(persistentDB) {
            };

            template <typename NetworkType>
            Future<std::vector<typename NetworkType::FilledBlock>> PersistentBlockchainDatabase<NetworkType>::getBlocks(uint32_t heightFrom, uint32_t heightTo) {
                return _persistentDB->GetBlocks(heightFrom, heightTo)
                    .map<std::vector<FilledBlock>>(_context, [](const std::vector<RawBlock>& blocks) {
                    std::vector<FilledBlock> res;
                    for (auto& rawBlock : blocks) {
                        FilledBlock fb;
                        deserialize<NetworkType>(rawBlock, fb);
                        res.push_back(fb);
                    }
                    return res;
                });
            }

            template <typename NetworkType>
            Future<Option<typename NetworkType::FilledBlock>> PersistentBlockchainDatabase<NetworkType>::getBlock(uint32_t height) {
                return _persistentDB->GetBlock(height)
                    .map<Option<FilledBlock>>(_context, [](const Option<RawBlock>& rawBlock) {
                    return rawBlock.map<FilledBlock>([](const RawBlock& rawBlock) {
                        FilledBlock fb;
                        deserialize<NetworkType>(rawBlock, fb);
                        return fb;});
                });
            }

            // Return the last block header or genesis block
            template <typename NetworkType>
            Future<Option<typename NetworkType::Block>> PersistentBlockchainDatabase<NetworkType>::getLastBlockHeader() {
                return _persistentDB->GetLastBlock()
                    .map<Option<Block>>(_context, [](const Option<RawBlock>& rawBlock) {
                    return rawBlock.map<Block>([](const RawBlock& block) {
                        FilledBlock filledBlock;
                        deserialize<NetworkType>(block, filledBlock);
                        return filledBlock.header;
                    });
                });
            }

            template <typename NetworkType>
            void PersistentBlockchainDatabase<NetworkType>::addBlock(const FilledBlock& block) {
                RawBlock rawBlock = serialize<NetworkType>(block);
                _persistentDB->AddBlock(block.header.height, rawBlock);
            }
            // Remove all blocks with height in [heightFrom, heightTo)
            template <typename NetworkType>
            void PersistentBlockchainDatabase<NetworkType>::removeBlocks(uint32_t heightFrom, uint32_t heightTo) {
                _persistentDB->RemoveBlocks(heightFrom, heightTo);
            }
            // Remove all blocks with height < heightTo
            template <typename NetworkType>
            void PersistentBlockchainDatabase<NetworkType>::removeBlocksUpTo(uint32_t heightTo) {
                _persistentDB->RemoveBlocksUpTo(heightTo);
            }
            template <typename NetworkType>
            void PersistentBlockchainDatabase<NetworkType>::CleanAll() {
                _persistentDB->CleanAll();
            }
        }
    };
}

template class ledger::core::common::PersistentBlockchainDatabase<ledger::core::BitcoinLikeNetwork>;
template class std::shared_ptr<ledger::core::common::PersistentBlockchainDatabase<ledger::core::BitcoinLikeNetwork>>;