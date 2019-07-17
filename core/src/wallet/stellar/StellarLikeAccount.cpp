/*
 *
 * StellarLikeAccount.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/02/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#include "StellarLikeAccount.hpp"
#include <wallet/stellar/StellarLikeWallet.hpp>
#include <utils/DateUtils.hpp>
#include <events/Event.hpp>
#include "database/StellarLikeAccountDatabaseHelper.hpp"
#include <wallet/common/database/AccountDatabaseHelper.h>
#include "database/StellarLikeLedgerDatabaseHelper.hpp"
#include "database/StellarLikeTransactionDatabaseHelper.hpp"
#include <wallet/common/database/OperationDatabaseHelper.h>
#include <set>

using namespace ledger::core;

static const std::set<stellar::OperationType> ACCEPTED_PAYMENT_TYPES {
    stellar::OperationType::PAYMENT, stellar::OperationType::CREATE_ACCOUNT
};

namespace ledger {
    namespace core {

        StellarLikeAccount::StellarLikeAccount(const std::shared_ptr<StellarLikeWallet> &wallet,
                                               const StellarLikeAccountParams &params)
                                               : AbstractAccount(wallet, params.index), _params(params) {

        }

        bool StellarLikeAccount::isSynchronizing() {
            return _params.synchronizer->isSynchronizing();
        }

        std::shared_ptr<api::EventBus> StellarLikeAccount::synchronize() {
            std::lock_guard<std::mutex> lock(_synchronizationLock);

            if (_currentSyncEventBus != nullptr)
                return _currentSyncEventBus;

            auto eventPublisher = std::make_shared<EventPublisher>(getContext());

            _currentSyncEventBus = eventPublisher->getEventBus();
            auto self = std::dynamic_pointer_cast<StellarLikeAccount>(shared_from_this());
            auto synchronizer = _params.synchronizer;
            auto future = synchronizer->synchronize(self)->getFuture();

            //Update current block height (needed to compute trust level)
            _params.explorer->getLastLedger().onComplete(getContext(),
                                                    [self](const TryPtr<stellar::Ledger> &l) mutable {
                                                        if (l.isSuccess()) {
                                                            self->_currentLedgerHeight = l.getValue()->height;
                                                        }
                                                    });

            auto startTime = DateUtils::now();
            eventPublisher->postSticky(
                    std::make_shared<Event>(api::EventCode::SYNCHRONIZATION_STARTED, api::DynamicObject::newInstance()),
                    0);
            future.onComplete(getContext(), [eventPublisher, self, startTime](const Try<Unit> &result) {
                api::EventCode code;
                auto payload = std::make_shared<DynamicObject>();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(
                        DateUtils::now() - startTime).count();
                payload->putLong(api::Account::EV_SYNC_DURATION_MS, duration);
                if (result.isSuccess()) {
                    code = api::EventCode::SYNCHRONIZATION_SUCCEED;
                } else {
                    code = api::EventCode::SYNCHRONIZATION_FAILED;
                    payload->putString(api::Account::EV_SYNC_ERROR_CODE,
                                       api::to_string(result.getFailure().getErrorCode()));
                    payload->putInt(api::Account::EV_SYNC_ERROR_CODE_INT, (int32_t) result.getFailure().getErrorCode());
                    payload->putString(api::Account::EV_SYNC_ERROR_MESSAGE, result.getFailure().getMessage());
                }
                eventPublisher->postSticky(std::make_shared<Event>(code, payload), 0);
                std::lock_guard<std::mutex> lock(self->_synchronizationLock);
                self->_currentSyncEventBus = nullptr;

            });
            return eventPublisher->getEventBus();
        }

        void StellarLikeAccount::startBlockchainObservation() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        void StellarLikeAccount::stopBlockchainObservation() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        bool StellarLikeAccount::isObservingBlockchain() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        std::string StellarLikeAccount::getRestoreKey() {
            return _params.keychain->getRestoreKey();
        }

        FuturePtr<ledger::core::Amount> StellarLikeAccount::getBalance() {
            auto self = getSelf();
            return async<std::shared_ptr<Amount>>([=] () {
                const auto& currency = self->getWallet()->getCurrency();
                soci::session sql(self->_params.database->getPool());
                stellar::Account account {};

                auto accountId = AccountDatabaseHelper::createAccountUid(self->getWallet()->getWalletUid(), self->_params.index);
                StellarLikeAccountDatabaseHelper::getAccount(sql, accountId, account);
                StellarLikeAccountDatabaseHelper::getAccountBalances(sql, accountId, account);
                BigInt amount;

                for (const auto& balance : account.balances) {
                    if (balance.assetType == "native") {
                        amount = balance.value;
                        break;
                    }
                }

                return std::make_shared<Amount>(self->getWallet()->getCurrency(), 0, amount);
            });
        }

        Future<AbstractAccount::AddressList> StellarLikeAccount::getFreshPublicAddresses() {
            auto self = getSelf();
            return async<AbstractAccount::AddressList>([=] () {
                return AbstractAccount::AddressList({self->_params.keychain->getAddress()});
            });
        }

        Future<std::vector<std::shared_ptr<api::Amount>>>
        StellarLikeAccount::getBalanceHistory(const std::string &start, const std::string &end,
                                              api::TimePeriod precision) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        Future<api::ErrorCode> StellarLikeAccount::eraseDataSince(const std::chrono::system_clock::time_point &date) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        std::shared_ptr<StellarLikeAccount> StellarLikeAccount::getSelf() {
            return std::dynamic_pointer_cast<StellarLikeAccount>(shared_from_this());
        }

        int StellarLikeAccount::putLedger(soci::session &sql, stellar::Ledger &ledger) {
            return StellarLikeLedgerDatabaseHelper::putLedger(sql, getWallet()->getCurrency(), ledger) ? 1 : 0;
        }

        void StellarLikeAccount::putTransaction(soci::session &sql, stellar::Transaction &tx) {
            StellarLikeTransactionDatabaseHelper::putTransaction(sql, getWallet()->getCurrency(), tx);
        }

        int StellarLikeAccount::putOperation(soci::session &sql, stellar::Operation &op) {
            auto address = _params.keychain->getAddress()->toString();
            if ((op.from != address && op.to != address) || (op.sourceAmount.nonEmpty() && op.asset.type != "native"))
                return 0;

            Operation operation;
            operation.accountUid = getAccountUid();
            operation.stellarOperation = op;
            operation.currencyName = getWallet()->getCurrency().name;
            operation.amount = op.amount;
            operation.recipients = {op.to};
            operation.senders = {op.from};
            operation.date = op.createdAt;
            operation.walletType = getWallet()->getWalletType();
            operation.walletUid = getWallet()->getWalletUid();
            operation.trust = std::make_shared<TrustIndicator>();
            operation.trust->setTrustLevel(api::TrustLevel::TRUSTED);
            operation.block = {};

            if (op.from == address && (ACCEPTED_PAYMENT_TYPES.find(op.type) != ACCEPTED_PAYMENT_TYPES.end() ||
                (op.type == stellar::OperationType::PATH_PAYMENT &&
                 op.sourceAsset.getValueOr({}).type == "native"))) {
                operation.type = api::OperationType::SEND;
                if (op.type == stellar::OperationType::PATH_PAYMENT) {
                    // Small hack until path_payment is completely integrated
                    operation.amount = op.sourceAmount.getValueOr(op.amount);
                }
                operation.refreshUid();
                OperationDatabaseHelper::putOperation(sql, operation);
            }
            if (op.to == address && (ACCEPTED_PAYMENT_TYPES.find(op.type) != ACCEPTED_PAYMENT_TYPES.end() ||
                                       (op.type == stellar::OperationType::PATH_PAYMENT &&
                                        op.asset.type == "native")))  {
                operation.type = api::OperationType::RECEIVE;
                operation.refreshUid();
                OperationDatabaseHelper::putOperation(sql, operation);
            }

            return 0;
        }

        void StellarLikeAccount::updateAccountInfo(soci::session &sql, stellar::Account &account) {
            if (account.accountId == _params.keychain->getAddress()->toString()) {
                soci::transaction tr(sql);
                StellarLikeAccountDatabaseHelper::putAccount(sql, getWallet()->getWalletUid(), _params.index, account);
                tr.commit();
            }
        }

        std::shared_ptr<api::OperationQuery> StellarLikeAccount::queryOperations() {
            auto query = std::make_shared<OperationQuery>(
                    api::QueryFilter::accountEq(getAccountUid()),
                    getWallet()->getDatabase(),
                    getWallet()->getContext(),
                    getWallet()->getMainExecutionContext()
            );
            query->registerAccount(shared_from_this());
            return query;
        }

    }
}
