/*
 *
 * wallet_tests
 * ledger-core
 *
 * Created by Pierre Pollastri on 19/05/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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

#include "BaseFixture.h"

class WalletTests : public BaseFixture {

};

TEST_F(WalletTests, CreateNewWallet) {
    auto pool = newDefaultPool();
    auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", api::DynamicObject::newInstance()));
    EXPECT_EQ(wallet->getCurrency().name, "bitcoin");
    EXPECT_EQ(wallet->getWalletType(), api::WalletType::BITCOIN);
    auto bitcoinWallet = std::dynamic_pointer_cast<BitcoinLikeWallet>(wallet);
    EXPECT_TRUE(bitcoinWallet != nullptr);
}

TEST_F(WalletTests, GetAccountWithSameInstance) {
    auto pool = newDefaultPool();
    auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", api::DynamicObject::newInstance()));
    auto account = createBitcoinLikeAccount(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
    auto fetchedAccount = std::dynamic_pointer_cast<BitcoinLikeAccount>(wait(wallet->getAccount(0)));
    EXPECT_EQ(account.get(), fetchedAccount.get());
    EXPECT_EQ(wait(account->getFreshPublicAddresses())[0], wait(fetchedAccount->getFreshPublicAddresses())[0]);
}

TEST_F(WalletTests, GetAccountOnEmptyWallet) {
    auto pool = newDefaultPool();
    auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", api::DynamicObject::newInstance()));
    EXPECT_THROW(wait(wallet->getAccount(0)), Exception);
}

TEST_F(WalletTests, GetMultipleAccounts) {
    auto pool = newDefaultPool();
    auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", api::DynamicObject::newInstance()));
    for (auto i = 0; i < 5; i++)
        createBitcoinLikeAccount(wallet, i, P2PKH_MEDIUM_XPUB_INFO);
    auto accounts = wait(wallet->getAccounts(0, 5));
    EXPECT_EQ(accounts.size(), 5);
}

TEST_F(WalletTests, GetTooManyMultipleAccounts) {
    auto pool = newDefaultPool();
    auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", api::DynamicObject::newInstance()));
    for (auto i = 0; i < 5; i++)
        createBitcoinLikeAccount(wallet, i, P2PKH_MEDIUM_XPUB_INFO);
    EXPECT_EQ(wait(wallet->getAccounts(0, 10)).size(), 5);
}

TEST_F(WalletTests, GetAccountAfterPoolReopen) {
    std::string addr;
    {
        auto pool = newDefaultPool();
        auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", api::DynamicObject::newInstance()));
        auto account = createBitcoinLikeAccount(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
        addr = wait(account->getFreshPublicAddresses())[0]->toString();
    }
    {
        auto pool = newDefaultPool();
        auto wallet = wait(pool->getWallet("my_wallet"));
        auto account = std::dynamic_pointer_cast<AbstractAccount>(wait(wallet->getAccount(0)));
        EXPECT_EQ(wait(account->getFreshPublicAddresses())[0]->toString(), addr);
    }
}

TEST_F(WalletTests, CreateNonContiguousAccount) {
    auto pool = newDefaultPool();
    auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", api::DynamicObject::newInstance()));
    auto account = createBitcoinLikeAccount(wallet, 6, P2PKH_MEDIUM_XPUB_INFO);
    auto fetchedAccount = std::dynamic_pointer_cast<AbstractAccount>(wait(wallet->getAccount(6)));
    EXPECT_EQ(wait(account->getFreshPublicAddresses())[0], wait(fetchedAccount->getFreshPublicAddresses())[0]);
    auto accountCount = wait(wallet->getAccountCount());
    auto accounts = wait(wallet->getAccounts(0, accountCount));
    EXPECT_EQ(wait(std::dynamic_pointer_cast<ledger::core::AbstractAccount>(accounts.front())->getFreshPublicAddresses())[0], wait(fetchedAccount->getFreshPublicAddresses())[0]);
}


TEST_F(WalletTests, CreateNonContiguousAccountBis) {
    auto pool = newDefaultPool();
    auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", api::DynamicObject::newInstance()));
    auto account1 = createBitcoinLikeAccount(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
    account1->startBlockchainObservation();
    auto account2 = createBitcoinLikeAccount(wallet, 6, P2PKH_MEDIUM_XPUB_INFO);
    account2->startBlockchainObservation();
    auto account3 = createBitcoinLikeAccount(wallet, 4, P2PKH_MEDIUM_XPUB_INFO);
    account3->startBlockchainObservation();
    account1->stopBlockchainObservation();
    account2->stopBlockchainObservation();
    account3->stopBlockchainObservation();
}

TEST_F(WalletTests, CreateAccountBug) {
    auto pool = newDefaultPool();
    auto wallet = wait(pool->createWallet("my_wallet", "bitcoin", api::DynamicObject::newInstance()));
    auto list = [pool, this] () -> Future<Unit> {
        return pool->getWalletCount().flatMap<std::vector<std::shared_ptr<AbstractWallet>>>(dispatcher->getMainExecutionContext(), [pool] (const int64_t& count) -> Future<std::vector<std::shared_ptr<AbstractWallet>>> {
            return pool->getWallets(0, count);
        }).flatMap<Unit>(dispatcher->getMainExecutionContext(), [] (const std::vector<std::shared_ptr<AbstractWallet>>& wallets) -> Future<Unit> {
            return Future<Unit>::successful(unit);
        });
    };
    std::function<void (int)> loop;
    loop = [&loop, &wallet, this, list] (int index) {
        if (index >= 250) {
            dispatcher->stop();
            return ;
        }
        auto info = P2PKH_MEDIUM_XPUB_INFO;
        info.index = index;
        wallet->newAccountWithExtendedKeyInfo(info).onComplete(dispatcher->getMainExecutionContext(), [&loop, index, list, this] (const TryPtr<api::Account>& res) {
            list().onComplete(dispatcher->getMainExecutionContext(), [res, &loop, list, index] (const Try<Unit>&) {
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