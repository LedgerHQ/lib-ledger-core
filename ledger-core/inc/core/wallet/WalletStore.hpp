#pragma once

#include <memory>
#include <vector>
#include <unordered_map>

#include <core/Services.hpp>
#include <core/api/Currency.hpp>
#include <core/api/ErrorCode.hpp>
#include <core/api/DynamicObject.hpp>
#include <core/api/WalletStore.hpp>
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
        class WalletStore final : public DedicatedContext, public std::enable_shared_from_this<WalletStore>, public api::WalletStore {
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
            void getCurrency(
                const std::string & name,
                const std::function<void(std::experimental::optional<api::Currency>, std::experimental::optional<api::Error>)> & callback
            ) override;

            std::vector<api::Currency> const& getCurrencies() const;
            void getCurrencies(
                const std::function<void(std::experimental::optional<std::vector<api::Currency>>, std::experimental::optional<api::Error>)> & callback
            ) override;

            Future<Unit> addCurrency(api::Currency const& currency);
            Future<Unit> removeCurrency(std::string const& currencyName);

            // Fetch wallet
            Future<int64_t> getWalletCount() const;
            void getWalletCount(
                const std::function<void(std::experimental::optional<int32_t>, std::experimental::optional<api::Error>)> & callback
            ) override;

            Future<std::vector<std::shared_ptr<AbstractWallet>>> getWallets(int64_t from, int64_t size);
            void getWallets(
                int32_t from,
                int32_t size,
                const std::function<void(std::experimental::optional<std::vector<std::shared_ptr<api::Wallet>>>, std::experimental::optional<api::Error>)> & callback
            ) override;

            FuturePtr<AbstractWallet> getWallet(const std::string& name);
            void getWallet(
                const std::string & name,
                const std::function<void(std::shared_ptr<api::Wallet>, std::experimental::optional<api::Error>)> & callback
            ) override;

            Future<api::ErrorCode> updateWalletConfig(
                std::string const& name,
                std::shared_ptr<api::DynamicObject> const& configuration
            );
            void updateWalletConfig(
                const std::string & name,
                const std::shared_ptr<api::DynamicObject> & configuration,
                const std::function<void(std::experimental::optional<api::ErrorCode>, std::experimental::optional<api::Error>)> & callback
            ) override;

            Future<std::vector<std::string>> getWalletNames(int64_t from, int64_t size) const;

            // Create wallet
            FuturePtr<AbstractWallet> createWallet(
                std::string const& name,
                std::string const& currencyName,
                std::shared_ptr<api::DynamicObject> const& configuration
            );
            void createWallet(
                const std::string & name,
                const api::Currency & currency,
                const std::shared_ptr<api::DynamicObject> & configuration,
                const std::function<void(std::shared_ptr<api::Wallet>, std::experimental::optional<api::Error>)> & callback
            ) override;

            // Deletion
            Future<api::ErrorCode> eraseDataSince(const std::chrono::system_clock::time_point & date);
            void eraseDataSince(
                const std::chrono::system_clock::time_point & date,
                const std::function<void(std::experimental::optional<api::ErrorCode>, std::experimental::optional<api::Error>)> & callback
            ) override;

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
