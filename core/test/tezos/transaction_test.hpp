#pragma once
#include "Fixtures.hpp"
#include <string>
#include <api/AccountCreationInfo.hpp>
#include <api/DynamicObject.hpp>
#include <api/TezosConfiguration.hpp>
#include <api/TezosConfigurationDefaults.hpp>
#include <wallet/tezos/TezosLikeWallet.h>
#include <wallet/tezos/TezosLikeAccount.h>
#include <wallet/tezos/transaction_builders/TezosLikeTransactionBuilder.h>
#include <wallet/tezos/api_impl/TezosLikeTransactionApi.h>
#include <test/integration/IntegrationEnvironment.h>

#include "../integration/BaseFixture.h"

using namespace ledger::core;

namespace ledger {
    namespace testing {
        namespace tezos {

           

            
            struct TezosMakeBaseTransaction : public BaseFixture {

                void SetUp() override;

                void recreate();

                void TearDown() override;
                            
                struct Callback: public api::StringCallback {
                    Callback(std::shared_ptr<QtThreadDispatcher> dispatcher): _dispatcher(dispatcher)
                    {}
                    virtual void onCallback(const std::experimental::optional<std::string> & result, const std::experimental::optional<api::Error> & error) override {
                        if (result) {
                            std::cout << "broadcastTransaction callback result " << result.value() << std::endl;
                        }
                        if (error) {
                            std::cout << "broadcastTransaction callback error " << error.value().code << "/" <<error.value().message << std::endl;
                        }
                        _dispatcher->stop();
                    }
                    private:
                    std::shared_ptr<QtThreadDispatcher> _dispatcher;
                };

                void broadcast(std::shared_ptr<TezosLikeTransactionApi> tx);

                void broadcast(const std::vector<uint8_t> &raw);

                std::shared_ptr<TezosLikeTransactionBuilder> tx_builder();
                std::shared_ptr<WalletPool> pool;
                std::shared_ptr<AbstractWallet> wallet;
                std::shared_ptr<TezosLikeAccount> account;
                api::Currency currency;
                TransactionTestData testData;

            protected:
                virtual void SetUpConfig() = 0;
            };

        }
    }
}


