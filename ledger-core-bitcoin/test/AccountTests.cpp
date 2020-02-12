/*
 *
 * AdccountTests
 * ledger-core-bitcoin
 *
 * Created by Alexis Le Provost on 30/01/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#include <gtest/gtest.h>

#include <core/api/DynamicObject.hpp>
#include <core/api/KeychainEngines.hpp>

#include <bitcoin/BitcoinLikeCurrencies.hpp>
#include <bitcoin/BitcoinLikeAccount.hpp>
#include <bitcoin/factories/BitcoinLikeWalletFactory.hpp>
#include <bitcoin/operations/BitcoinLikeOperationQuery.hpp>
#include <integration/WalletFixture.hpp>

#include "fixtures/Fixtures.hpp"
#include "fixtures/MediumXPubFixtures.hpp"
#include "fixtures/TestnetXPubFixtures.hpp"

class BitcoinAccounts : public WalletFixture<BitcoinLikeWalletFactory> {

};

using namespace ledger::testing;

TEST_F(BitcoinAccounts, CreateAccountWithInfo) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, api::DynamicObject::newInstance()));
    auto account = createAccount<BitcoinLikeAccount>(wallet, 0, P2PKH_MEDIUM_KEYS_INFO);
    auto address = wait(account->getFreshPublicAddresses())[0]->toString();

    EXPECT_EQ(address, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7"); 
}

TEST_F(BitcoinAccounts, CreateAccountWithInfoOnExistingWallet) {
    auto const currency = currencies::bitcoin();
    auto const walletName = "bitcoin";

    registerCurrency(currency);

    wait(walletStore->createWallet(walletName, currency.name, api::DynamicObject::newInstance()));
    
    EXPECT_THROW(wait(walletStore->createWallet(walletName, currency.name, api::DynamicObject::newInstance())), Exception);

    auto wallet = wait(walletStore->getWallet(walletName));
    auto account = createAccount<BitcoinLikeAccount>(wallet, 0, P2PKH_MEDIUM_KEYS_INFO);
    auto address = wait(account->getFreshPublicAddresses())[0]->toString();

    EXPECT_EQ(address, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7"); 
}

TEST_F(BitcoinAccounts, ChangePassword) {
    auto const currency = currencies::bitcoin();
    auto const walletName = "bitcoin";
    std::string oldPassword;
    auto newPassword = DEFAULT_PASSWORD;

    registerCurrency(currency);

    {
        auto wallet = wait(walletStore->createWallet(walletName, currency.name, api::DynamicObject::newInstance()));
        auto account = createAccount<BitcoinLikeAccount>(wallet, 0, P2PKH_MEDIUM_KEYS_INFO);
        auto address = wait(account->getFreshPublicAddresses())[0]->toString();

        EXPECT_EQ(address, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7"); 
    }

    for (auto &password : {"new_test", "new_test_0", ""}) {
        oldPassword = newPassword;
        newPassword = password;

        wait(services->changePassword(oldPassword, newPassword));
        {
            auto wallet = wait(walletStore->getWallet(walletName));
            auto account = std::dynamic_pointer_cast<BitcoinLikeAccount>(wait(wallet->getAccount(0)));
            auto address = wait(account->getFreshPublicAddresses())[0]->toString();

            EXPECT_EQ(address, "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7");  
        }
    }
}

TEST_F(BitcoinAccounts, FirstAccountInfo) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, api::DynamicObject::newInstance()));
    auto info = wait(wallet->getNextAccountCreationInfo());

    EXPECT_EQ(info.index, 0);
    EXPECT_EQ(info.owners[0], "main");
    EXPECT_EQ(info.derivations[0], "44'/0'");
    EXPECT_EQ(info.owners[1], "main");
    EXPECT_EQ(info.derivations[1], "44'/0'/0'");
}

TEST_F(BitcoinAccounts, AnotherAccountInfo) {
    auto const currency = currencies::bitcoin();
    
    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, DynamicObject::newInstance()));
    auto info = wait(wallet->getAccountCreationInfo(20));

    EXPECT_EQ(info.index, 20);
    EXPECT_EQ(info.owners[0], "main");
    EXPECT_EQ(info.derivations[0], "44'/0'");
    EXPECT_EQ(info.owners[1], "main");
    EXPECT_EQ(info.derivations[1], "44'/0'/20'");
}

TEST_F(BitcoinAccounts, GetAddressOnEmptyAccount) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, api::DynamicObject::newInstance()));
    auto account = createAccount<BitcoinLikeAccount>(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
    auto addresses = wait(account->getFreshPublicAddresses());

    EXPECT_EQ(addresses.size(), 20);
    EXPECT_EQ(addresses.front()->toString(), "1DDBzjLyAmDr4qLRC2T2WJ831cxBM5v7G7");
}

TEST_F(BitcoinAccounts, GetBalanceOnEmptyAccount) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, api::DynamicObject::newInstance()));
    auto account = createAccount<BitcoinLikeAccount>(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
    auto balance = wait(account->getBalance());
    
    EXPECT_EQ(balance->toMagnitude(0)->toLong(), 0);
}

TEST_F(BitcoinAccounts, QueryOperationsOnEmptyAccount) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, api::DynamicObject::newInstance()));
    auto account = createAccount<BitcoinLikeAccount>(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
    auto query = std::dynamic_pointer_cast<BitcoinLikeOperationQuery>(account->queryOperations()->limit(100)->partial());
    auto operations = wait(query->execute());

    EXPECT_EQ(operations.size(), 0);
}

TEST_F(BitcoinAccounts, GetBalanceOnAccountWithSomeTxs) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, api::DynamicObject::newInstance())); 
    auto account = medium_xpub::inflate(services, wallet);
    auto balance = wait(account->getBalance());
    auto utxos = wait(account->getUTXO());
    auto uxtoCount = wait(account->getUTXOCount());

    EXPECT_EQ(balance->toLong(), 143590816L);
    EXPECT_EQ(utxos.size(), 8);
    EXPECT_EQ(uxtoCount, 8);
}

TEST_F(BitcoinAccounts, GetBalanceHistoryOnAccountWithSomeTxs) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, api::DynamicObject::newInstance()));
    auto account = medium_xpub::inflate(services, wallet);
    auto fromDate = "2017-10-12T13:38:23Z";
    auto toDate = DateUtils::toJSON(DateUtils::now());
    auto balanceHistory = wait(account->getBalanceHistory(fromDate, toDate, api::TimePeriod::MONTH));
    auto balance = wait(account->getBalance());

    EXPECT_EQ(balanceHistory[balanceHistory.size() - 1]->toLong(), balance->toLong());
}

TEST_F(BitcoinAccounts, QueryOperations) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, api::DynamicObject::newInstance()));
    auto account = medium_xpub::inflate(services, wallet);
    auto query = std::dynamic_pointer_cast<BitcoinLikeOperationQuery>(account->queryOperations()->limit(100)->partial());
    auto operations = wait(query->execute());

    EXPECT_EQ(operations.size(), 100);
}

TEST_F(BitcoinAccounts, GetTestnetUnits) {
    auto const currency = currencies::bitcoin_testnet();
    auto configuration = DynamicObject::newInstance();

    registerCurrency(currency);
    registerCurrency(currencies::bitcoin());

    configuration->putString(api::Configuration::KEYCHAIN_ENGINE,api::KeychainEngines::BIP49_P2SH);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, configuration));
    auto targetedCurrency = walletStore->getCurrency("bitcoin");
    EXPECT_EQ(targetedCurrency->name, "bitcoin");
    std::cout<<">>>> Get account"<<std::endl;
    auto account = testnet_xpub::inflate(services, wallet);
    std::cout<<">>>> Get balance"<<std::endl;
    auto balance = wait(account->getBalance());
    std::cout<<">>>> Get balance toLong"<<std::endl;
    auto balanceLong = balance->toLong();
    //EXPECT_EQ(balanceLong, 100L);
    //auto balance = wait(account->getBalance());
    //auto balance =
    //EXPECT_EQ();
}
