#include <bytes/serialization.hpp>
#include <wallet/bitcoin/PersistentUTXOSource.hpp>
#include <database/BlockchainDB.hpp>

namespace ledger {
    namespace core {
        namespace bitcoin {
            PersistentUTXOSource::PersistentUTXOSource(
                const std::shared_ptr<db::BlockchainDB>& blockchainDB,
                const std::shared_ptr<UTXOSourceInMemory>& inMemorySource
            ) : _db(blockchainDB), _inMemorySource(inMemorySource) {
            }

            Future<UTXOSourceList> PersistentUTXOSource::getUTXOs(std::shared_ptr<api::ExecutionContext> ctx) {
                auto self = shared_from_this();

                return _inMemorySource->getUTXOs(ctx).template flatMap<UTXOSourceList>(ctx, [=](const UTXOSourceList& sourceList) {
                    return _db->GetLastBlock().template map<UTXOSourceList>(ctx, [=](const Option<std::pair<uint32_t, db::ReadOnlyBlockchainDB::RawBlock>>& lastBlock) {
                        if (lastBlock.hasValue()) {
                            auto dbHeight = std::get<0>(*lastBlock);
                            auto inMemHeight = sourceList.height;

                            if (dbHeight < inMemHeight) {
                                // the in-memory source has advanced; we must persist the new state
                                auto block = std::vector<uint8_t>();
                                serialization::save(sourceList, block);
                                self->_db->AddBlock(inMemHeight, block);
                            }
                        } else {
                            // no block in the DB so far; persist the source list
                            auto block = std::vector<uint8_t>();
                            serialization::save(sourceList, block);
                            self->_db->AddBlock(inMemHeight, block);
                        }

                        return sourceList;
                    });
                });
            }
        }
    }
}
