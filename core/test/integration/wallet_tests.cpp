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
#include <ledger/core/api/ErrorCode.hpp>
class WalletTests : public BaseFixture {

};

TEST_F(WalletTests, CreateNewWallet) {
    auto pool = newDefaultPool();
    auto wallet = uv::wait(pool->createWallet(randomWalletName(), "bitcoin", api::DynamicObject::newInstance()));
    EXPECT_EQ(wallet->getCurrency().name, "bitcoin");
    EXPECT_EQ(wallet->getWalletType(), api::WalletType::BITCOIN);
    auto bitcoinWallet = std::dynamic_pointer_cast<BitcoinLikeWallet>(wallet);
    EXPECT_TRUE(bitcoinWallet != nullptr);
}

TEST_F(WalletTests, GetAccountWithSameInstance) {
    auto pool = newDefaultPool();
    auto wallet = uv::wait(pool->createWallet(randomWalletName(), "bitcoin", api::DynamicObject::newInstance()));
    auto account = createBitcoinLikeAccount(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
    auto fetchedAccount = std::dynamic_pointer_cast<BitcoinLikeAccount>(uv::wait(wallet->getAccount(0)));
    EXPECT_EQ(account.get(), fetchedAccount.get());
    EXPECT_EQ(uv::wait(account->getFreshPublicAddresses())[0]->toString(), uv::wait(fetchedAccount->getFreshPublicAddresses())[0]->toString());
}

TEST_F(WalletTests, GetAccountOnEmptyWallet) {
    auto pool = newDefaultPool();
    auto wallet = uv::wait(pool->createWallet(randomWalletName(), "bitcoin", api::DynamicObject::newInstance()));
    EXPECT_THROW(uv::wait(wallet->getAccount(0)), Exception);
}

TEST_F(WalletTests, GetMultipleAccounts) {
    auto pool = newDefaultPool();
    auto wallet = uv::wait(pool->createWallet(randomWalletName(), "bitcoin", api::DynamicObject::newInstance()));
    for (auto i = 0; i < 5; i++)
        createBitcoinLikeAccount(wallet, i, P2PKH_MEDIUM_XPUB_INFO);
    auto accounts = uv::wait(wallet->getAccounts(0, 5));
    EXPECT_EQ(accounts.size(), 5);
}

TEST_F(WalletTests, GetTooManyMultipleAccounts) {
    auto pool = newDefaultPool();
    auto wallet = uv::wait(pool->createWallet(randomWalletName(), "bitcoin", api::DynamicObject::newInstance()));
    for (auto i = 0; i < 5; i++)
        createBitcoinLikeAccount(wallet, i, P2PKH_MEDIUM_XPUB_INFO);
    EXPECT_EQ(uv::wait(wallet->getAccounts(0, 10)).size(), 5);
}

TEST_F(WalletTests, GetAccountAfterPoolReopen) {
    std::string addr;
    const auto walletName = randomWalletName();
    auto pool = newDefaultPool();
    {
        auto wallet = uv::wait(pool->createWallet(walletName, "bitcoin", api::DynamicObject::newInstance()));
        auto account = createBitcoinLikeAccount(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
        addr = uv::wait(account->getFreshPublicAddresses())[0]->toString();
    }
    {
        auto wallet = uv::wait(pool->getWallet(walletName));
        auto account = std::dynamic_pointer_cast<AbstractAccount>(uv::wait(wallet->getAccount(0)));
        EXPECT_EQ(uv::wait(account->getFreshPublicAddresses())[0]->toString(), addr);
    }
}

TEST_F(WalletTests, CreateNonContiguousAccount) {
    auto pool = newDefaultPool();
    auto wallet = uv::wait(pool->createWallet(randomWalletName(), "bitcoin", api::DynamicObject::newInstance()));
    auto account = createBitcoinLikeAccount(wallet, 6, P2PKH_MEDIUM_XPUB_INFO);
    auto fetchedAccount = std::dynamic_pointer_cast<AbstractAccount>(uv::wait(wallet->getAccount(6)));
    EXPECT_EQ(uv::wait(account->getFreshPublicAddresses())[0]->toString(), uv::wait(fetchedAccount->getFreshPublicAddresses())[0]->toString());
    auto accountCount = uv::wait(wallet->getAccountCount());
    auto accounts = uv::wait(wallet->getAccounts(0, accountCount));
    EXPECT_EQ(uv::wait(std::dynamic_pointer_cast<ledger::core::AbstractAccount>(accounts.front())->getFreshPublicAddresses())[0]->toString(), uv::wait(fetchedAccount->getFreshPublicAddresses())[0]->toString());
}


TEST_F(WalletTests, CreateNonContiguousAccountBis) {
    auto pool = newDefaultPool();
    auto wallet = uv::wait(pool->createWallet(randomWalletName(), "bitcoin", api::DynamicObject::newInstance()));
    auto account1 = createBitcoinLikeAccount(wallet, 0, P2PKH_MEDIUM_XPUB_INFO);
    auto account2 = createBitcoinLikeAccount(wallet, 6, P2PKH_MEDIUM_XPUB_INFO);
    auto account3 = createBitcoinLikeAccount(wallet, 4, P2PKH_MEDIUM_XPUB_INFO);
}

TEST_F(WalletTests, CreateAccountBug) {
    auto pool = newDefaultPool();
    auto wallet = uv::wait(pool->createWallet(randomWalletName(), "bitcoin", api::DynamicObject::newInstance()));
    auto list = [pool, this] () -> Future<Unit> {
        return pool->getWalletCount().flatMap<std::vector<std::shared_ptr<AbstractWallet>>>(dispatcher->getMainExecutionContext(), [pool] (const int64_t& count) -> Future<std::vector<std::shared_ptr<AbstractWallet>>> {
            return pool->getWallets(0, count);
        }).flatMap<Unit>(dispatcher->getMainExecutionContext(), [] (const std::vector<std::shared_ptr<AbstractWallet>>& wallets) -> Future<Unit> {
            return Future<Unit>::successful(unit);
        });
    };
    //Otherwise test takes a long time
    //int LOOP_COUNT = 250;
    int LOOP_COUNT = 5;
    std::function<void (int)> loop;
    loop = [&loop, &wallet, this, list, LOOP_COUNT] (int index) {
        if (index >= LOOP_COUNT) {
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

TEST_F(WalletTests, ChangeWalletConfig) {
    auto pool = newDefaultPool();
    auto walletName = randomWalletName();
    auto derivationScheme = "44'/<coin_type>'/<account>'/<node>/<address>";
    {
        auto oldEndpoint = "http://eth-ropsten.explorers.dev.aws.ledger.fr";
        auto config = api::DynamicObject::newInstance();
        config->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, oldEndpoint);
        config->putString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME, derivationScheme);
        auto wallet = uv::wait(pool->createWallet(walletName, "ethereum", config));
        auto walletConfig = wallet->getConfiguration();
        EXPECT_EQ(walletConfig->getString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT).value(), oldEndpoint);
        EXPECT_EQ(walletConfig->getString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME).value(), derivationScheme);
    }
    {
        auto newEndpoint = "http://eth-ropsten.explorers.prod.aws.ledger.fr";
        auto config = api::DynamicObject::newInstance();
        config->putString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT, newEndpoint);
        auto returnCode = uv::wait(pool->updateWalletConfig(walletName, config));
        EXPECT_EQ(returnCode, api::ErrorCode::FUTURE_WAS_SUCCESSFULL);
        auto wallet = uv::wait(pool->getWallet(walletName));
        auto walletConfig = wallet->getConfiguration();
        EXPECT_EQ(walletConfig->getString(api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT).value(), newEndpoint);
        EXPECT_EQ(walletConfig->getString(api::Configuration::KEYCHAIN_DERIVATION_SCHEME).value(), derivationScheme);
    }
}
