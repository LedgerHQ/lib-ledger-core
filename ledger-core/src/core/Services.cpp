#include <core/Services.hpp>
#include <core/api/PoolConfiguration.hpp>
#include <core/api/Configuration.hpp>
#include <core/api/ConfigurationDefaults.hpp>
#include <core/debug/LoggerApi.hpp>

namespace ledger {
    namespace core {
        Services::Services(
            const std::string &tenant,
            const std::string &password,
            const std::shared_ptr<api::HttpClient> &httpClient,
            const std::shared_ptr<api::WebSocketClient> &webSocketClient,
            const std::shared_ptr<api::PathResolver> &pathResolver,
            const std::shared_ptr<api::LogPrinter> &logPrinter,
            const std::shared_ptr<api::ThreadDispatcher> &dispatcher,
            const std::shared_ptr<api::RandomNumberGenerator> &rng,
            const std::shared_ptr<api::DatabaseBackend> &backend,
            const std::shared_ptr<api::DynamicObject> &configuration
        ): DedicatedContext(dispatcher->getSerialExecutionContext(fmt::format("pool_queue_{}", tenant))),
           _blockCache(std::chrono::seconds(
                configuration->getInt(api::Configuration::TTL_CACHE)
                    .value_or(api::ConfigurationDefaults::DEFAULT_TTL_CACHE)
           ))
        {
            // General
            _tenant = tenant;

            _configuration = std::static_pointer_cast<DynamicObject>(configuration);

            // File system management
            _pathResolver = pathResolver;

            // HTTP management
            _httpEngine = httpClient;

            // WS management
            _wsClient = std::make_shared<WebSocketClient>(webSocketClient);

            // Preferences management
            _externalPreferencesBackend = std::make_shared<PreferencesBackend>(
                fmt::format("/{}/preferences.db", _tenant),
                getContext(),
                _pathResolver
            );
            _internalPreferencesBackend = std::make_shared<PreferencesBackend>(
                fmt::format("/{}/__preferences__.db", _tenant),
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
                    tenant + "-l",
                    dispatcher->getSerialExecutionContext(fmt::format("logger_queue_{}", tenant)),
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
               Option<std::string>(configuration->getString(api::PoolConfiguration::DATABASE_NAME)).getValueOr(tenant),
               password
            );

            // Threading management
            _threadDispatcher = dispatcher;

            _publisher = std::make_shared<EventPublisher>(getContext());

            _threadPoolExecutionContext = _threadDispatcher->getThreadPoolExecutionContext(fmt::format("pool_{}_thread_pool", tenant));
        }

        std::shared_ptr<Services> Services::newInstance(
            const std::string &tenant,
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
            auto meta = std::shared_ptr<Services>(new Services(
                tenant,
                password,
                httpClient,
                webSocketClient,
                pathResolver,
                logPrinter,
                dispatcher,
                rng,
                backend,
                configuration
            ));

            return meta;
        }

        std::shared_ptr<Preferences> Services::getExternalPreferences() const {
            return _externalPreferencesBackend->getPreferences("pool");
        }

        std::shared_ptr<Preferences> Services::getInternalPreferences() const {
            return _internalPreferencesBackend->getPreferences("pool");
        }

        std::shared_ptr<spdlog::logger> Services::logger() const {
            return _logger;
        }

        std::shared_ptr<DatabaseSessionPool> Services::getDatabaseSessionPool() const {
            return _database;
        }

        std::shared_ptr<DynamicObject> Services::getConfiguration() const {
            return _configuration;
        }

        std::string Services::getTenant() const {
            return _tenant;
        }

        std::string Services::getPassword() const {
            return _password;
        }

        std::shared_ptr<api::PathResolver> Services::getPathResolver() const {
            return _pathResolver;
        }

        std::shared_ptr<api::RandomNumberGenerator> Services::rng() const {
            return _rng;
        }

        std::shared_ptr<api::ThreadDispatcher> Services::getDispatcher() const {
            return _threadDispatcher;
        }

        std::shared_ptr<HttpClient> Services::getHttpClient(const std::string &baseUrl) {
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

        std::shared_ptr<api::EventBus> Services::getEventBus() const {
            return _publisher->getEventBus();
        }

        std::shared_ptr<EventPublisher> Services::getEventPublisher() const {
            return _publisher;
        }

        std::shared_ptr<WebSocketClient> Services::getWebSocketClient() const {
            return _wsClient;
        }

        Future<api::Block> Services::getLastBlock(const std::string &currencyName) {
            auto optBlock = _blockCache.get(currencyName);
            if (optBlock.hasValue()) {
                return Future<api::Block>::successful(optBlock.getValue());
            }
            auto self = shared_from_this();
            return Future<api::Block>::async(_threadPoolExecutionContext, [self, currencyName] () -> api::Block {
                soci::session sql(self->getDatabaseSessionPool()->getPool());
                auto block = BlockDatabaseHelper::getLastBlock(sql, currencyName);
                if (block.isEmpty()) {
                    throw make_exception(api::ErrorCode::BLOCK_NOT_FOUND, "Currency '{}' may not exist", currencyName);
                }
                // Update cache
                self->_blockCache.put(currencyName, block.getValue());
                return block.getValue();
            });
        }

        Future<api::ErrorCode> Services::changePassword(
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

        void Services::changePassword(
            const std::string & oldPassword,
            const std::string & newPassword,
            const std::shared_ptr<api::ErrorCodeCallback> & callback
        ) {
            changePassword(oldPassword, newPassword).callback(_threadDispatcher->getMainExecutionContext(), callback);
        }

        Future<api::ErrorCode> Services::freshResetAll() {
            auto self = shared_from_this();

            return Future<api::ErrorCode>::async(_threadDispatcher->getMainExecutionContext(), [=]() {
                // FIXME: drop coins first

                // drop the main database first
                //self->getDatabaseSessionPool()->performDatabaseMigrationUnsetup();

                // then reset preferences
                _externalPreferencesBackend->clear();
                _internalPreferencesBackend->clear();

                // and we’re done
                return Future<api::ErrorCode>::successful(api::ErrorCode::FUTURE_WAS_SUCCESSFULL);
            });
        }

        Option<api::Block> Services::getBlockFromCache(const std::string &currencyName) {
            return _blockCache.get(currencyName);
        }

        std::shared_ptr<api::ExecutionContext> Services::getThreadPoolExecutionContext() const {
            return _threadPoolExecutionContext;
        }

        std::shared_ptr<api::Logger> Services::getLogger() const {
            return std::make_shared<LoggerApi>(_logger);
        }

        std::shared_ptr<api::Preferences> Services::getPreferences() const {
            return getExternalPreferences();
        }

        void Services::getLastBlock(
            std::string const& currencyName,
            const std::shared_ptr<api::BlockCallback> & callback
        ) {
            getLastBlock(currencyName).callback(_threadDispatcher->getMainExecutionContext(), callback);
        }

        void Services::freshResetAll(
            const std::shared_ptr<api::ErrorCodeCallback> & callback
        ) {
            freshResetAll().callback(_threadDispatcher->getMainExecutionContext(), callback);
        }

        std::shared_ptr<api::Services> api::Services::newInstance(
            const std::string & name,
            const std::string & password,
            const std::shared_ptr<HttpClient> & httpClient,
            const std::shared_ptr<WebSocketClient> & webSocketClient,
            const std::shared_ptr<PathResolver> & pathResolver,
            const std::shared_ptr<LogPrinter> & logPrinter,
            const std::shared_ptr<ThreadDispatcher> & dispatcher,
            const std::shared_ptr<RandomNumberGenerator> & rng,
            const std::shared_ptr<DatabaseBackend> & backend,
            const std::shared_ptr<DynamicObject> & configuration
        ) {
            return ledger::core::Services::newInstance(
                name,
                password,
                httpClient,
                webSocketClient,
                pathResolver,
                logPrinter,
                dispatcher,
                rng,
                backend,
                configuration
            );
        }
    }
}
