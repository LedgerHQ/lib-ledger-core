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
        private:
            Future<message::bitcoin::SyncAccountResponse> processRequest(const message::bitcoin::SyncAccountRequest& req);
            Future<message::bitcoin::GetBalanceResponse> processRequest(const message::bitcoin::GetBalanceRequest& req);
            Future<message::bitcoin::GetOperationsResponse> processRequest(const message::bitcoin::GetOperationsRequest& req);
            Future<message::bitcoin::GetLastBlockResponse> processRequest(const message::bitcoin::GetLastBlockRequest& req);
            Future<message::bitcoin::GetFreshAddressResponse> processRequest(const message::bitcoin::GetFreshAddressRequest& req);
            Future<std::shared_ptr<api::Account>> getOrCreateAccount(const message::bitcoin::AccountID& ccountID);
            std::shared_ptr<WalletPool> _walletPool;
        };
    }
}