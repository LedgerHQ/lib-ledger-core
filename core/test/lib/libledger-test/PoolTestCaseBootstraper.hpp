/*
 *
 * PoolTestCaseBootstraper
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/12/2016.
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
#ifndef LEDGER_CORE_POOLTESTCASEBOOTSTRAPER_HPP
#define LEDGER_CORE_POOLTESTCASEBOOTSTRAPER_HPP

#include "NativeThreadDispatcher.hpp"
#include "NativePathResolver.hpp"
#include "CoutLogPrinter.hpp"
#include "MongooseHttpClient.hpp"
#include "../../../lib/cereal/cereal/types/unordered_map.hpp"
#include <ledger/core/api/WalletPool.hpp>
#include <memory>
#include <ledger/core/utils/Exception.hpp>
#include <ledger/core/async/Future.hpp>
#include <ledger/core/api/BitcoinLikeWallet.hpp>
#include <ledger/core/api/BitcoinLikeExtendedPublicKeyProvider.hpp>
#include <ledger/core/api/BitcoinLikeNetworkParameters.hpp>
#include <ledger/core/api/Configuration.hpp>
#include <ledger/core/api/BitcoinLikeWallet.hpp>
#include <ledger/core/api/BitcoinLikeExtendedPublicKeyProvider.hpp>
#include <ledger/core/api/BitcoinLikeBase58ExtendedPublicKeyProvider.hpp>
#include <ledger/core/collections/collections.hpp>
#include <ledger/core/api/Configuration.hpp>
#include <ledger/core/api/DynamicObject.hpp>

class PoolTestCaseBootstraper;
class BootstrapperBase58ExtendedPublicKeyProvider :  public ledger::core::api::BitcoinLikeBase58ExtendedPublicKeyProvider {
public:
    BootstrapperBase58ExtendedPublicKeyProvider(PoolTestCaseBootstraper* self) : _self(self) {};

    void get(const std::string &deviceId, const std::string &path,
             const ledger::core::api::BitcoinLikeNetworkParameters &params,
             const std::shared_ptr<ledger::core::api::StringCompletionBlock> &completion) override;

private:
    PoolTestCaseBootstraper* _self;
};

class PoolTestCaseBootstraper  {
public:
    friend class BootstrapperBase58ExtendedPublicKeyProvider;

    PoolTestCaseBootstraper(const std::string &poolName);
    void setup(std::function<void (std::shared_ptr<ledger::core::api::WalletPool>, std::experimental::optional<ledger::core::api::Error>)> callback);
    void tearDown();

    ledger::core::Future<std::shared_ptr<ledger::core::api::BitcoinLikeWallet>>
    getBitcoinLikeWallet( const std::shared_ptr<ledger::core::api::BitcoinLikeExtendedPublicKeyProvider> &publicKeyProvider,
                          const ledger::core::api::BitcoinLikeNetworkParameters &networkParams,
                          const std::shared_ptr<ledger::core::api::DynamicObject> &configuration);

    ledger::core::Future<std::shared_ptr<ledger::core::api::BitcoinLikeWallet>>
    getBitcoinWallet(const std::shared_ptr<ledger::core::api::DynamicObject>& configuration = ledger::core::api::DynamicObject::newInstance());

    PoolTestCaseBootstraper& xpubs(const ledger::core::Map<std::string, std::string>& xpubs);

public:
    std::shared_ptr<NativeThreadDispatcher> dispatcher;
    std::shared_ptr<NativePathResolver> resolver;
    std::shared_ptr<CoutLogPrinter> printer;
    std::shared_ptr<MongooseHttpClient> client;
    std::shared_ptr<ledger::core::api::ExecutionContext> mainContext;
    std::shared_ptr<ledger::core::api::ExecutionContext> workerContext;

private:
    std::string _poolName;
    ledger::core::Map<std::string, std::string> _xpubs;
};

#endif //LEDGER_CORE_POOLTESTCASEBOOTSTRAPER_HPP
