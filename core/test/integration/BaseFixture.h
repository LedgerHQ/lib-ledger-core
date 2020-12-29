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
#include <wallet/ethereum/EthereumLikeAccount.h>
#include <wallet/ripple/RippleLikeAccount.h>
#include <wallet/algorand/AlgorandAccount.hpp>
#include <wallet/tezos/TezosLikeAccount.h>
#include <api/BitcoinLikeOperation.hpp>
#include <api/BitcoinLikeTransaction.hpp>
#include <api/BitcoinLikeInput.hpp>
#include <api/BitcoinLikeOutput.hpp>
#include <api/BigInt.hpp>
#include <CppHttpLibClient.hpp>
#include <ProxyHttpClient.hpp>
#include <events/LambdaEventReceiver.hpp>
#include <soci.h>
#include <api/Account.hpp>
#include <api/BitcoinLikeAccount.hpp>
#include <FakeWebSocketClient.h>
#include <OpenSSLRandomNumberGenerator.hpp>
#include <utils/FilesystemUtils.hpp>

using namespace ledger::core; // Only do that for testing
using namespace ledger::core::test;

extern api::ExtendedKeyAccountCreationInfo P2PKH_MEDIUM_XPUB_INFO;
extern api::ExtendedKeyAccountCreationInfo P2WPKH_MEDIUM_XPUB_INFO;
extern api::AccountCreationInfo P2PKH_MEDIUM_KEYS_INFO;
extern api::AccountCreationInfo P2WPKH_DGB_MEDIUM_KEYS_INFO;
extern api::AccountCreationInfo P2WPKH_LTC_MEDIUM_KEYS_INFO;
extern api::ExtendedKeyAccountCreationInfo P2PKH_BIG_XPUB_INFO;
extern api::ExtendedKeyAccountCreationInfo P2SH_XPUB_INFO;
extern api::ExtendedKeyAccountCreationInfo ETH_XPUB_INFO;
extern api::ExtendedKeyAccountCreationInfo ETH_ROPSTEN_XPUB_INFO;
extern api::ExtendedKeyAccountCreationInfo ETH_MAIN_XPUB_INFO;
extern api::AccountCreationInfo ETH_KEYS_INFO;
extern api::AccountCreationInfo ETH_KEYS_INFO_VAULT;
extern api::AccountCreationInfo ETH_KEYS_INFO_LIVE;
extern api::AccountCreationInfo ETC_KEYS_INFO_LIVE;
extern api::AccountCreationInfo XRP_KEYS_INFO;
extern api::AccountCreationInfo VAULT_XRP_KEYS_INFO;
extern api::AccountCreationInfo XTZ_KEYS_INFO;
extern api::AccountCreationInfo XTZ_WITH_100_OPS_KEYS_INFO;
extern api::AccountCreationInfo XTZ_NON_ACTIVATED_KEYS_INFO;
extern const std::string TX_1;
extern const std::string TX_2;
extern const std::string TX_3;
extern const std::string TX_4;


class BaseFixture : public ::testing::Test {
public:
    virtual void SetUp() override;
    virtual void TearDown() override;
    std::shared_ptr<WalletPool> newDefaultPool(const std::string &poolName = "my_ppol",
                                               const std::string &password = "test",
                                               const std::shared_ptr<api::DynamicObject> &configuration = api::DynamicObject::newInstance(),
                                               bool usePostgreSQL = false);
    void createWallet(const std::shared_ptr<WalletPool>& pool,
                      const std::string& walletName,
                      const std::string& currencyName,
                      const std::shared_ptr<api::DynamicObject> &configuration);
    void createAccount(const std::shared_ptr<WalletPool>& pool, const std::string &walletName, int32_t index);
    BitcoinLikeWalletDatabase newBitcoinAccount(const std::shared_ptr<WalletPool>& pool,
                                                const std::string& walletName,
                                                const std::string& currencyName,
                                                const std::shared_ptr<api::DynamicObject> &configuration,
                                                int32_t index,
                                                const std::string& xpub);
    std::shared_ptr<BitcoinLikeAccount> createBitcoinLikeAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                                                int32_t index,
                                                                const api::AccountCreationInfo &info);
    std::shared_ptr<BitcoinLikeAccount> createBitcoinLikeAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                                                 int32_t index,
                                                                 const api::ExtendedKeyAccountCreationInfo& info);

    std::shared_ptr<EthereumLikeAccount> createEthereumLikeAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                                                 int32_t index,
                                                                 const api::AccountCreationInfo &info);
    std::shared_ptr<EthereumLikeAccount> createEthereumLikeAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                                                   int32_t index,
                                                                   const api::ExtendedKeyAccountCreationInfo& info);

    std::shared_ptr<RippleLikeAccount> createRippleLikeAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                                               int32_t index,
                                                               const api::AccountCreationInfo &info);
    std::shared_ptr<RippleLikeAccount> createRippleLikeAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                                               int32_t index,
                                                               const api::ExtendedKeyAccountCreationInfo& info);

    std::shared_ptr<algorand::Account> createAlgorandAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                                               int32_t index,
                                                               const api::AccountCreationInfo &info);
    std::shared_ptr<TezosLikeAccount> createTezosLikeAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                                             int32_t index,
                                                             const api::AccountCreationInfo &info);

    std::shared_ptr<uv::SequentialExecutionContext> getTestExecutionContext();

    void resetDispatcher();

    std::shared_ptr<uv::UvThreadDispatcher> dispatcher;
    std::shared_ptr<NativePathResolver> resolver;
    std::shared_ptr<DatabaseBackend> backend;
    std::shared_ptr<CoutLogPrinter> printer;
    std::shared_ptr<ProxyHttpClient> http;
    std::shared_ptr<FakeWebSocketClient> ws;
    std::shared_ptr<OpenSSLRandomNumberGenerator> rng;
};

#endif //LEDGER_CORE_BASEFIXTURE_H
