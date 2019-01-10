#include <wallet/bitcoin/UTXOSourceInMemory.hpp>
#include <wallet/NetworkTypes.hpp>
#include <wallet/Keychain.hpp>

namespace ledger {
    namespace core {
    namespace bitcoin {
        UTXOSourceInMemory::UTXOSourceInMemory(
            std::shared_ptr<BlocksDB> blockDB,
            std::shared_ptr<KeychainRegistry> keychainRegistry
        ): UTXOSourceInMemory(blockDB, keychainRegistry, 0) {
        }

        UTXOSourceInMemory::UTXOSourceInMemory(
            std::shared_ptr<BlocksDB> blockDB,
            std::shared_ptr<KeychainRegistry> keychainRegistry,
            uint32_t lowestHeight
        ): _keychainRegistry(keychainRegistry), _lowestHeight(lowestHeight),
           _lastHeight(std::max(1u, lowestHeight) - 1), _blockDB(blockDB) {
        }

        Future<UTXOSourceList> UTXOSourceInMemory::getUTXOs(std::shared_ptr<api::ExecutionContext> ctx) {
            auto self = shared_from_this();

            // get the last block height
            return _blockDB->getLastBlockHeight().flatMap<UTXOSourceList>(ctx, [=](const Option<uint32_t>& lastBlockHeight) {
                // compute the list of blocks we need to retreive
                if (lastBlockHeight) {
                    auto currentHeight = lastBlockHeight.getValue();

                    if (currentHeight > self->_lastHeight) {
                        // there are blocks we don’t know about
                        return self->_blockDB->getBlocks(0, currentHeight).map<UTXOSourceList>(ctx, [=](const std::vector<BitcoinLikeNetwork::FilledBlock>& blocks) {
                            auto spent = std::set<UTXOKey>();

                            for (auto block : blocks) {
                                for (auto tr : block.transactions) {
                                    // treat inputs
                                    for (auto input : tr.inputs) {
                                        if (input.previousTxHash.hasValue()) {
                                            auto hashTX = *input.previousTxHash;
                                            auto index = *input.previousTxOutputIndex;
                                            auto key = std::make_pair(hashTX, index);
                                            auto it = self->_cache.find(key);

                                            if (it != std::end(self->_cache)) {
                                                // the key already exists so we need to spend that UTXO;
                                                // which means removing it from our map, as simple as it
                                                // gets
                                                self->_cache.erase(it);
                                            } else {
                                                // unused key, store it
                                                spent.insert(std::move(key));
                                            }
                                        }
                                    }

                                    // treat outputs
                                    for (auto output : tr.outputs) {
                                        if (output.address.hasValue()) {
                                            // check whether this is about one of our public keys
                                            if (_keychainRegistry->containsAddress(output.address.getValue())) {
                                                auto hashTX = output.transactionHash;
                                                auto index = output.index;
                                                auto key = std::make_pair(hashTX, index);
                                                auto value = UTXOValue(output.value, *output.address);

                                                // in theory, we don’t have to check whether this UTXO
                                                // already exists in our map because otherwise it would
                                                // mean it’s acquired twice, which is not possible
                                                self->_cache.insert(std::make_pair(key, value));
                                            }
                                        }
                                    }
                                }
                            }

                            self->_lastHeight = currentHeight;

                            // copy the UTXO map and move it in our list
                            auto utxos = self->_cache;

                            // create the resulting UTXO source list
                            auto sourceList = UTXOSourceList(std::move(utxos), std::move(spent), currentHeight);
                            return sourceList;
                        });
                    } else {
                        // copy the UTXO map and move it in our list
                        auto utxos = self->_cache;

                        // create the resulting UTXO source list
                        auto sourceList = UTXOSourceList(std::move(utxos), {}, currentHeight);
                        return Future<UTXOSourceList>::successful(sourceList);
                    }
                } else {
                    // no data, just return nothing
                    return Future<UTXOSourceList>::successful(UTXOSourceList({}, {}, 0));
                }
            });
        }

        void UTXOSourceInMemory::invalidate() {
            // remove all entries in the cache and reset the last known block height
            _cache.clear();
            _lastHeight = _lowestHeight;
        }

        uint32_t UTXOSourceInMemory::currentSynchronizedHeight() {
            return _lastHeight;
        }
    }
    }
}
