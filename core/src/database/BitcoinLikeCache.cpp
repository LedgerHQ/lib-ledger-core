#include <database/BitcoinLikeCache.hpp>
#include <wallet/NetworkTypes.hpp>

namespace ledger {
    namespace core {
        BitcoinLikeCache::BitcoinLikeCache(std::shared_ptr<ReadOnlyBlockchainDatabase<BitcoinLikeNetwork>> blockDB):
            _lowerHeight(0, _lastHeight(0), _blockDB(blockDB) {
        }

        BitcoinLikeCache::BitcoinLikeCache(
            std::shared_ptr<ReadOnlyBlockchainDatabase<BitcoinLikeNetwork>> blockDB,
            uint32_t startHeight):
            _lowerHeight(std::max(1, startHeight) - 1), _lastHeight(_lowerHeight), _blockDB(blockDB) {
        }

        void
        BitcoinLikeCache::getUTXOs(
            std::shared_ptr<api::ExcutionContext> ctx,
            const std::vector<std::string>& addresses,
            std::function<void (const std::map<UTXOKey, BigInt>&)> onUTXOs
        ) {
            auto self = shared_from_this();

            // get the last block height
            auto future = _db->getLastBlockHeader().flatMap(ctx, [=](Option<Block>& block) {
                // compute the list of blocks we need to retreive
                auto currentHeight = block.heightTo;

                if (currentHeight > self->_lastHeight) {
                    // there are blocks we don’t know about
                    return self->_db->getBlocks(0, block.heightTo).flatMap(ctx, [=](std::vector<FilledBlock>& blocks) {
                        for (auto block : blocks) {
                            for (auto tr : block.transactions) {
                                // treat inputs
                                for (auto input : tr.inputs) {
                                    if (input.previousTxHash.hasValue()) {
                                        auto hashTX = *input.previousTxHash;
                                        auto index = *input.previousTxOutputIndex;
                                        auto key = UTXOKey(hashTX, index);
                                        auto it = self->_cache.find(key);

                                        if (it != std::end(self->_cache)) {
                                            // the key already exists so we need to spend that UXTO;
                                            // which means removing it from our map, as simple as it
                                            // gets
                                            self->_cache.erase(it);
                                        }
                                    }
                                }

                                // treat outputs
                                for (auto output : tr.outputs) {
                                    if (output.address.hasValue()) {
                                        // check whether this is about one of our public keys
                                        if (std::find(std::cbegin(addresses), std::cend(addresses), *output.address) != std::cend(addresses)) {
                                            auto hashTX = output.transactionHash;
                                            auto index = output.index;
                                            auto key = UTXOKey(hashTX, index);

                                            // in theory, we don’t have to check whether this UTXO
                                            // already exists in our map because otherwise it would
                                            // mean it’s acquired twice, which is not possible
                                            self->_cache.insert(std::make_pair(key, output.value));
                                        }
                                    }
                                }
                            }
                        }

                        // update the height
                        self->_lastHeight = currentHeight;
                    });
                }
                
                onUTXOs(self->_cache);
            });

            return future;
        }

        void BitcoinLikeCache::invalidateUTXO() {
            _cache.clear();
            _lastHeight = _lowerHeight;
        }
    }
}
