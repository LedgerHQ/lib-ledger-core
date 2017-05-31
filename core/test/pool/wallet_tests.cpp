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

#include <gtest/gtest.h>
#include <async/QtThreadDispatcher.hpp>
#include <src/database/DatabaseSessionPool.hpp>
#include <NativePathResolver.hpp>
#include <unordered_set>
#include <src/wallet/pool/WalletPool.hpp>
#include <CoutLogPrinter.hpp>
#include <src/api/DynamicObject.hpp>
#include <wallet/common/CurrencyBuilder.hpp>
#include <wallet/bitcoin/BitcoinLikeWallet.hpp>

using namespace ledger::core;
using namespace ledger::qt;

TEST(Wallets, CreateNewWallet) {
    auto dispatcher = std::make_shared<QtThreadDispatcher>();
    auto resolver = std::make_shared<NativePathResolver>();
    auto backend = std::static_pointer_cast<DatabaseBackend>(DatabaseBackend::getSqlite3Backend());
    auto printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    auto newPool = [&]() -> std::shared_ptr<WalletPool> {
        return WalletPool::newInstance(
            "my_pool",
            Option<std::string>::NONE,
            nullptr,
            nullptr,
            resolver,
            printer,
            dispatcher,
            nullptr,
            backend,
            api::DynamicObject::newInstance()
        );
    };
    {
        auto pool = newPool();
        pool->createWallet("my_wallet", "bitcoin", api::DynamicObject::newInstance())
            .onComplete(dispatcher->getMainExecutionContext(), [&] (const TryPtr<AbstractWallet>& result) {
                if (result.isFailure()) {
                    std::cerr << result.getFailure().getMessage() << std::endl;
                    FAIL();
                } else {
                    auto& wallet = result.getValue();
                    EXPECT_EQ(wallet->getCurrency().name, "bitcoin");
                    EXPECT_EQ(wallet->getWalletType(), api::WalletType::BITCOIN);
                    auto bitcoinWallet = std::dynamic_pointer_cast<BitcoinLikeWallet>(wallet);
                    EXPECT_TRUE(bitcoinWallet != nullptr);
                }
                dispatcher->stop();
            });
        dispatcher->waitUntilStopped();
    }
    resolver->clean();
}