#pragma once

#include <async/Future.hpp>
#include <vector>
#include <wallet/NetworkTypes.hpp>
#include <api/StringCallback.hpp>

namespace ledger {
    namespace core {
        template<typename NetworkType>
        class TransactionBroadcaster {
        public:
            typedef typename NetworkType::TransactionToBroadcast TransactionToBroadcast;

            virtual ~TransactionBroadcaster() {};

            virtual void broadcastRawTransaction(const std::vector<uint8_t> & transaction, const std::shared_ptr<api::StringCallback> & callback) = 0;
            virtual void broadcastRawTransaction(const std::vector<uint8_t> & transaction) = 0;

            virtual void broadcastTransaction(const std::shared_ptr<TransactionToBroadcast> & transaction, const std::shared_ptr<api::StringCallback> & callback) = 0;
        };
    };
}