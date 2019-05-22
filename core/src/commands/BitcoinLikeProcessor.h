#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "bitcoin_like.pb.h"
#include "async/Future.hpp"
#include "wallet/pool/WalletPool.hpp"


namespace ledger {
    namespace core {
        namespace api {
            class Account;
        };

        class BitcoinLikeCommandProcessor {
        public:
            BitcoinLikeCommandProcessor(const std::shared_ptr<WalletPool>& walletPool);
            Future<std::string> processRequest(const std::string& message);
        protected:
            Future<bitcoinlike_proto::CreateAccountResponse> processRequest(const bitcoinlike_proto::CreateAccountRequest& req);
            Future<bitcoinlike_proto::SyncAccountResponse> processRequest(const bitcoinlike_proto::SyncAccountRequest& req);
            Future<bitcoinlike_proto::GetBalanceResponse> processRequest(const bitcoinlike_proto::GetBalanceRequest& req);
        private:
            std::shared_ptr<WalletPool> _walletPool;
            std::unordered_map<std::string, std::shared_ptr<api::Account>> _accounts;
        };
    }
}