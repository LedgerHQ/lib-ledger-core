#pragma once

#include <string>

#include <core/Services.hpp>
#include <core/api/DynamicObject.hpp>
#include <core/wallet/WalletStore.hpp>

#include "integration/BaseFixture.hpp"

using namespace ledger::core;

template <typename CoinAccount>
struct TransactionTestData {
    std::shared_ptr<api::DynamicObject> configuration;
    std::string walletName;
    std::string currencyName;

    std::function<std::shared_ptr<CoinAccount> (
        const std::shared_ptr<Services>&,
        const std::shared_ptr<AbstractWallet>&
    )> inflate_coin;
};

template <typename CoinAccount, typename CoinWalletFactory, typename CoinWallet, typename CoinTransactionBuilder>
struct MakeBaseTransaction : public BaseFixture {
    void SetUp() override {
        BaseFixture::SetUp();
        SetUpConfig();
        recreate();
    }

    void recreate() {
        services = newDefaultServices();

        walletStore = newWalletStore(services);
        walletStore->addCurrency(getCurrency());

        auto factory = std::make_shared<CoinWalletFactory>(getCurrency(), services);
        walletStore->registerFactory(getCurrency(), factory);

        wallet = std::dynamic_pointer_cast<CoinWallet>(wait(walletStore->createWallet(
            testData.walletName,
            testData.currencyName,
            testData.configuration
        )));
        account = testData.inflate_coin(services, wallet);
        currency = wallet->getCurrency();
    }

    void TearDown() override {
        account = nullptr;
        wallet = nullptr;
        walletStore = nullptr;

        wait(services->freshResetAll());
        services = nullptr;
        BaseFixture::TearDown();
    }

    std::shared_ptr<CoinTransactionBuilder> tx_builder() {
        return std::dynamic_pointer_cast<CoinTransactionBuilder>(account->buildTransaction());
    }

    std::shared_ptr<Services> services;
    std::shared_ptr<WalletStore> walletStore;
    std::shared_ptr<AbstractWallet> wallet;
    std::shared_ptr<CoinAccount> account;
    api::Currency currency;
    TransactionTestData<CoinAccount> testData;

protected:
    virtual api::Currency getCurrency() const = 0;
    virtual void SetUpConfig() = 0;
};
