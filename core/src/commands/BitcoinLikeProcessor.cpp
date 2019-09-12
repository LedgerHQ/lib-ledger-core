#include "BitcoinLikeProcessor.h"
#include <memory>
#include <string>
#include "api/DynamicObject.hpp"
#include "api/Account.hpp"
#include "api/AccountCallback.hpp"
#include "api/Address.hpp"
#include "api/Amount.hpp"
#include "api/AmountCallback.hpp"
#include "api/AccountCreationInfo.hpp"
#include "api/Block.hpp"
#include "api/BlockCallback.hpp"
#include "api/BitcoinLikeOperation.hpp"
#include "api/Wallet.hpp"
#include "api/WalletCallback.hpp"
#include "api/WalletPool.hpp"
#include "api/Configuration.hpp"
#include "api/KeychainEngines.hpp"
#include "api/Event.hpp"
#include "api/EventCode.hpp"
#include "api/EventBus.hpp"
#include "api/EventReceiver.hpp"
#include "api/ExtendedKeyAccountCreationInfo.hpp"
#include "api/Operation.hpp"
#include "api/OperationQuery.hpp"
#include "api/OperationOrderKey.hpp"
#include "api/OperationType.hpp"
#include "messages/bitcoin/operation.pb.h"
#include "wallet/bitcoin/BitcoinLikeAccount.hpp"
#include "wallet/currencies.hpp"
#include <functional>


namespace ledger {
namespace core {
    using namespace message::bitcoin;

    std::string KeychainToString(message::bitcoin::KeychainEngine keychainEngine) {
        switch (keychainEngine) {
        case KeychainEngine::BIP49_P2SH:
            return api::KeychainEngines::BIP49_P2SH;
        case KeychainEngine::BIP32_P2PKH:
            return api::KeychainEngines::BIP32_P2PKH;
        case KeychainEngine::BIP173_P2WPKH:
            return api::KeychainEngines::BIP173_P2WPKH;
        case KeychainEngine::BIP173_P2WSH:
            return api::KeychainEngines::BIP173_P2WSH;
        default:
            throw std::runtime_error("Can't parse BitcoinLikeRequest");
        }
    }

    std::string AccountIDToString(const message::bitcoin::AccountID& accountID) {
        return accountID.xpub() + ":" + accountID.currency_name() + ":" + KeychainToString(accountID.keychain_engine());
    }

    BitcoinLikeCommandProcessor::BitcoinLikeCommandProcessor(const std::shared_ptr<WalletPool>& walletPool)
        : _walletPool(walletPool) {

    }

    Future<std::shared_ptr<api::Account>> BitcoinLikeCommandProcessor::getOrCreateAccount(const message::bitcoin::AccountID& accountID) {
        auto accountName = AccountIDToString(accountID);
        return _walletPool->getWallet(accountName)
            .recoverWith(_walletPool->getContext(), [walletPool = _walletPool, accountID, accountName](const Exception& ex) {
                if (ex.getErrorCode() != api::ErrorCode::WALLET_NOT_FOUND)
                    throw ex;
                auto walletConfig = DynamicObject::newInstance();
                walletConfig->putString(api::Configuration::KEYCHAIN_ENGINE, KeychainToString(accountID.keychain_engine()));
                return walletPool->createWallet(accountName, accountID.currency_name(), walletConfig);
            })
            .flatMap<std::shared_ptr<api::Account>>(_walletPool->getContext(), [walletPool = _walletPool, xpub = accountID.xpub()](const std::shared_ptr<AbstractWallet>& wallet) {
                return wallet->getAccount(0)
                    .recoverWith(walletPool->getContext(), [xpub, wallet](const Exception& ex) {
                        // create account
                        api::ExtendedKeyAccountCreationInfo info;
                        info.index = 0;
                        info.extendedKeys.push_back(xpub);
                        return wallet->newAccountWithExtendedKeyInfo(info);
                    });
            });
    }

    Future<std::string> BitcoinLikeCommandProcessor::processRequest(const std::string& request) {
        BitcoinRequest req;
        if (!req.ParseFromString(request)) {
            throw std::runtime_error("Can't parse BitcoinLikeRequest");
        }
        switch (req.request_case())
        {
        case BitcoinRequest::RequestCase::kSyncAccount: {
            return processRequest(req.sync_account())
                .map<std::string>(_walletPool->getContext(), [](const SyncAccountResponse& syncAccResponce) {
                    return syncAccResponce.SerializeAsString();
                });
        }
        case BitcoinRequest::RequestCase::kGetBalance: {
            return processRequest(req.get_balance())
                .map<std::string>(_walletPool->getContext(), [](const GetBalanceResponse& balanceResponce) {
                    return balanceResponce.SerializeAsString();
                });
        }
        case BitcoinRequest::RequestCase::kGetOperations: {
            return processRequest(req.get_operations())
                .map<std::string>(_walletPool->getContext(), [](const GetOperationsResponse& getOperationResponce) {
                    return getOperationResponce.SerializeAsString();
                });
        }
        case BitcoinRequest::RequestCase::kGetFreshAddress: {
            return processRequest(req.get_fresh_address())
                .map<std::string>(_walletPool->getContext(), [](const GetFreshAddressResponse& getFreshAddressResponse) {
                    return getFreshAddressResponse.SerializeAsString();
                });
        }
        default:
            throw std::runtime_error("Unknown BitcoinLikeRequestType");;
        }
        std::string ans;
        req.SerializeToString(&ans);
        return Future<std::string>::successful(ans);
    }

    class EventListener : public std::enable_shared_from_this<EventListener> ,public api::EventReceiver {
    public:
        EventListener(std::function<void(const std::shared_ptr<api::Event>&)> onFinish)
            : _onFinish(onFinish) {};

        void onEvent(const std::shared_ptr<api::Event>& incomingEvent) override {
            api::EventCode code = incomingEvent->getCode();
            if (code == api::EventCode::SYNCHRONIZATION_SUCCEED ||
                code == api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT ||
                code == api::EventCode::SYNCHRONIZATION_FAILED) {
                _onFinish(incomingEvent);
            }
        }
    private:
        std::function<void(const std::shared_ptr<api::Event>&)> _onFinish;
    };

    Future<SyncAccountResponse> BitcoinLikeCommandProcessor::processRequest(const SyncAccountRequest& req) {
        return getOrCreateAccount(req.account_id())
            .flatMap<SyncAccountResponse>(_walletPool->getContext(), [walletPool = _walletPool](const std::shared_ptr<api::Account>& account) {
                std::shared_ptr<api::EventBus> eventBus = account->synchronize();
                auto res = std::make_shared<Promise<SyncAccountResponse>>();
                auto eventListener = std::make_shared<EventListener>([res, eventBus](const std::shared_ptr<api::Event>& event) {
                    SyncAccountResponse resp;
                    switch (event->getCode()) {
                    case api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT: {
                        resp.set_new_acc(true);
                        res->complete(resp);
                        return;
                    }
                    case api::EventCode::SYNCHRONIZATION_SUCCEED: {
                        resp.set_new_acc(false);
                        res->complete(resp);
                        return;
                    }
                    default:
                        core::Exception ex(api::ErrorCode::RUNTIME_ERROR, "Synchronization failed");
                        res->failure(ex);
                    }
                    });
                eventBus->subscribe(walletPool->getContext(), eventListener);
                return res->getFuture();
            });
    }

    class AmountCallback: public api::AmountCallback {
    public:
        typedef std::function<void(const std::string&)> OnSuccess;
        typedef std::function<void(const api::Error&)> OnError;

        AmountCallback(OnSuccess onSuccess, OnError onError)
            : _onSuccess(onSuccess)
            , _onError(onError) {};

        void onCallback(const std::shared_ptr<api::Amount>& result, const std::experimental::optional<api::Error>& error) override {
            if (error) {
                _onError(error.value());
                return;
            }
            _onSuccess(result->toString());
        }
    private:
        OnSuccess _onSuccess;
        OnError _onError;
    };

    class BlockCallback : public api::BlockCallback {
    public:
        typedef std::function<void(const api::Block&)> OnSuccess;
        typedef std::function<void(const api::Error&)> OnError;

        BlockCallback(OnSuccess onSuccess, OnError onError)
            : _onSuccess(onSuccess)
            , _onError(onError) {};

        void onCallback(const std::experimental::optional<api::Block>& result, const std::experimental::optional<api::Error>& error) override {
            if (error) {
                _onError(error.value());
                return;
            }
            if (!result) {
                api::Error er(api::ErrorCode::BLOCK_NOT_FOUND, "BlockCallback doesn't get either error nor value");
                _onError(er);
                return;
            }
            _onSuccess(result.value());
        }
    private:
        OnSuccess _onSuccess;
        OnError _onError;
    };

    class AddressListCallback : public api::AddressListCallback {
    public:
        typedef std::function<void(const std::shared_ptr<api::Address>&)> OnSuccess;
        typedef std::function<void(const api::Error&)> OnError;

        AddressListCallback(OnSuccess onSuccess, OnError onError)
            : _onSuccess(onSuccess)
            , _onError(onError) {};

        void onCallback(const std::experimental::optional<std::vector<std::shared_ptr<api::Address>>>& result, const std::experimental::optional<api::Error>& error) override {
            if (error) {
                _onError(error.value());
                return;
            }
            if ((!result) || (result.value().size() == 0)) {
                api::Error er(api::ErrorCode::BLOCK_NOT_FOUND, "AddressListCallback doesn't get either error nor value");
                _onError(er);
                return;
            }
            _onSuccess(result.value()[0]);
        }
    private:
        OnSuccess _onSuccess;
        OnError _onError;
    };

    Future<GetBalanceResponse> BitcoinLikeCommandProcessor::processRequest(const GetBalanceRequest& req) {
        return
            getOrCreateAccount(req.account_id())
                .flatMap<GetBalanceResponse>(_walletPool->getContext(), [=] (const std::shared_ptr<api::Account> & account) {
                    auto res = std::make_shared<Promise<GetBalanceResponse>>();
                    auto amountCallback = std::make_shared<AmountCallback>(
                        [res](const std::string& amount) {
                            GetBalanceResponse resp;
                            message::common::Amount* amnt = resp.mutable_amount();
                            amnt->set_value(amount);
                            std::string x = resp.SerializeAsString();
                            res->complete(resp);
                        },
                        [res](const api::Error& err) {
                            res->failure(core::Exception(err.code, err.message));
                        });
                    account->getBalance(amountCallback);
                    return res->getFuture();
                });
    }

    void convertUnit(const api::CurrencyUnit& unit, message::common::Unit* protoUnit) {
        protoUnit->set_name(unit.name);
        protoUnit->set_magnitude(unit.numberOfDecimal);
    }

    void convertAmount(const std::shared_ptr<api::Amount>& apiAmount, message::common::Amount* protoAmount) {
        protoAmount->set_value(apiAmount->toString());
        convertUnit(apiAmount->getUnit(), protoAmount->mutable_unit());
    }

    void convertOperation(const std::shared_ptr<api::Operation>& apiOperation, message::bitcoin::Operation* protoOperation) {
        convertAmount(apiOperation->getAmount(), protoOperation->mutable_amount());
        for (auto& sender : apiOperation->getSenders()) {
            protoOperation->add_senders(sender);
        }
        for (auto& recipient : apiOperation->getRecipients()) {
            protoOperation->add_receivers(recipient);
        }
        if (apiOperation->getFees())
        convertAmount(apiOperation->getFees(), protoOperation->mutable_fee());
        switch (apiOperation->getOperationType()) {
        case api::OperationType::SEND:
            protoOperation->set_operation_type(message::bitcoin::Operation_OperationType_SEND);
            break;
        case api::OperationType::RECEIVE:
            protoOperation->set_operation_type(message::bitcoin::Operation_OperationType_RECEIVE);
        };
        if (apiOperation->getBlockHeight())
            protoOperation->set_block_height(apiOperation->getBlockHeight().value());
        protoOperation->set_date_epoch_ms(std::chrono::duration_cast<std::chrono::milliseconds>(apiOperation->getDate().time_since_epoch()).count());
        protoOperation->set_transaction_hash(apiOperation->asBitcoinLikeOperation()->getTransaction()->getHash());
    }

    Future<GetOperationsResponse> BitcoinLikeCommandProcessor::processRequest(const message::bitcoin::GetOperationsRequest& req) {
        return 
            getOrCreateAccount(req.account_id())
                .flatMap<std::vector<std::shared_ptr<api::Operation>>>(_walletPool->getContext(), [](const std::shared_ptr<api::Account>& account) {
                    auto completedQuery = account->queryOperations()->complete();
                    completedQuery->addOrder(api::OperationOrderKey::DATE, false);
                    return std::dynamic_pointer_cast<OperationQuery>(completedQuery)->execute();
                })
                .map<GetOperationsResponse>(_walletPool->getContext(), [](const std::vector<std::shared_ptr<api::Operation>>& operations) {
                    GetOperationsResponse response;
                    for (auto& apiOperation : operations) {
                        message::bitcoin::Operation* operation = response.add_operations();
                        convertOperation(apiOperation, operation);
                    }
                    return response;
                });
    }

    Future<GetLastBlockResponse> BitcoinLikeCommandProcessor::processRequest(const GetLastBlockRequest& req) {
        return
            getOrCreateAccount(req.account_id())
            .flatMap<GetLastBlockResponse>(_walletPool->getContext(), [](const std::shared_ptr<api::Account>& account) {
                auto res = std::make_shared<Promise<GetLastBlockResponse>>();
                auto blockCallback = std::make_shared<BlockCallback>(
                    [res](const api::Block& result) {
                        GetLastBlockResponse resp;
                        message::common::Block* block = resp.mutable_last_block();
                        block->set_height(result.height);
                        block->set_hash(result.blockHash);
                        std::string x = resp.SerializeAsString();
                        res->complete(resp);
                    },
                    [res](const api::Error& err) {
                        res->failure(core::Exception(err.code, err.message));
                    });
                account->getLastBlock(blockCallback);
                return res->getFuture();
            });
    }

    Future<GetFreshAddressResponse> BitcoinLikeCommandProcessor::processRequest(const GetFreshAddressRequest& req) {
        return
            getOrCreateAccount(req.account_id())
            .flatMap<GetFreshAddressResponse>(_walletPool->getContext(), [](const std::shared_ptr<api::Account>& account) {
                auto res = std::make_shared<Promise<GetFreshAddressResponse>>();
                auto callback = std::make_shared<AddressListCallback>(
                        [res](const std::shared_ptr<api::Address>& result) {
                            GetFreshAddressResponse resp;
                            resp.set_address(result->toString());
                            auto path = result->getDerivationPath();
                            if (path)
                                resp.set_path(path.value());
                            res->complete(resp);
                        },
                        [res](const api::Error& err) {
                            res->failure(core::Exception(err.code, err.message));
                        });
                account->getFreshPublicAddresses(callback);
                return res->getFuture();
                });
    }
}
}