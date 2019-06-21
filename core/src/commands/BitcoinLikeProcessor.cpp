#include "BitcoinLikeProcessor.h"
#include <memory>
#include <string>
#include "api/DynamicObject.hpp"
#include "api/Account.hpp"
#include "api/AccountCallback.hpp"
#include "api/Amount.hpp"
#include "api/AmountCallback.hpp"
#include "api/AccountCreationInfo.hpp"
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
#include "wallet/bitcoin/BitcoinLikeAccount.hpp"
#include "wallet/currencies.hpp"
#include <functional>


namespace ledger {
namespace core {
    using namespace message::bitcoin;

    BitcoinLikeCommandProcessor::BitcoinLikeCommandProcessor(const std::shared_ptr<WalletPool>& walletPool)
        : _walletPool(walletPool) {

    }

    Future<std::string> BitcoinLikeCommandProcessor::processRequest(const std::string& request) {
        Request req;
        if (!req.ParseFromString(request)) {
            throw std::runtime_error("Can't parse BitcoinLikeRequest");
        }
        switch (req.type())
        {
        case RequestType::CREATE_ACCOUNT: {
            CreateAccountRequest createAcc;
            if (!createAcc.ParseFromString(req.submessage())) {
                throw std::runtime_error("Can't parse bitcoin CreateAccountRequest");
            }
            return processRequest(createAcc)
                .map<std::string>(_walletPool->getContext(), [](const CreateAccountResponse& createAccResp) {
                    return createAccResp.SerializeAsString();
                });
        }
        case RequestType::SYNC_ACCOUNT: {
            SyncAccountRequest syncAcc;
            if (!syncAcc.ParseFromString(req.submessage())) {
                throw std::runtime_error("Can't parse bitcoin SyncAccountRequest");
            }
            return processRequest(syncAcc)
                .map<std::string>(_walletPool->getContext(), [](const SyncAccountResponse& syncAccResp) {
                    return syncAccResp.SerializeAsString();
                });
        }
        case RequestType::GET_ACCOUNT_BALANCE: {
            GetBalanceRequest getBalance;
            if (!getBalance.ParseFromString(req.submessage())) {
                throw std::runtime_error("Can't parse bitcoin GetBalanceRequest");
            }
            return processRequest(getBalance)
                .map<std::string>(_walletPool->getContext(), [](const GetBalanceResponse& balanceResp) {
                    return balanceResp.SerializeAsString();
                });
        }
        default:
            throw std::runtime_error("Unknown BitcoinLikeRequestType");;
        }
        std::string ans;
        req.SerializeToString(&ans);
        return Future<std::string>::successful(ans);
    }

    Future<CreateAccountResponse> BitcoinLikeCommandProcessor::processRequest(const CreateAccountRequest& req) {
        auto walletConfig = api::DynamicObject::newInstance();
        if (req.config().keychain_engine() == KeychainEngine::BIP49_P2SH) {
            walletConfig->putString(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP49_P2SH);
        }
        else {
            walletConfig->putString(api::Configuration::KEYCHAIN_ENGINE, api::KeychainEngines::BIP32_P2PKH);
        }
        return
        _walletPool->createWallet("wallet1", ledger::core::currencies::BITCOIN.name, walletConfig)
            .flatMap<std::shared_ptr<api::Account>>(_walletPool->getContext(), [req](const std::shared_ptr<AbstractWallet>& wallet) {
                api::ExtendedKeyAccountCreationInfo info;
                info.index = req.index();
                info.extendedKeys.push_back(req.xpub());
                return wallet->newAccountWithExtendedKeyInfo(info);
            })
            .map<CreateAccountResponse>(_walletPool->getContext(), [this](const std::shared_ptr<api::Account> & acc) {
                CreateAccountResponse resp;
                resp.mutable_created_account()->set_index(acc->getIndex());
                resp.mutable_created_account()->set_uid("change_me");
                _accounts["change_me"] = acc;
                return resp;
            });
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
        auto findIt = _accounts.find(req.acc_uid());
        if (findIt == _accounts.end()) {
            throw Exception(api::ErrorCode::RUNTIME_ERROR, "Unknown account");
        }
        std::shared_ptr<api::EventBus> eventBus = findIt->second->synchronize();
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
        eventBus->subscribe(_walletPool->getContext(), eventListener);
        return res->getFuture();
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

    Future<GetBalanceResponse> BitcoinLikeCommandProcessor::processRequest(const GetBalanceRequest& req) {
        auto findIt = _accounts.find(req.acc_uid());
        if (findIt == _accounts.end()) {
            throw Exception(api::ErrorCode::RUNTIME_ERROR, "Unknown account");
        }
        auto res = std::make_shared<Promise<GetBalanceResponse>>();
        auto amountCallback = std::make_shared<AmountCallback>(
            [res](const std::string& amount ) {
                GetBalanceResponse resp;
                message::common::Amount* amnt = resp.mutable_amount();
                amnt->set_value(amount);
                res->complete(resp);
            },
            [res](const api::Error& err) {
                res->failure(core::Exception(err.code, err.message));
            });
        findIt->second->getBalance(amountCallback);
        return res->getFuture();
    }
}
}