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

        void UTXOCacheMemoryMap::getUTXOs(
            std::shared_ptr<api::ExecutionContext> ctx,
            const std::vector<std::string>& addresses,
            std::function<void (std::vector<std::pair<UTXOCache::Key, UTXOCache::Value>>)> onUTXOs
        ) {
            auto self = shared_from_this();

            // get the last block height
            auto future = _blockDB->getLastBlockHeader().foreach(ctx, [=](Option<BitcoinLikeNetwork::Block>& lastBlock) {
                // compute the list of blocks we need to retreive
                if (lastBlock) {
                    auto currentHeight = lastBlock->height;

                    if (currentHeight > self->getLastHeight()) {
                        // there are blocks we don’t know about
                        self->_blockDB->getBlocks(0, lastBlock->height).foreach(ctx, [=](std::vector<BitcoinLikeNetwork::FilledBlock>& blocks) {
                            for (auto block : blocks) {
                                for (auto tr : block.transactions) {
                                    // treat inputs
                                    for (auto input : tr.inputs) {
                                        if (input.previousTxHash.hasValue()) {
                                            auto hashTX = *input.previousTxHash;
                                            auto index = *input.previousTxOutputIndex;
                                            auto key = UTXOCache::Key(hashTX, index);
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
                                            auto address = std::find(std::begin(addresses), std::end(addresses), *output.address);
                                            if (address != std::end(addresses)) {
                                                auto hashTX = output.transactionHash;
                                                auto index = output.index;
                                                auto key = UTXOCache::Key(hashTX, index);
                                                auto value = UTXOCache::Value(output.value, *output.address);

                                                // in theory, we don’t have to check whether this UTXO
                                                // already exists in our map because otherwise it would
                                                // mean it’s acquired twice, which is not possible
                                                self->_cache.insert(std::make_pair(key, value));
                                            }
                                        }
                                    }
                                }
                            }

                            self->updateLastHeight(currentHeight);
                        });
                    }
                    
                    // copy the UTXO in a vector
                    auto utxos = std::vector<std::pair<UTXOCache::Key, UTXOCache::Value>>();
                    std::copy(std::begin(self->_cache), std::end(self->_cache), std::begin(utxos));
                    onUTXOs(utxos);
                }
            });
        }

        void UTXOCacheMemoryMap::invalidate() {
            // remove all entries in the cache and reset the last known block height
            _cache.clear();
            _lastHeight = _lowestHeight;
        }
    }
}
