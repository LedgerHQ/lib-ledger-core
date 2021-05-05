#include <gtest/gtest.h>
#include "../integration/IntegrationEnvironment.h"
#include "../integration/BaseFixture.h"
#include <wallet/bitcoin/transaction_builders/BitcoinLikeTransactionBuilder.h>
#include <api/KeychainEngines.hpp>
#include <api/PoolConfiguration.hpp>
#include <fstream>

struct Parameter {
    std::string key;
    std::string value;
    enum {
        STRING,
        BOOL,
        INT
    } type;
};

class BitcoinLikeWalletSynchronization : public BaseFixture {
    public:
    virtual void TestBody() {}
    void sync(const std::string& xpub, const std::vector<Parameter>& parameters) {
        auto configuration = DynamicObject::newInstance();
        #ifdef PG_SUPPORT
            const bool usePostgreSQL = true;
            configuration->putString(api::PoolConfiguration::DATABASE_NAME, "postgres://user:password@localhost:5432/test_db");
            auto pool = newDefaultPool("user", "password", configuration, usePostgreSQL);
        #else
            auto pool = newDefaultPool();
        #endif
        for (const auto& param: parameters) {
            configuration->putString(param.key, param.value);
        }
        auto wallet = wait(pool->createWallet("e847815f-488a-4301-b67c-378a5e9c8a61", "bitcoin",
                                              configuration));
        auto nextIndex = wait(wallet->getNextAccountIndex());
            EXPECT_EQ(nextIndex, 0);
        
        api::ExtendedKeyAccountCreationInfo test_info( 
                    0, {"main"}, {"84'/0'/0'"}, {xpub}
            );
        auto account = createBitcoinLikeAccount(wallet, nextIndex, test_info);

        auto receiver = make_receiver([=](const std::shared_ptr<api::Event> &event) {
                fmt::print("Received event {}\n", api::to_string(event->getCode()));
                if (event->getCode() == api::EventCode::SYNCHRONIZATION_STARTED)
                    return;
                EXPECT_NE(event->getCode(), api::EventCode::SYNCHRONIZATION_FAILED);

                bool successed = (event->getCode() == api::EventCode::SYNCHRONIZATION_SUCCEED) ||
                    (event->getCode() == api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT);
                EXPECT_EQ(successed, true);
                auto balance = wait(account->getBalance())->toString();
                std::cout << "balance: " << balance << std::endl;
                auto ops = wait(std::dynamic_pointer_cast<OperationQuery>(account->queryOperations()->complete())->execute());
                std::cout << "Ops size: " << ops.size() << std::endl;
                auto utxoCount = wait(account->getUTXOCount());
                std::cout << "utxo size: " << utxoCount << std::endl;
                /*for (auto& op : ops) {
                    std::cout << "op: " << op->asBitcoinLikeOperation()->getTransaction()->getHash() << std::endl;
                    std::cout << " amount: " << op->getAmount()->toLong() << std::endl;
                    std::cout << " type: " << api::to_string(op->getOperationType()) << std::endl;
                }*/
                
                dispatcher->stop();
            });
            account->synchronize()->subscribe(dispatcher->getMainExecutionContext(),receiver);
            dispatcher->waitUntilStopped();
    }
};

void help() {
    std::cout << "help: ledger-core-manager <command> <conf_file> <xpub>" << std::endl;
    std::cout << "help: <command> list: sync" << std::endl;
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
	IntegrationEnvironment::initInstance(argc, argv);

    if (argc == 4) {
        std::vector<Parameter> parameters;
        readConf(argv[2], parameters);

        if (strcmp(argv[1],"sync") == 0) {
            BitcoinLikeWalletSynchronization bitcoinEngine;
            bitcoinEngine.SetUp();
            bitcoinEngine.sync(std::string(argv[3]), parameters);
            bitcoinEngine.TearDown();
        }
        else {
            help();
        }
    }
    else {
        help();
    }
}
