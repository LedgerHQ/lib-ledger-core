#pragma once

#include <net/HttpClient.hpp>
#include <async/Future.hpp>
#include <collections/Bytes.hpp>
#include <wallet/TransactionBroadcaster.hpp>
#include <vector>
#include <utility>
#include <utils/Option.hpp>

namespace ledger {
    namespace core {
        namespace common {
            template<typename NetworkType>
            class ExplorerTransactionBroadcaster : public TransactionBroadcaster<NetworkType> {
            public:
                ExplorerTransactionBroadcaster(
                    const std::shared_ptr<api::ExecutionContext>& context,
                    const std::shared_ptr<core::HttpClient>& http,
                    const api::BitcoinLikeNetworkParameters& parameters,
                    const std::shared_ptr<api::DynamicObject>& configuration
                ) {
                    throw Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "implement me");
                };

                void broadcastRawTransaction(const std::vector<uint8_t> & transaction) override {
                    throw Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "implement me");
                }

                virtual void broadcastRawTransaction(const std::vector<uint8_t> & transaction, const std::shared_ptr<api::StringCallback> & callback) override {
                    throw Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "implement me");
                }

                void broadcastTransaction(const std::shared_ptr<TransactionToBroadcast> & transaction, const std::shared_ptr<api::StringCallback> & callback) override {
                    throw Exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "implement me");
                }
            };
        }
    };
};