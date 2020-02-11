/*
 *
 * WalletTests
 * ledger-core-bitcoin
 *
 * Created by Alexis Le Provost on 28/01/2020.
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

#include <core/api/ErrorCode.hpp>
#include <core/api/ExtendedKeyAccountCreationInfo.hpp>

#include <bitcoin/BitcoinLikeAccount.hpp>
#include <bitcoin/BitcoinLikeCurrencies.hpp>
#include <bitcoin/BitcoinLikeWallet.hpp>
#include <bitcoin/factories/BitcoinLikeWalletFactory.hpp>

#include <integration/WalletFixture.hpp>

#include "fixtures/Fixtures.hpp"

using namespace ledger::testing;

class BitcoinWallets : public WalletFixture<BitcoinLikeWalletFactory> {

};

TEST_F(BitcoinWallets, CreateAndGetWallet) {
    auto const currency = currencies::bitcoin();
    auto const walletName = "bitcoin";

    registerCurrency(currency);

    {
        auto newWallet = wait(walletStore->createWallet(walletName, currency.name, api::DynamicObject::newInstance()));
        auto existingWallet = wait(walletStore->getWallet(walletName));

        EXPECT_TRUE(newWallet.get() == existingWallet.get());
    }
    {

        auto existingWallet = wait(walletStore->getWallet(walletName));
        EXPECT_TRUE(existingWallet->getName() == walletName);
        auto wallets = wait(walletStore->getWallets(0, 1));
        EXPECT_TRUE(wallets.front()->getName() == walletName);
    }
}

TEST_F(BitcoinWallets, CreateNewWallet) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, api::DynamicObject::newInstance()));

    EXPECT_EQ(wallet->getCurrency().name, currency.name);
    EXPECT_TRUE(std::dynamic_pointer_cast<BitcoinLikeWallet>(wallet) != nullptr);
}

TEST_F(BitcoinWallets, GetAccountWithSameInstance) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, api::DynamicObject::newInstance()));
    auto account = createAccount<BitcoinLikeAccount>(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
    auto fetchedAccount = std::dynamic_pointer_cast<BitcoinLikeAccount>(wait(wallet->getAccount(0)));

    EXPECT_EQ(account.get(), fetchedAccount.get());
    EXPECT_EQ(wait(account->getFreshPublicAddresses())[0]->toString(), wait(fetchedAccount->getFreshPublicAddresses())[0]->toString());
}

TEST_F(BitcoinWallets, GetAccountOnEmptyWallet) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, api::DynamicObject::newInstance()));

    EXPECT_THROW(wait(wallet->getAccount(0)), Exception);
}

TEST_F(BitcoinWallets, GetMultipleAccounts) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, api::DynamicObject::newInstance()));

    for (auto i = 0; i < 5; i++) {
        createAccount<BitcoinLikeAccount>(wallet, i, P2PKH_MEDIUM_XPUB_INFO);
    }

    auto accounts = wait(wallet->getAccounts(0, 5));

    EXPECT_EQ(accounts.size(), 5);
}

TEST_F(BitcoinWallets, GetTooManyMultipleAccounts) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, api::DynamicObject::newInstance()));

    for (auto i = 0; i < 5; i++) {
        createAccount<BitcoinLikeAccount>(wallet, i, P2PKH_MEDIUM_XPUB_INFO);
    }

    EXPECT_EQ(wait(wallet->getAccounts(0, 10)).size(), 5);
}

TEST_F(BitcoinWallets, GetAccountAfterPoolReopen) {
    std::string addr;
    auto const currency = currencies::bitcoin();
    auto const walletName = "bitcoin";

    registerCurrency(currency);

    {
        auto wallet = wait(walletStore->createWallet(walletName, currency.name, api::DynamicObject::newInstance()));
        auto account = createAccount<BitcoinLikeAccount>(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);

        addr = wait(account->getFreshPublicAddresses())[0]->toString();
    } 

    {
        auto wallet = wait(walletStore->getWallet(walletName));
        auto account = std::dynamic_pointer_cast<AbstractAccount>(wait(wallet->getAccount(0)));
        
        EXPECT_EQ(wait(account->getFreshPublicAddresses())[0]->toString(), addr);
    }
}

TEST_F(BitcoinWallets, CreateNonContiguousAccount) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, api::DynamicObject::newInstance()));
    auto account = createAccount<BitcoinLikeAccount>(wallet, 6, P2PKH_MEDIUM_XPUB_INFO);
    auto fetchedAccount = std::dynamic_pointer_cast<AbstractAccount>(wait(wallet->getAccount(6)));
    
    EXPECT_EQ(wait(account->getFreshPublicAddresses())[0]->toString(), wait(fetchedAccount->getFreshPublicAddresses())[0]->toString());
    
    auto accountCount = wait(wallet->getAccountCount());
    auto accounts = wait(wallet->getAccounts(0, accountCount));
    
    EXPECT_EQ(wait(std::dynamic_pointer_cast<ledger::core::AbstractAccount>(accounts.front())->getFreshPublicAddresses())[0]->toString(), wait(fetchedAccount->getFreshPublicAddresses())[0]->toString());
}

TEST_F(BitcoinWallets, CreateNonContiguousAccountBis) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, api::DynamicObject::newInstance()));
    
    auto account1 = createAccount<BitcoinLikeAccount>(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
    account1->startBlockchainObservation();
    auto account2 = createAccount<BitcoinLikeAccount>(wallet, 6, P2PKH_MEDIUM_XPUB_INFO);
    account2->startBlockchainObservation();
    auto account3 = createAccount<BitcoinLikeAccount>(wallet, 4, P2PKH_MEDIUM_XPUB_INFO);
    account3->startBlockchainObservation();
    account1->stopBlockchainObservation();
    account2->stopBlockchainObservation();
    account3->stopBlockchainObservation();
}

TEST_F(BitcoinWallets, CreateAccountBug) {
    auto const currency = currencies::bitcoin();

    registerCurrency(currency);

    auto wallet = wait(walletStore->createWallet("bitcoin", currency.name, api::DynamicObject::newInstance()));

    auto list = [this] () -> Future<Unit> {
        return walletStore->getWalletCount().template flatMap<std::vector<std::shared_ptr<AbstractWallet>>>(dispatcher->getMainExecutionContext(), [this] (const int64_t& count) -> Future<std::vector<std::shared_ptr<AbstractWallet>>> {
            return walletStore->getWallets(0, count);
        }).template flatMap<Unit>(dispatcher->getMainExecutionContext(), [] (const std::vector<std::shared_ptr<AbstractWallet>>& wallets) -> Future<Unit> {
            return Future<Unit>::successful(unit);
        });
    };

    std::function<void (int)> loop;
    loop = [&loop, wallet, this, &list] (int index) {
        if (index >= 5) {
            dispatcher->stop();
            return ;
        }
        
        auto info = P2PKH_MEDIUM_XPUB_INFO;

        info.index = index;
        wallet->newAccountWithExtendedKeyInfo(info).onComplete(dispatcher->getMainExecutionContext(), [&loop, index, &list, this] (const TryPtr<api::Account>& res) {
            list().onComplete(dispatcher->getMainExecutionContext(), [res, &loop, &list, index] (const Try<Unit>&) {
                if (res.isSuccess()) {
                    res.getValue()->synchronize();
                    loop(index + 1);
                } else {
                    fmt::print("Failure {}\n", res.getFailure().getMessage());
                    loop(index);
                }
            });
        });
    };

    loop(0);
    dispatcher->waitUntilStopped();
}