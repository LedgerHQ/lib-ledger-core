#pragma once

#include <string>

#include <core/Services.hpp>
#include <core/api/DynamicObject.hpp>
#include <core/wallet/WalletStore.hpp>
#include <ripple/RippleLikeWallet.hpp>
#include <ripple/RippleLikeAccount.hpp>
#include <ripple/transaction_builders/RippleLikeTransactionBuilder.hpp>
#include <ripple/RippleLikeCurrencies.hpp>

#include "integration/BaseFixture.hpp"

using namespace ledger::core;

struct TransactionTestData {
    std::shared_ptr<api::DynamicObject> configuration;
    std::string walletName;
    std::string currencyName;

    std::function<std::shared_ptr<RippleLikeAccount> (const std::shared_ptr<Services>&,
                                                      const std::shared_ptr<AbstractWallet>& )> inflate_xrp;
};

struct RippleMakeBaseTransaction : public BaseFixture {
    void SetUp() override {
        BaseFixture::SetUp();
        SetUpConfig();
        recreate();
    }

    void recreate() {
        services = newDefaultServices();

        walletStore = newWalletStore(services);
        walletStore->addCurrency(currencies::ripple());

        auto factory = std::make_shared<RippleLikeWalletFactory>(currencies::ripple(), services);
        walletStore->registerFactory(currencies::ripple(), factory);

        wallet = std::dynamic_pointer_cast<RippleLikeWallet>(wait(walletStore->createWallet(
            testData.walletName,
            testData.currencyName,
            testData.configuration
        )));
        account = testData.inflate_xrp(services, wallet);
        currency = wallet->getCurrency();
    }

    void TearDown() override {
        BaseFixture::TearDown();
        account = nullptr;
        wallet = nullptr;
        walletStore = nullptr;
        services = nullptr;
    }

    std::shared_ptr<RippleLikeTransactionBuilder> tx_builder() {
        return std::dynamic_pointer_cast<RippleLikeTransactionBuilder>(account->buildTransaction());
    }

    std::shared_ptr<Services> services;
    std::shared_ptr<WalletStore> walletStore;
    std::shared_ptr<AbstractWallet> wallet;
    std::shared_ptr<RippleLikeAccount> account;
    api::Currency currency;
    TransactionTestData testData;

protected:
    virtual void SetUpConfig() = 0;
};
