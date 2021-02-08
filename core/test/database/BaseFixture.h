/*
 *
 * BaseFixture.h
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/09/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#ifndef LEDGER_CORE_BASEFIXTURE_H
#define LEDGER_CORE_BASEFIXTURE_H

#include <gtest/gtest.h>
#include <UvThreadDispatcher.hpp>
#include <src/database/DatabaseSessionPool.hpp>
#include <NativePathResolver.hpp>
#include <unordered_set>
#include <src/wallet/pool/WalletPool.hpp>
#include <CoutLogPrinter.hpp>
#include <src/api/DynamicObject.hpp>
#include <wallet/common/CurrencyBuilder.hpp>
#include <wallet/bitcoin/BitcoinLikeWallet.hpp>
#include <wallet/bitcoin/database/BitcoinLikeWalletDatabase.h>
#include <wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h>
#include <wallet/common/database/AccountDatabaseHelper.h>
#include <wallet/pool/database/PoolDatabaseHelper.hpp>
#include <utils/JSONUtils.h>
#include <wallet/bitcoin/explorers/api/TransactionParser.hpp>
#include <wallet/bitcoin/BitcoinLikeAccount.hpp>
#include <api/BitcoinLikeOperation.hpp>
#include <api/BitcoinLikeTransaction.hpp>
#include <api/BitcoinLikeInput.hpp>
#include <api/BitcoinLikeOutput.hpp>
#include <api/BigInt.hpp>
#include <CppHttpLibClient.hpp>
#include <events/LambdaEventReceiver.hpp>
#include <soci.h>
#include <api/Account.hpp>
#include <api/BitcoinLikeAccount.hpp>

using namespace ledger::core; // Only do that for testing
using namespace ledger::core::test;

extern api::ExtendedKeyAccountCreationInfo P2PKH_MEDIUM_XPUB_INFO;
extern api::AccountCreationInfo P2PKH_MEDIUM_KEYS_INFO;
extern api::ExtendedKeyAccountCreationInfo P2PKH_BIG_XPUB_INFO;
extern const std::string TX_1;
extern const std::string TX_2;
extern const std::string TX_3;
extern const std::string TX_4;


class BaseFixture : public ::testing::Test {
public:
    void SetUp() override;
    void TearDown() override;
    std::shared_ptr<WalletPool> newDefaultPool(std::string poolName = "my_ppol");
    void createWallet(const std::shared_ptr<WalletPool>& pool,
                      const std::string& walletName,
                      const std::string& currencyName,
                      const std::shared_ptr<api::DynamicObject> &configuration);

    void createAccount(const std::shared_ptr<WalletPool>& pool, const std::string& walletName, int32_t index);
    BitcoinLikeWalletDatabase newBitcoinAccount(const std::shared_ptr<WalletPool>& pool,
                                                const std::string& walletName,
                                                const std::string& currencyName,
                                                const std::shared_ptr<api::DynamicObject> &configuration,
                                                int32_t index,
                                                const std::string& xpub);
    std::shared_ptr<BitcoinLikeAccount> createBitcoinLikeAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                                                int32_t index,
                                                                const api::AccountCreationInfo &info
    );
    std::shared_ptr<BitcoinLikeAccount> createBitcoinLikeAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                                                 int32_t index,
                                                                 const api::ExtendedKeyAccountCreationInfo& info
    );

    std::shared_ptr<uv::UvThreadDispatcher> dispatcher;
    std::shared_ptr<NativePathResolver> resolver;
    std::shared_ptr<DatabaseBackend> backend;
    std::shared_ptr<CoutLogPrinter> printer;
    std::shared_ptr<CppHttpLibClient> http;
};

#endif //LEDGER_CORE_BASEFIXTURE_H
