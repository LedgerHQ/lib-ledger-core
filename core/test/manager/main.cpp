#include <wallet/bitcoin/transaction_builders/BitcoinLikeTransactionBuilder.h>
#include <api/KeychainEngines.hpp>
#include <api/PoolConfiguration.hpp>
#include <fstream>
#include <async/QtThreadDispatcher.hpp>
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
#include <async/async_wait.h>
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
#include <net/QtHttpClient.hpp>
#include <events/LambdaEventReceiver.hpp>
#include <soci.h>
#include <api/Account.hpp>
#include <api/BitcoinLikeAccount.hpp>
#include <FakeWebSocketClient.h>
#include <OpenSSLRandomNumberGenerator.hpp>
#include <api/ConfigurationDefaults.hpp>
#include <api/ErrorCode.hpp>
#include <utils/FilesystemUtils.h>
#include <utils/hex.h>
#include <QCoreApplication>

using namespace ledger::core; 
using namespace ledger::qt;

struct Parameter {
    std::string key;
    std::string value;
    enum {
        STRING,
        BOOL,
        INT
    } type;
};

class Environment {
public:
    static Environment* initInstance(int argc, char** argv);
    static Environment* getInstance();
    std::string getApplicationDirPath() const;
private:
    Environment(int argc, char** argv);

    std::string _appDir;
    static Environment* _instance;
};

Environment* Environment::_instance = nullptr;

Environment::Environment(int argc, char **argv) {
    QCoreApplication app(argc, argv);
    _appDir = app.applicationDirPath().toStdString();
}

std::string Environment::getApplicationDirPath() const {
    return _appDir;
}

Environment *Environment::initInstance(int argc, char **argv) {
    if (_instance == nullptr) {
        _instance = new Environment(argc, argv);
    }
    return _instance;
}

Environment *Environment::getInstance() {
    return _instance;
}


/////////////////////////////////////////////////////////

class ManagerBase {
public:
    ManagerBase();
    ~ManagerBase();
    std::shared_ptr<WalletPool> newDefaultPool(const std::string &poolName = "",
                                               const std::string &password = "",
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

    std::shared_ptr<AbstractWallet> getOrCreateWallet(std::shared_ptr<WalletPool> pool,
                                                        const std::string &name, 
                                                        const std::string& currencyName,
                                                        const std::shared_ptr<api::DynamicObject> &configuration);
    std::shared_ptr<BitcoinLikeAccount> getOrCreateBitcoinLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, 
                                    int32_t index,
                                    const api::ExtendedKeyAccountCreationInfo &info);

    std::shared_ptr<QtThreadDispatcher> dispatcher;
    std::shared_ptr<NativePathResolver> resolver;
    std::shared_ptr<DatabaseBackend> backend;
    std::shared_ptr<CoutLogPrinter> printer;
    std::shared_ptr<QtHttpClient> http;
    std::shared_ptr<FakeWebSocketClient> ws;
    std::shared_ptr<OpenSSLRandomNumberGenerator> rng;
};

ManagerBase::ManagerBase() {
    ledger::qt::FilesystemUtils::clearFs(Environment::getInstance()->getApplicationDirPath());
    dispatcher = std::make_shared<QtThreadDispatcher>();
    resolver = std::make_shared<NativePathResolver>(Environment::getInstance()->getApplicationDirPath());
    printer = std::make_shared<CoutLogPrinter>(dispatcher->getMainExecutionContext());
    http = std::make_shared<QtHttpClient>(dispatcher->getMainExecutionContext());
    ws = std::make_shared<FakeWebSocketClient>();
    rng = std::make_shared<OpenSSLRandomNumberGenerator>();
}

ManagerBase::~ManagerBase() {
    resolver->clean();
}

std::shared_ptr<WalletPool> ManagerBase::newDefaultPool(const std::string &poolName,
                                                        const std::string &password,
                                                        const std::shared_ptr<api::DynamicObject> &configuration,
                                                        bool usePostgreSQL) {

    backend = std::static_pointer_cast<DatabaseBackend>(usePostgreSQL ?
            DatabaseBackend::getPostgreSQLBackend(api::ConfigurationDefaults::DEFAULT_PG_CONNECTION_POOL_SIZE) : DatabaseBackend::getSqlite3Backend()
    );

    return WalletPool::newInstance(
            poolName,
            password,
            http,
            ws,
            resolver,
            printer,
            dispatcher,
            rng,
            backend,
            configuration,
            nullptr,
            nullptr
    );
}

BitcoinLikeWalletDatabase
ManagerBase::newBitcoinAccount(const std::shared_ptr<WalletPool> &pool,
                               const std::string &walletName,
                               const std::string &currencyName,
                               const std::shared_ptr<api::DynamicObject> &configuration,
                               int32_t index,
                               const std::string &xpub) {
    BitcoinLikeWalletDatabase db(pool, walletName, currencyName);
    if (!db.accountExists(index)) {
        createWallet(pool, walletName, currencyName, configuration);
        createAccount(pool, walletName, index);
        db.createAccount(index, xpub);
    }
    return db;
}

void ManagerBase::createWallet(const std::shared_ptr<WalletPool> &pool,
                               const std::string &walletName,
                               const std::string &currencyName,
                               const std::shared_ptr<api::DynamicObject> &configuration) {
    soci::session sql(pool->getDatabaseSessionPool()
                              ->getPool());
    WalletDatabaseEntry entry;
    entry.configuration = std::static_pointer_cast<DynamicObject>(configuration);
    entry.name = walletName;
    entry.poolName = pool->getName();
    entry.currencyName = currencyName;
    entry.updateUid();
    PoolDatabaseHelper::putWallet(sql, entry);
}

void ManagerBase::createAccount(const std::shared_ptr<WalletPool> &pool, const std::string &walletName, int32_t index) {
    soci::session sql(pool->getDatabaseSessionPool()
                              ->getPool());
    auto walletUid = WalletDatabaseEntry::createWalletUid(pool->getName(), walletName);
    if (!AccountDatabaseHelper::accountExists(sql, walletUid, index))
        AccountDatabaseHelper::createAccount(sql, walletUid, index);
}

std::shared_ptr<BitcoinLikeAccount>
ManagerBase::createBitcoinLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                      const api::AccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<BitcoinLikeAccount>(::wait(wallet->newAccountWithInfo(info)));
}

std::shared_ptr<BitcoinLikeAccount>
ManagerBase::createBitcoinLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                      const api::ExtendedKeyAccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<BitcoinLikeAccount>(::wait(wallet->newAccountWithExtendedKeyInfo(i)));
}

std::shared_ptr<BitcoinLikeAccount>
ManagerBase::getOrCreateBitcoinLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                      const api::ExtendedKeyAccountCreationInfo &info) {
    
    std::shared_ptr<BitcoinLikeAccount> account;
    try {
        account = std::dynamic_pointer_cast<BitcoinLikeAccount>(::wait(wallet->getAccount(index)));
        std::cout << "loading an already created bitcoin account" << std::endl;
    }
    catch(...) {
        auto i = info;
        i.index = index;
        account = std::dynamic_pointer_cast<BitcoinLikeAccount>(::wait(wallet->newAccountWithExtendedKeyInfo(i)));
        std::cout << "creating a new bitcoin account" << std::endl;
    }
    
    return account;
}

std::shared_ptr<AbstractWallet>
ManagerBase::getOrCreateWallet(std::shared_ptr<WalletPool> pool,
                                const std::string &name, 
                                const std::string& currencyName,
                                const std::shared_ptr<api::DynamicObject> &configuration) {
    std::shared_ptr<AbstractWallet> wallet;
    try {
        wallet = wait(pool->getWallet(name));
        std::cout << "loading an already created wallet" << std::endl;
    }
    catch(...) {
        wallet = wait(pool->createWallet(name, currencyName, configuration));
        std::cout << "creating a new wallet" << std::endl;
    }
    
    return wallet;
}

std::shared_ptr<EthereumLikeAccount>
ManagerBase::createEthereumLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                      const api::AccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<EthereumLikeAccount>(::wait(wallet->newAccountWithInfo(info)));
}

std::shared_ptr<EthereumLikeAccount>
ManagerBase::createEthereumLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                      const api::ExtendedKeyAccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<EthereumLikeAccount>(::wait(wallet->newAccountWithExtendedKeyInfo(i)));
}

std::shared_ptr<RippleLikeAccount>
ManagerBase::createRippleLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                       const api::AccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<RippleLikeAccount>(::wait(wallet->newAccountWithInfo(info)));
}

std::shared_ptr<algorand::Account>
ManagerBase::createAlgorandAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                       const api::AccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<algorand::Account>(::wait(wallet->newAccountWithInfo(info)));
}

std::shared_ptr<RippleLikeAccount>
ManagerBase::createRippleLikeAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index,
                                       const api::ExtendedKeyAccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<RippleLikeAccount>(::wait(wallet->newAccountWithExtendedKeyInfo(i)));
}

std::shared_ptr<TezosLikeAccount>
ManagerBase::createTezosLikeAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                    int32_t index,
                                    const api::AccountCreationInfo &info) {
    auto i = info;
    i.index = index;
    return std::dynamic_pointer_cast<TezosLikeAccount>(::wait(wallet->newAccountWithInfo(i)));
}

/////////////////////////////////////////////////////////



class BitcoinLikeWalletSynchronization : public ManagerBase {
    private:
        std::shared_ptr<WalletPool> pool;
        std::shared_ptr<AbstractWallet> wallet;
        std::shared_ptr<BitcoinLikeAccount> account;
        std::shared_ptr<api::DynamicObject> configuration;
    public:
    void load(const std::string& xpub, 
                const std::vector<Parameter>& parameters) {
        configuration = DynamicObject::newInstance();
        #ifdef PG_SUPPORT
            const bool usePostgreSQL = true;
            configuration->putString(api::PoolConfiguration::DATABASE_NAME, "postgres://user:password@localhost:5432/test_db");
            pool = newDefaultPool("user", "password", configuration, usePostgreSQL);
        #else
            pool = newDefaultPool();
        #endif
        for (const auto& param: parameters) {
            configuration->putString(param.key, param.value);
        }
        wallet = getOrCreateWallet(pool, "e847815f-488a-4301-b67c-378a5e9c8a61", "bitcoin", configuration);
        auto index = 0; //wait(wallet->getNextAccountIndex());
        
        api::ExtendedKeyAccountCreationInfo test_info( 
                    0, {"main"}, {"84'/0'/0'"}, {xpub}
            );
        account = getOrCreateBitcoinLikeAccount(wallet, index, test_info);
    }

    void sync(const std::string& xpub, const std::vector<Parameter>& parameters) {
        load (xpub, parameters);
        auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;

                if (event->getCode() != api::EventCode::SYNCHRONIZATION_SUCCEED &&
                    event->getCode() != api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT) {
                        exit(1);
                }
                auto balance = wait(account->getBalance())->toString();
                std::cout << "balance: " << balance << std::endl;

                /*auto ops = wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
                std::cout << "Ops size: " << ops.size() << std::endl;
                auto utxoCount = wait(account->getUTXOCount());
                std::cout << "utxo size: " << utxoCount << std::endl;
                for (auto& op : ops) {
                    std::cout << "op: " << op->asBitcoinLikeOperation()->getTransaction()->getHash() << std::endl;
                    std::cout << " amount: " << op->getAmount()->toLong() << std::endl;
                    std::cout << " type: " << api::to_string(op->getOperationType()) << std::endl;
                }*/
                
                dispatcher->stop();
            });
            account->synchronize()->subscribe(dispatcher->getMainExecutionContext(),receiver);
            dispatcher->waitUntilStopped();
    }

    void balance(const std::string& xpub, const std::vector<Parameter>& parameters) {
        load (xpub, parameters);
        auto balance = wait(account->getBalance())->toString();
        std::cout << "balance: " << balance << std::endl;
    }

    void erase(const std::string& xpub, const std::vector<Parameter>& parameters) {
        load (xpub, parameters);
        auto x = std::chrono::system_clock::from_time_t(0);
        wait(account->eraseDataSince(x));
    }

};

void help() {
    std::cout << "help: ledger-core-manager <command> <conf_file> <xpub>" << std::endl;
    std::cout << "help: <command> list: sync, balance, erase" << std::endl;
}

void readConf(char* config, std::vector<Parameter>& parameters) {
    std::string line;
    std::ifstream infile(config);
    while (std::getline(infile, line)) {
        auto delimiter = line.find('=');
        if (delimiter != std::string::npos) {
            Parameter parameter;
            parameter.key = line.substr(0, delimiter);
            parameter.value = line.substr(delimiter+1, line.size());
            parameters.push_back(parameter);
        }
    }
}

int main(int argc, char **argv) {
	Environment::initInstance(argc, argv);

    if (argc == 4) {
        std::vector<Parameter> parameters;
        readConf(argv[2], parameters);

        if (strcmp(argv[1],"sync") == 0) {
            BitcoinLikeWalletSynchronization bitcoinEngine;
            bitcoinEngine.sync(std::string(argv[3]), parameters);
        }
        else if (strcmp(argv[1],"balance") == 0) {
            BitcoinLikeWalletSynchronization bitcoinEngine;
            bitcoinEngine.balance(std::string(argv[3]), parameters);
        }
        else if (strcmp(argv[1],"erase") == 0) {
            BitcoinLikeWalletSynchronization bitcoinEngine;
            bitcoinEngine.erase(std::string(argv[3]), parameters);
        }
        else {
            help();
        }
    }
    else {
        help();
    }
}
