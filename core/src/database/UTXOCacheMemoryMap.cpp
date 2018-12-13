#include <database/UTXOCacheMemoryMap.hpp>
#include <wallet/NetworkTypes.hpp>

namespace ledger {
    namespace core {
        UTXOCacheMemoryMap::UTXOCacheMemoryMap(std::shared_ptr<ReadOnlyBlockchainDatabase<BitcoinLikeNetwork>> blockDB)
            : UTXOCacheMemoryMap(blockDB, 0) {
        }

        UTXOCacheMemoryMap::UTXOCacheMemoryMap(
            std::shared_ptr<ReadOnlyBlockchainDatabase<BitcoinLikeNetwork>> blockDB,
            uint32_t lowestHeight)
            : UTXOCache(lowestHeight), _blockDB(blockDB) {
        }

        void UTXOCacheMemory::getUTXOs(
            std::shared_ptr<api::ExecutionContext> ctx,
            const std::vector<std::string>& addresses,
            std::function<void (UTXOIterable)> onUTXOs
        ) {
            auto self = shared_from_this();

            // get the last block height
            auto future = _db->getLastBlockHeader().flatMap(ctx, [=](Option<Block>& block) {
                // compute the list of blocks we need to retreive
                auto currentHeight = block.heightTo;

                if (currentHeight > self->getLastHeight()) {
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

                        self->updateLastHeight(currentHeight);
                    });
                }
                
                onUTXOs(self->_cache);
            });

            return future;
        }

        void UTXOCacheMemoryMap::invalidate() {
            // remove all entries in the cache and reset the last known block height
            _cache.clear();
            _lastHeight = _lowestHeight;
        }
    }
}
