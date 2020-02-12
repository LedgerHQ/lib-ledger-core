#include <gtest/gtest.h>

#include <tezos/TezosLikeWallet.hpp>
#include <tezos/TezosLikeCurrencies.hpp>
#include <tezos/factories/TezosLikeWalletFactory.hpp>

#include <integration/WalletFixture.hpp>

struct TezosAccounts : public WalletFixture<TezosLikeWalletFactory> {

};

TEST_F(TezosAccounts, FirstXTZAccountInfo) {
    auto const currency = currencies::tezos();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("my_wallet", currency.name, api::DynamicObject::newInstance()));
    auto info = wait(wallet->getNextAccountCreationInfo());

    EXPECT_EQ(info.index, 0);
    EXPECT_EQ(info.owners.size(), 1);
    EXPECT_EQ(info.derivations.size(), 1);
    EXPECT_EQ(info.owners[0], "main");
    // TODO: XTZ is account-based; the expected derivation is completely
    // questionable. 
    // EXPECT_EQ(info.derivations[0], "44'/1729'/0'/0/0");
}