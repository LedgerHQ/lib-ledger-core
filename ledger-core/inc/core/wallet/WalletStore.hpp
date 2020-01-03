#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include <core/Services.hpp>
#include <core/api/Currency.hpp>
#include <core/api/ErrorCode.hpp>
#include <core/api/DynamicObject.hpp>
#include <core/async/DedicatedContext.hpp>
#include <core/async/Future.hpp>
#include <core/utils/Option.hpp>
#include <core/utils/Unit.hpp>
#include <core/wallet/AbstractWallet.hpp>
#include <core/wallet/AbstractWalletFactory.hpp>
#include <core/wallet/WalletDatabaseEntry.hpp>

namespace ledger {
    namespace core {
        // A wallet store which maps wallet names to abstract wallets.
        //
        // This type is a current workaround as it implies downcasting wallets to their real representation.
        class WalletStore final : public DedicatedContext, public std::enable_shared_from_this<WalletStore> {
            std::shared_ptr<Services> _services;

            // Factories management
            std::unordered_map<std::string, std::shared_ptr<AbstractWalletFactory>> _factories;

            // Wallets
            std::unordered_map<std::string, std::shared_ptr<AbstractWallet>> _wallets;

            // Currencies
            std::vector<api::Currency> _currencies;

            // Event filter variables
            std::mutex _eventFilterMutex;
            std::unordered_map<std::string, int64_t> _lastEmittedBlocks;

        public:
            WalletStore() = delete;
            ~WalletStore() = default;

            WalletStore(std::shared_ptr<Services> const& services);

            // Currencies
            Option<api::Currency> getCurrency(std::string const& name) const;
            std::vector<api::Currency> const& getCurrencies() const;
            Future<Unit> addCurrency(api::Currency const& currency);
            Future<Unit> removeCurrency(std::string const& currencyName);

            // Fetch wallet
            Future<int64_t> getWalletCount() const;
            Future<std::vector<std::shared_ptr<AbstractWallet>>> getWallets(int64_t from, int64_t size);
            FuturePtr<AbstractWallet> getWallet(const std::string& name);
            Future<api::ErrorCode> updateWalletConfig(
                std::string const& name,
                std::shared_ptr<api::DynamicObject> const& configuration
            );
            Future<std::vector<std::string>> getWalletNames(int64_t from, int64_t size) const;

            // Create wallet
            FuturePtr<AbstractWallet> createWallet(
                std::string const& name,
                std::string const& currencyName,
                std::shared_ptr<api::DynamicObject> const& configuration
            );

            // Deletion
            Future<api::ErrorCode> eraseDataSince(const std::chrono::system_clock::time_point & date);

            // Factories

            // Register a factory for a given currency. Return true if registered and false if
            // already registered.
            bool registerFactory(
                api::Currency const& currency,
                std::shared_ptr<AbstractWalletFactory> const& factory
            );
            std::shared_ptr<AbstractWalletFactory> getFactory(std::string const& currencyName) const;

        private:
            std::shared_ptr<AbstractWallet> buildWallet(WalletDatabaseEntry const& entry);

            Option<WalletDatabaseEntry> getWalletEntryFromDatabase(std::string const& name);
        };
    }
}
