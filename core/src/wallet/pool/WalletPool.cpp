/*
 *
 * WalletPool
 * ledger-core
 *
 * Created by Pierre Pollastri on 10/05/2017.
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
#include "WalletPool.hpp"
#include <api/PoolConfiguration.hpp>
#include <wallet/currencies.hpp>
#include <wallet/ethereum/ERC20/erc20Tokens.h>
#include <wallet/pool/database/CurrenciesDatabaseHelper.hpp>
#include <wallet/pool/database/PoolDatabaseHelper.hpp>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <database/soci-date.h>
#include <bitcoin/bech32/Bech32Parameters.h>

namespace ledger {
    namespace core {
        WalletPool::WalletPool(
            const std::string &name,
            const std::string &password,
            const std::shared_ptr<api::HttpClient> &httpClient,
            const std::shared_ptr<api::WebSocketClient> &webSocketClient,
            const std::shared_ptr<api::PathResolver> &pathResolver,
            const std::shared_ptr<api::LogPrinter> &logPrinter,
            const std::shared_ptr<api::ThreadDispatcher> &dispatcher,
            const std::shared_ptr<api::RandomNumberGenerator> &rng,
            const std::shared_ptr<api::DatabaseBackend> &backend,
            const std::shared_ptr<api::DynamicObject> &configuration
        ): DedicatedContext(dispatcher->getSerialExecutionContext(fmt::format("pool_queue_{}", name))) {
            // General
            _poolName = name;

            _configuration = std::static_pointer_cast<DynamicObject>(configuration);

            // File system management
            _pathResolver = pathResolver;

            // HTTP management
            _httpEngine = httpClient;

            // WS management
            _wsClient = std::make_shared<WebSocketClient>(webSocketClient);

            // Preferences management
            _externalPreferencesBackend = std::make_shared<PreferencesBackend>(
                fmt::format("/{}/preferences.db", _poolName),
                getContext(),
                _pathResolver
            );
            _internalPreferencesBackend = std::make_shared<PreferencesBackend>(
                fmt::format("/{}/__preferences__.db", _poolName),
                getContext(),
                _pathResolver
            );

            _rng = rng;
            // Encrypt the preferences, if needed
            _password = password;
            if (!_password.empty()) {
                _externalPreferencesBackend->setEncryption(_rng, _password);
                _internalPreferencesBackend->setEncryption(_rng, _password);
            }

            // Logger management
            _logPrinter = logPrinter;
            auto enableLogger = _configuration->getBoolean(api::PoolConfiguration::ENABLE_INTERNAL_LOGGING).value_or(true);
            _logger = logger::create(
                    name + "-l",
                    dispatcher->getSerialExecutionContext(fmt::format("logger_queue_{}", name)),
                    pathResolver,
                    logPrinter,
                    logger::DEFAULT_MAX_SIZE,
                    enableLogger
            );

            // Database management
            _database = std::make_shared<DatabaseSessionPool>(
               std::static_pointer_cast<DatabaseBackend>(backend),
               pathResolver,
               _logger,
               Option<std::string>(configuration->getString(api::PoolConfiguration::DATABASE_NAME)).getValueOr(name),
               password
            );

            // Threading management
            _threadDispatcher = dispatcher;

            _publisher = std::make_shared<EventPublisher>(getContext());

            _threadPoolExecutionContext = _threadDispatcher->getThreadPoolExecutionContext(fmt::format("pool_{}_thread_pool", name));
        }

        std::shared_ptr<WalletPool>
        WalletPool::newInstance(
            const std::string &name,
            const std::string &password,
            const std::shared_ptr<api::HttpClient> &httpClient,
            const std::shared_ptr<api::WebSocketClient> &webSocketClient,
            const std::shared_ptr<api::PathResolver> &pathResolver,
            const std::shared_ptr<api::LogPrinter> &logPrinter,
            const std::shared_ptr<api::ThreadDispatcher> &dispatcher,
            const std::shared_ptr<api::RandomNumberGenerator> &rng,
            const std::shared_ptr<api::DatabaseBackend> &backend,
            const std::shared_ptr<api::DynamicObject> &configuration
        ) {
            auto pool = std::shared_ptr<WalletPool>(new WalletPool(
                name, password, httpClient, webSocketClient, pathResolver, logPrinter, dispatcher, rng, backend, configuration
            ));

            // Initialization
            //  Load currencies
            pool->initializeCurrencies();

            //  Create factories
            pool->initializeFactories();

            return pool;
        }

        void WalletPool::initializeCurrencies() {
            soci::session sql(getDatabaseSessionPool()->getPool());
            sql.begin();
            PoolDatabaseHelper::insertPool(sql, *this);
            for (auto& currency : currencies::ALL) {
                CurrenciesDatabaseHelper::insertCurrency(sql, currency);
            }
            //Init erc20 tokens
            for (auto& erc20Token : erc20Tokens::ALL_ERC20) {
                CurrenciesDatabaseHelper::insertERC20Token(sql, erc20Token.second);
            }
            //Init bech32 params
            for (auto& bech32Params : Bech32Parameters::ALL) {
                Bech32Parameters::insertParameters(sql, bech32Params);
            }
            sql.commit();
            CurrenciesDatabaseHelper::getAllCurrencies(sql, _currencies);
        }

        void WalletPool::initializeFactories() {
            for (const auto& currency : _currencies) {
               createFactory(currency);
            }
        }

        void WalletPool::createFactory(const api::Currency &currency) {
            switch (currency.walletType) {
                case api::WalletType::BITCOIN:
                    _factories.push_back(make_factory<api::WalletType::BITCOIN>(currency, shared_from_this()));
                    break;
                case api::WalletType::ETHEREUM:
                    _factories.push_back(make_factory<api::WalletType::ETHEREUM>(currency, shared_from_this()));
                    break;
                case api::WalletType::RIPPLE:
                    _factories.push_back(make_factory<api::WalletType::RIPPLE>(currency, shared_from_this()));
                    break;
                case api::WalletType::MONERO:
                    _factories.push_back(make_factory<api::WalletType::MONERO>(currency, shared_from_this()));
                    break;
                case api::WalletType::TEZOS:
                    _factories.push_back(make_factory<api::WalletType::TEZOS>(currency, shared_from_this()));
                    break;
            }
        }

        std::shared_ptr<Preferences> WalletPool::getExternalPreferences() const {
            return _externalPreferencesBackend->getPreferences("pool");
        }

        std::shared_ptr<Preferences> WalletPool::getInternalPreferences() const {
            return _internalPreferencesBackend->getPreferences("pool");
        }

        std::shared_ptr<spdlog::logger> WalletPool::logger() const {
            return _logger;
        }

        std::shared_ptr<DatabaseSessionPool> WalletPool::getDatabaseSessionPool() const {
            return _database;
        }

        std::shared_ptr<DynamicObject> WalletPool::getConfiguration() const {
            return _configuration;
        }

        const std::string &WalletPool::getName() const {
            return _poolName;
        }

        const std::string WalletPool::getPassword() const {
            return _password;
        }

        std::shared_ptr<api::PathResolver> WalletPool::getPathResolver() const {
            return _pathResolver;
        }

        std::shared_ptr<api::RandomNumberGenerator> WalletPool::rng() const {
            return _rng;
        }

        std::shared_ptr<api::ThreadDispatcher> WalletPool::getDispatcher() const {
            return _threadDispatcher;
        }

        std::shared_ptr<HttpClient> WalletPool::getHttpClient(const std::string &baseUrl) {
            auto it = _httpClients.find(baseUrl);
            if (it == _httpClients.end() || !it->second.lock()) {
                auto client = std::make_shared<HttpClient>(
                    baseUrl,
                    _httpEngine,
                    getDispatcher()->getMainExecutionContext()
                );
                _httpClients[baseUrl] = client;
                client->setLogger(logger());
                return client;
            }
            auto client = _httpClients[baseUrl].lock();
            if (!client) {
                throw make_exception(api::ErrorCode::NULL_POINTER, "HttpClient was released.");
            }
            return client;
        }

        const std::vector<api::Currency> &WalletPool::getCurrencies() const {
            return _currencies;
        }

        std::shared_ptr<AbstractWalletFactory> WalletPool::getFactory(const std::string &currencyName) const {
            for (auto& factory : _factories) {
                if (factory->getCurrency().name == currencyName) {
                    return factory;
                }
            }
            return nullptr;
        }

        Future<int64_t> WalletPool::getWalletCount() const {
            auto self = shared_from_this();
            return async<int64_t>([=] () -> int64_t {
                soci::session sql(self->getDatabaseSessionPool()->getPool());
                return PoolDatabaseHelper::getWalletCount(sql, *self);
            });
        }

        Future<std::vector<std::string>> WalletPool::getWalletNames(int64_t from, int64_t size) const {
            auto self = shared_from_this();
            return async<std::vector<std::string>>([=] () -> std::vector<std::string> {
                soci::session sql(self->getDatabaseSessionPool()->getPool());
                std::vector<WalletDatabaseEntry> entries((size_t) size);
                auto count = PoolDatabaseHelper::getWallets(sql, *self, from, entries);
                std::vector<std::string> names((size_t) count);
                auto index = 0;
                for (const auto& entry : entries) {
                    names[index] = entry.name;
                    index += 1;
                }
                return names;
            });
        }

        Option<WalletDatabaseEntry> WalletPool::getWalletEntryFromDatabase(const std::shared_ptr<WalletPool> &walletPool,
                                                                           const std::string &name) {
            WalletDatabaseEntry entry;
            soci::session sql(walletPool->getDatabaseSessionPool()->getPool());
            if (!PoolDatabaseHelper::getWallet(sql, *walletPool, name, entry)) {
                return Option<WalletDatabaseEntry>();
            }
            return Option<WalletDatabaseEntry>(entry);
        }

        FuturePtr<AbstractWallet> WalletPool::getWallet(const std::string &name) {
            auto it = _wallets.find(WalletDatabaseEntry::createWalletUid(getName(), name));
            if (it != _wallets.end()) {
                auto ptr = it->second;
                if (ptr != nullptr) {
                    return FuturePtr<AbstractWallet>::successful(ptr);
                }
            }
            auto self = shared_from_this();
            return Future<std::shared_ptr<AbstractWallet>>::async(_threadDispatcher->getMainExecutionContext(), [=] () {
                auto it = self->_wallets.find(WalletDatabaseEntry::createWalletUid(self->getName(), name));
                auto entry = getWalletEntryFromDatabase(self, name);
                if (!entry.hasValue()) {
                    throw Exception(api::ErrorCode::WALLET_NOT_FOUND, fmt::format("Wallet '{}' doesn't exist.", name));
                }
                return self->buildWallet(entry.getValue());
            });
        }

        Future<api::ErrorCode> WalletPool::updateWalletConfig(const std::string &name,
                                                              const std::shared_ptr<api::DynamicObject> &configuration) {
            auto self = shared_from_this();
            return async<api::ErrorCode>([=] () {
                auto entry = getWalletEntryFromDatabase(self, name);
                if (!entry.hasValue()) {
                    return api::ErrorCode::INVALID_ARGUMENT;
                }
                // Wallet exists, let's update its configuration
                auto walletEntry = entry.getValue();
                walletEntry.configuration->updateWithConfiguration(std::static_pointer_cast<ledger::core::DynamicObject>(configuration));
                soci::session sql(self->getDatabaseSessionPool()->getPool());
                PoolDatabaseHelper::putWallet(sql, walletEntry);
                // No need to check if currency supported (factory non null), because we are supposed to fetch
                // walletEntry from database, which implies that it was already checked before at creation
                self->_wallets[walletEntry.uid] = self->getFactory(walletEntry.currencyName)->build(walletEntry);
                return api::ErrorCode::FUTURE_WAS_SUCCESSFULL;
            });
        }

        std::shared_ptr<AbstractWallet> WalletPool::buildWallet(const WalletDatabaseEntry &entry) {
            // Check if the wallet already exists
            auto it = _wallets.find(entry.uid);
            if (it != _wallets.end()) {
                return it->second;
            } else {
                auto factory = getFactory(entry.currencyName);
                if (factory == nullptr) {
                    throw Exception(api::ErrorCode::UNSUPPORTED_CURRENCY,
                                    fmt::format("Wallet '{}' uses an unsupported currency ''{}", entry.name,
                                                entry.currencyName)
                    );
                }
                auto wallet = factory->build(entry);
                _publisher->relay(wallet->getEventBus());
                _wallets[entry.uid] = wallet;
                std::weak_ptr<WalletPool> weakSelf = shared_from_this();
                _publisher->setFilter([weakSelf] (const std::shared_ptr<api::Event>& event) -> bool {
                    auto self = weakSelf.lock();
                    if (self && event->getCode() == api::EventCode::NEW_BLOCK) {
                        std::lock_guard<std::mutex> lock(self->_eventFilterMutex);
                        auto height = event->getPayload()->getLong(api::Account::EV_NEW_BLOCK_HEIGHT);
                        auto currency = event->getPayload()->getString(api::Account::EV_NEW_BLOCK_CURRENCY_NAME);
                        auto lastBlockEmitted = self->_lastEmittedBlocks.find(currency.value());
                        if (lastBlockEmitted != self->_lastEmittedBlocks.end() && height > lastBlockEmitted->second) {
                            self->_lastEmittedBlocks[currency.value()] = height.value();
                            return true;
                        } else {
                            return false;
                        }
                    }
                    return true;
                });
                return wallet;
            }
        }

        Future<std::vector<std::shared_ptr<AbstractWallet>>> WalletPool::getWallets(int64_t from, int64_t size) {
            auto self = shared_from_this();
            return Future<std::vector<std::shared_ptr<AbstractWallet>>>::async(_threadPoolExecutionContext, [=] () {
                std::vector<WalletDatabaseEntry> entries((size_t) size);
                soci::session sql(self->getDatabaseSessionPool()->getPool());
                auto count = PoolDatabaseHelper::getWallets(sql, *self, from, entries);
                std::vector<std::shared_ptr<AbstractWallet>> wallets((size_t) count);
                for (auto i = 0; i < count; i++) {
                    wallets[i] = self->buildWallet(entries[i]);
                }
                return wallets;
            });
        }

        Future<Unit> WalletPool::addCurrency(const api::Currency &currency) {
            auto self = shared_from_this();
            return async<Unit>([=] () {
                auto factory = getFactory(currency.name);
                if (factory != nullptr) {
                    throw Exception(api::ErrorCode::CURRENCY_ALREADY_EXISTS, fmt::format("Currency '{}' already exists.", currency.name));
                }
                soci::session sql(self->getDatabaseSessionPool()->getPool());
                CurrenciesDatabaseHelper::insertCurrency(sql, currency);
                _currencies.push_back(currency);
                createFactory(currency);
                return unit;
            });
        }

        Future<Unit> WalletPool::removeCurrency(const std::string &currencyName) {
            auto self = shared_from_this();
            return async<Unit>([=] () {
                auto factory = getFactory(currencyName);
                if (factory == nullptr) {
                    throw Exception(api::ErrorCode::CURRENCY_NOT_FOUND, fmt::format("Currency '{}' not found.", currencyName));
                }
                soci::session sql(self->getDatabaseSessionPool()->getPool());
                CurrenciesDatabaseHelper::removeCurrency(sql, currencyName);
                auto cIt = std::find_if(_currencies.begin(), _currencies.end(), [currencyName] (const api::Currency& currency) {
                   return currencyName == currency.name;
                });
                auto fIt = std::find_if(_factories.begin(), _factories.end(), [currencyName] (const std::shared_ptr<AbstractWalletFactory>& f) {
                    return currencyName == f->getCurrency().name;
                });
                if (cIt != _currencies.end())
                    _currencies.erase(cIt);
                if (fIt != _factories.end())
                    _factories.erase(fIt);
                return unit;
            });
        }

        FuturePtr<AbstractWallet> WalletPool::createWallet(const std::string &name, const std::string& currencyName,
                                                           const std::shared_ptr<api::DynamicObject> &configuration) {
            auto self = shared_from_this();
            return async<std::shared_ptr<AbstractWallet>>([=] () {
                auto factory = self->getFactory(currencyName);
                if (factory == nullptr) {
                    throw make_exception(api::ErrorCode::CURRENCY_NOT_FOUND, "Currency '{}' not found.");
                }
                // Create the entry
                soci::session sql(self->getDatabaseSessionPool()->getPool());
                soci::transaction tr(sql);

                WalletDatabaseEntry entry;
                entry.name = name;
                entry.configuration = std::static_pointer_cast<ledger::core::DynamicObject>(configuration);
                entry.currencyName = currencyName;
                entry.poolName = self->getName();
                entry.uid = WalletDatabaseEntry::createWalletUid(self->getName(), name);
                if (PoolDatabaseHelper::walletExists(sql, entry))
                    throw make_exception(api::ErrorCode::WALLET_ALREADY_EXISTS, "Wallet '{}' for currency '{}' already exists", name, currencyName);
                PoolDatabaseHelper::putWallet(sql, entry);
                auto wallet = buildWallet(entry);
                tr.commit();
                return wallet;
            });
        }

        Option<api::Currency> WalletPool::getCurrency(const std::string &name) const {
            for (auto& currency : _currencies) {
                if (currency.name == name)
                    return Option<api::Currency>(currency);
            }
            return Option<api::Currency>();
        }

        std::shared_ptr<api::EventBus> WalletPool::getEventBus() const {
            return _publisher->getEventBus();
        }

        std::shared_ptr<WebSocketClient> WalletPool::getWebSocketClient() const {
            return _wsClient;
        }

        Future<api::Block> WalletPool::getLastBlock(const std::string &currencyName) {
            auto self = shared_from_this();
            return Future<api::Block>::async(_threadPoolExecutionContext, [self, currencyName] () -> api::Block {
                soci::session sql(self->getDatabaseSessionPool()->getPool());
                auto block = BlockDatabaseHelper::getLastBlock(sql, currencyName);
                if (block.isEmpty()) {
                    throw make_exception(api::ErrorCode::BLOCK_NOT_FOUND, "Currency '{}' may not exist", currencyName);
                }
                return block.getValue();
            });
        }

        Future<api::ErrorCode> WalletPool::eraseDataSince(const std::chrono::system_clock::time_point & date) {
            auto self = shared_from_this();
            auto name = getName();
            _logger->debug("Start erasing data of WalletPool : {}",name);

            return getWalletCount().flatMap<std::vector<std::shared_ptr<AbstractWallet>>>(getContext(), [self] (int64_t count) {
                return self->getWallets(0, count);
            }).flatMap<api::ErrorCode>(getContext(), [self, date] (const std::vector<std::shared_ptr<AbstractWallet>> &wallets) -> Future<api::ErrorCode>{

                static std::function<Future<api::ErrorCode> (int, const std::vector<std::shared_ptr<AbstractWallet>> &)>  eraseWallet = [date] (int index, const std::vector<std::shared_ptr<AbstractWallet>> &walletsToErase) -> Future<api::ErrorCode> {

                    if (index == walletsToErase.size()) {
                        return Future<api::ErrorCode>::successful(api::ErrorCode::FUTURE_WAS_SUCCESSFULL);
                    }

                    return walletsToErase[index]->eraseDataSince(date).flatMap<api::ErrorCode>(ImmediateExecutionContext::INSTANCE,[date, index, walletsToErase] (const api::ErrorCode &errorCode) -> Future<api::ErrorCode> {

                        if (errorCode != api::ErrorCode::FUTURE_WAS_SUCCESSFULL) {
                            return Future<api::ErrorCode>::failure(make_exception(api::ErrorCode::RUNTIME_ERROR, "Failed to erase wallets of WalletPool !"));
                        }

                        return eraseWallet(index + 1, walletsToErase);
                    });
                };
                return eraseWallet(0, wallets);

            }).flatMap<api::ErrorCode>(getContext(), [self, name, date] (const api::ErrorCode &err) {

                if (err != api::ErrorCode::FUTURE_WAS_SUCCESSFULL) {
                    return Future<api::ErrorCode>::failure(make_exception(api::ErrorCode::RUNTIME_ERROR, "Failed to erase wallets of WalletPool !"));
                }

                //Erase wallets created after date
                soci::session sql(self->getDatabaseSessionPool()->getPool());
                soci::rowset<soci::row> wallets = (sql.prepare << "SELECT uid FROM wallets "
                                                                    "WHERE pool_name = :pool_name AND created_at >= :date ",
                                                                    soci::use(name), soci::use(date));
                for (auto& wallet : wallets) {
                    if (wallet.get_indicator(0) != soci::i_null) {
                        self->_wallets.erase(wallet.get<std::string>(0));
                    }
                }
                sql << "DELETE FROM wallets WHERE pool_name = :pool_name AND created_at >= :date ", soci::use(name), soci::use(date);
                self->logger()->debug("Finish erasing data of WalletPool : {}",name);
                return Future<api::ErrorCode>::successful(api::ErrorCode::FUTURE_WAS_SUCCESSFULL);
            });
        }

        Future<api::ErrorCode> WalletPool::changePassword(
            const std::string& oldPassword,
            const std::string& newPassword
        ) {
            auto self = shared_from_this();

            return async<api::ErrorCode>([=]() {
                self->getDatabaseSessionPool()->performChangePassword(oldPassword, newPassword);
                self->_externalPreferencesBackend->resetEncryption(_rng, oldPassword, newPassword);
                self->_internalPreferencesBackend->resetEncryption(_rng, oldPassword, newPassword);
                return api::ErrorCode::FUTURE_WAS_SUCCESSFULL;
            });
        }

        Future<api::ErrorCode> WalletPool::freshResetAll() {
            auto self = shared_from_this();

            return Future<api::ErrorCode>::async(_threadDispatcher->getMainExecutionContext(), [=]() {
                // drop the main database first
                self->getDatabaseSessionPool()->performDatabaseRollback();

                // then reset preferences
                _externalPreferencesBackend->clear();
                _internalPreferencesBackend->clear();

                // and we’re done
                return Future<api::ErrorCode>::successful(api::ErrorCode::FUTURE_WAS_SUCCESSFULL);
            });
        }

        std::shared_ptr<api::ExecutionContext> WalletPool::getThreadPoolExecutionContext() const {
            return _threadPoolExecutionContext;
        }
    }
}
