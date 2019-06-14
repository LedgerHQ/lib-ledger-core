#pragma once
#include <string>
#include <memory>
#include <unordered_map>
#include "messages/bitcoin/commands.pb.h"
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
            Future<message::bitcoin::CreateAccountResponse> processRequest(const message::bitcoin::CreateAccountRequest& req);
            Future<message::bitcoin::SyncAccountResponse> processRequest(const message::bitcoin::SyncAccountRequest& req);
            Future<message::bitcoin::GetBalanceResponse> processRequest(const message::bitcoin::GetBalanceRequest& req);
        private:
            std::shared_ptr<WalletPool> _walletPool;
            std::unordered_map<std::string, std::shared_ptr<api::Account>> _accounts;
        };
    }
}