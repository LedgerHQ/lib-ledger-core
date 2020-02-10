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
#include <wallet/common/BalanceHistory.hpp>
#include <api/BoolCallback.hpp>
#include <api/AmountCallback.hpp>
#include <events/LambdaEventReceiver.hpp>
#include "transaction_builders/StellarLikeTransactionBuilder.hpp"
#include <api/BigIntCallback.hpp>
#include <database/soci-date.h>
#include <api/StringCallback.hpp>
#include <api/StellarLikeOperationType.hpp>


using namespace ledger::core;

static const std::set<stellar::OperationType> ACCEPTED_PAYMENT_TYPES {
    stellar::OperationType::PAYMENT, stellar::OperationType::CREATE_ACCOUNT
};

static const auto INVALID_SYNCHRONIZATION_DELAY = 60 * 60 * 1000;

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


            //Update current block height (needed to compute trust level)
            _params.explorer->getLastLedger().onComplete(getContext(),
                                                    [self, eventPublisher, synchronizer](const TryPtr<stellar::Ledger> &l) mutable {
                                                        if (l.isSuccess()) {
                                                            soci::session sql(self->getWallet()->getDatabase()->getPool());
                                                            self->putLedger(sql, *l.getValue());
                                                            self->_currentLedgerHeight = l.getValue()->height;
                                                        }
                                                        auto future = synchronizer->synchronize(self)->getFuture();
                                                        auto startTime = DateUtils::now();
                                                        eventPublisher->postSticky(
                                                                std::make_shared<Event>(api::EventCode::SYNCHRONIZATION_STARTED, api::DynamicObject::newInstance()),
                                                                0);
                                                        future.onComplete(self->getContext(), [eventPublisher, self, startTime](const Try<Unit> &result) {
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
            auto self = getSelf();
            return async<std::vector<std::shared_ptr<api::Amount>>>([=]() -> std::vector<std::shared_ptr<api::Amount>> {

                auto startDate = DateUtils::fromJSON(start);
                auto endDate = DateUtils::fromJSON(end);
                if (startDate >= endDate) {
                    throw make_exception(api::ErrorCode::INVALID_DATE_FORMAT,
                                         "Start date should be strictly greater than end date");
                }

                const auto &uid = self->getAccountUid();
                soci::session sql(self->getWallet()->getDatabase()->getPool());
                std::vector<Operation> operations;

                auto keychain = self->getKeychain();
                std::function<bool(const std::string &)> filter = [&keychain](const std::string addr) -> bool {
                    return keychain->contains(addr);
                };

                //Get operations related to an account
                OperationDatabaseHelper::queryOperations(sql, uid, operations, filter);

                auto lowerDate = startDate;
                auto upperDate = DateUtils::incrementDate(startDate, precision);

                std::vector<std::shared_ptr<api::Amount>> amounts;
                std::size_t operationsCount = 0;
                BigInt sum;
                while (lowerDate <= endDate && operationsCount < operations.size()) {

                    auto operation = operations[operationsCount];
                    while (operation.date > upperDate && lowerDate < endDate) {
                        lowerDate = DateUtils::incrementDate(lowerDate, precision);
                        upperDate = DateUtils::incrementDate(upperDate, precision);
                        amounts.emplace_back(
                                std::make_shared<ledger::core::Amount>(self->getWallet()->getCurrency(), 0, sum));
                    }

                    if (operation.date <= upperDate) {
                        switch (operation.type) {
                            case api::OperationType::RECEIVE: {
                                sum = sum + operation.amount;
                                break;
                            }
                            case api::OperationType::SEND: {
                                sum = sum - (operation.amount + operation.fees.getValueOr(BigInt::ZERO));
                                break;
                            }
                            default:
                                break;
                        }
                    }
                    operationsCount += 1;
                }

                while (lowerDate < endDate) {
                    lowerDate = DateUtils::incrementDate(lowerDate, precision);
                    amounts.emplace_back(
                            std::make_shared<ledger::core::Amount>(self->getWallet()->getCurrency(), 0, sum));
                }

                return amounts;
            });
        }

        Future<api::ErrorCode> StellarLikeAccount::eraseDataSince(const std::chrono::system_clock::time_point &date) {
            auto log = logger();
            auto accountUid = getAccountUid();
            auto self = getSelf();
            return async<api::ErrorCode>([=] () {
                soci::session sql(self->getWallet()->getDatabase()->getPool());
                sql << "DELETE FROM operations WHERE account_uid = :account_uid AND date >= :date ", soci::use(
                        accountUid), soci::use(date);
                self->_params.synchronizer->reset(self, date);
                log->debug(" Finish erasing data of account : {}", accountUid);
                return api::ErrorCode::FUTURE_WAS_SUCCESSFULL;
            });
        }

        std::shared_ptr<StellarLikeAccount> StellarLikeAccount::getSelf() {
            return std::dynamic_pointer_cast<StellarLikeAccount>(shared_from_this());
        }

        int StellarLikeAccount::putLedger(soci::session &sql, stellar::Ledger &ledger) {
            return StellarLikeLedgerDatabaseHelper::putLedger(sql, getWallet()->getCurrency(), ledger) ? 1 : 0;
        }

        int StellarLikeAccount::putTransaction(soci::session &sql, const stellar::Transaction &tx) {
            int createdOperations = 0;
            StellarLikeTransactionDatabaseHelper::putTransaction(sql, getWallet()->getCurrency(), tx);
            Operation operation;
            operation.accountUid = getAccountUid();
            operation.currencyName = getWallet()->getCurrency().name;
            operation.date = tx.createdAt;
            operation.walletType = getWallet()->getWalletType();
            operation.walletUid = getWallet()->getWalletUid();
            operation.trust = std::make_shared<TrustIndicator>();
            operation.trust->setTrustLevel(api::TrustLevel::TRUSTED);

            Block block;
            block.currencyName = getWallet()->getCurrency().name;
            block.time = tx.createdAt;
            block.height = tx.ledger;
            block.hash = fmt::format("{}", block.height);

            operation.block = block;
            operation.fees = tx.feePaid;
            operation.senders.emplace_back(tx.sourceAccount);

            auto accountAddress = getKeychain()->getAddress()->toString();
             auto networkParams = getWallet()->getCurrency().stellarLikeNetworkParameters.value();
            auto toAddr = [&] (const stellar::xdr::AccountID& accountId) {
                return StellarLikeAddress::convertXdrAccountToAddress(accountId, networkParams);
            };

            auto putOp = [&] (api::OperationType type, stellar::Operation stellarOperation) {
                operation.type = type;
                stellarOperation.from = operation.senders[0];
                if (!operation.recipients.empty())
                    stellarOperation.to = operation.recipients[0];
                operation.stellarOperation = stellarOperation;
                operation.refreshUid();
                OperationDatabaseHelper::putOperation(sql, operation);
                createdOperations += 1;
            };

            auto opIndex = 0;
            for (const auto& op : tx.envelope.tx.operations) {
                stellar::Operation stellarOperation;
                stellarOperation.createdAt = tx.createdAt;
                stellarOperation.transactionFee = tx.feePaid;
                stellarOperation.transactionSequence = tx.sourceAccountSequence;
                stellarOperation.transactionHash = tx.hash;
                stellarOperation.transactionSuccessful = tx.successful;
                stellarOperation.id = (BigInt(tx.pagingToken) + BigInt(opIndex + 1)).toString();
                stellarOperation.type = op.type;
                if (op.sourceAccount.nonEmpty()) {
                    operation.senders[0] = toAddr(op.sourceAccount.getValue());
                }

                switch (op.type) {
                    case stellar::OperationType::CREATE_ACCOUNT: {
                        auto& sop = boost::get<stellar::xdr::CreateAccountOp>(op.content);
                        operation.recipients.emplace_back(toAddr(sop.destination));
                        operation.amount.assignI64(sop.startingBalance);
                        stellarOperation.asset.type = "native";
                        if (operation.senders[0] == accountAddress) {
                            putOp(api::OperationType::SEND, stellarOperation);
                        }
                        if (operation.recipients[0] == accountAddress) {
                            putOp(api::OperationType::RECEIVE, stellarOperation);
                        }
                    }
                        break;
                    case stellar::OperationType::PAYMENT: {
                        auto &sop = boost::get<stellar::xdr::PaymentOp>(op.content);
                        if (sop.asset.type == stellar::xdr::AssetType::ASSET_TYPE_NATIVE) {
                            operation.amount.assignI64(sop.amount);
                            operation.recipients.emplace_back(toAddr(sop.destination));
                            stellar::xdrAssetToAsset(sop.asset, networkParams, stellarOperation.asset);
                            if (operation.senders[0] == accountAddress) {
                                putOp(api::OperationType::SEND, stellarOperation);
                            }
                            if (operation.recipients[0] == accountAddress) {
                                putOp(api::OperationType::RECEIVE, stellarOperation);
                            }
                        }
                        if (operation.senders[0] == accountAddress && opIndex >= tx.envelope.tx.operations.size() - 1
                            && createdOperations == 0) {
                            operation.amount = BigInt::ZERO;
                            putOp(api::OperationType::SEND, stellarOperation);
                        }
                    }
                        break;
                    case stellar::OperationType::PATH_PAYMENT: {
                        // Create op if sending or receiving native token, otherwise if source account is self
                        // create fees op
                        auto& sop = boost::get<stellar::xdr::PathPaymentOp>(op.content);
                        operation.recipients.emplace_back(toAddr(sop.destination));
                        stellar::xdrAssetToAsset(sop.destAsset, networkParams, stellarOperation.asset);
                        stellar::Asset sourceAsset;
                        stellar::xdrAssetToAsset(sop.sendAsset, networkParams, sourceAsset);
                        stellarOperation.sourceAsset = sourceAsset;
                        stellarOperation.sourceAmount = BigInt(sop.sendMax);
                        stellarOperation.amount = BigInt(sop.destAmount);
                        if (operation.senders[0] == accountAddress &&
                            sop.sendAsset.type == stellar::xdr::AssetType::ASSET_TYPE_NATIVE) {
                            operation.amount = BigInt(sop.sendMax);
                            putOp(api::OperationType::SEND, stellarOperation);
                        }
                        if (operation.recipients[0] == accountAddress &&
                            sop.destAsset.type == stellar::xdr::AssetType::ASSET_TYPE_NATIVE) {
                            putOp(api::OperationType::RECEIVE, stellarOperation);
                        }
                        if (operation.senders[0] == accountAddress && opIndex >= tx.envelope.tx.operations.size() - 1
                            && createdOperations == 0) {
                            operation.amount = BigInt::ZERO;
                            putOp(api::OperationType::SEND, stellarOperation);
                        }
                    }
                        break;
                    case stellar::OperationType::ACCOUNT_MERGE:
                    case stellar::OperationType::MANAGE_OFFER:
                    case stellar::OperationType::CREATE_PASSIVE_OFFER:
                    case stellar::OperationType::SET_OPTIONS:
                    case stellar::OperationType::CHANGE_TRUST:
                    case stellar::OperationType::ALLOW_TRUST:
                    case stellar::OperationType::INFLATION:
                    case stellar::OperationType::MANAGE_DATA:
                    case stellar::OperationType::BUMP_SEQUENCE:
                    case stellar::OperationType::MANAGE_BUY_OFFER:
                        // Create fees operation if source Account is accountAdddress
                        if (operation.senders[0] == accountAddress &&
                            opIndex >= tx.envelope.tx.operations.size() - 1 &&
                            createdOperations == 0) {
                            operation.amount = BigInt::ZERO;
                            putOp(api::OperationType::SEND, stellarOperation);
                        }
                        break;
                }
                if (op.sourceAccount.nonEmpty()) {
                    operation.senders[0] = tx.sourceAccount;
                }
                operation.recipients.clear();
                opIndex += 1;
            }
            return createdOperations;
        }

//        int StellarLikeAccount::putOperation(soci::session &sql, stellar::Transaction &tx, stellar::Operation &op) {
//            auto address = _params.keychain->getAddress()->toString();
//            if ((op.from != address && op.to != address) || (op.sourceAmount.nonEmpty() && op.asset.type != "native"))
//                return 0;
//
//            stellar::Transaction tx;
//
//            // Retrieve the transaction containing this operation to get some additional data.
//            if (StellarLikeTransactionDatabaseHelper::getTransaction(sql, op.transactionHash, tx) == 0) {
//                return 0;
//            }
//
//            Operation operation;
//            operation.accountUid = getAccountUid();
//            operation.stellarOperation = op;
//            operation.currencyName = getWallet()->getCurrency().name;
//            operation.amount = op.amount;
//            operation.recipients = {op.to};
//            operation.senders = {op.from};
//            operation.date = op.createdAt;
//            operation.walletType = getWallet()->getWalletType();
//            operation.walletUid = getWallet()->getWalletUid();
//            operation.trust = std::make_shared<TrustIndicator>();
//            operation.trust->setTrustLevel(api::TrustLevel::TRUSTED);
//
//            Block block;
//            block.currencyName = getWallet()->getCurrency().name;
//            block.time = tx.createdAt;
//            block.height = tx.ledger;
//            block.hash = fmt::format("{}", block.height);
//
//            operation.block = block;
//
//            if (op.type == stellar::OperationType::CREATE_ACCOUNT) {
//                operation.stellarOperation.getValue().asset.type = "native";
//            }
//
//            if (op.from == address && (ACCEPTED_PAYMENT_TYPES.find(op.type) != ACCEPTED_PAYMENT_TYPES.end() ||
//                (op.type == stellar::OperationType::PATH_PAYMENT &&
//                 op.sourceAsset.getValueOr({}).type == "native"))) {
//                auto operationCount = 0;
//                operation.type = api::OperationType::SEND;
//                if (op.type == stellar::OperationType::PATH_PAYMENT) {
//                    // Small hack until path_payment is completely integrated
//                    operation.amount = op.sourceAmount.getValueOr(op.amount);
//                }
//                if (operationCount == 0) {
//                    // First operation inserted with fees
//                    operation.fees = tx.feePaid;
//                }
//                operation.refreshUid();
//                OperationDatabaseHelper::putOperation(sql, operation);
//            }
//            if (op.to == address && (ACCEPTED_PAYMENT_TYPES.find(op.type) != ACCEPTED_PAYMENT_TYPES.end() ||
//                                       (op.type == stellar::OperationType::PATH_PAYMENT &&
//                                        op.asset.type == "native")))  {
//                operation.type = api::OperationType::RECEIVE;
//                operation.refreshUid();
//                OperationDatabaseHelper::putOperation(sql, operation);
//            }
//
//            return 0;
//        }

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

        void StellarLikeAccount::exists(const std::shared_ptr<api::BoolCallback> &callback) {
            exists().onComplete(getContext(), [=] (const Try<bool>& result) {
                if (result.isFailure()) {
                    callback->onCallback(optional<bool>(), optional<api::Error>(api::Error(result.getFailure().getErrorCode(), result.getFailure().getMessage())));
                } else {
                    callback->onCallback(optional<bool>(result.getValue()), optional<api::Error>());
                }
            });
        }

        std::shared_ptr<api::StellarLikeTransactionBuilder> StellarLikeAccount::buildTransaction() {
            return std::make_shared<ledger::core::StellarLikeTransactionBuilder>(getSelf());
        }

        void StellarLikeAccount::broadcastRawTransaction(const std::vector<uint8_t> &tx,
                                                         const std::shared_ptr<api::StringCallback> &callback) {
            broadcastRawTransaction(tx).callback(getMainExecutionContext(), callback);
        }


        Future<std::string> StellarLikeAccount::broadcastRawTransaction(const std::vector<uint8_t> &tx) {
            return _params.explorer->postTransaction(tx);
        }

        Future<bool> StellarLikeAccount::exists() {
            return  std::dynamic_pointer_cast<StellarLikeWallet>(getWallet())->exists(_params.keychain->getAddress()->toString());
        }

        void StellarLikeAccount::getBaseReserve(const std::shared_ptr<api::AmountCallback> &callback) {
            getBaseReserve().callback(getContext(), callback);
        }

        FuturePtr<Amount> StellarLikeAccount::getBaseReserve() {
            auto self = getSelf();
            auto queryLastLedgerAndGetReserve = [=] () -> std::shared_ptr<Amount> {
                soci::session sql(self->getWallet()->getDatabase()->getPool());
                stellar::Ledger lgr;
                auto now = std::chrono::duration_cast<std::chrono::milliseconds>(DateUtils::now().time_since_epoch()).count();
                if (!StellarLikeLedgerDatabaseHelper::getLastLedger(sql, self->getWallet()->getCurrency(), lgr) ||
                      now - std::chrono::duration_cast<std::chrono::milliseconds>(lgr.time.time_since_epoch()).count() >  INVALID_SYNCHRONIZATION_DELAY) {
                    throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Last ledger is out dated");
                }
                stellar::Account acc;
                StellarLikeAccountDatabaseHelper::getAccount(sql, self->getAccountUid(), acc);
                return std::make_shared<Amount>(self->getWallet()->getCurrency(), 0, (BigInt(2) + BigInt(acc.subentryCount)) * lgr.baseReserve);
            };
            return async<std::shared_ptr<Amount>>([=] () {
                return queryLastLedgerAndGetReserve();
            }).recoverWith(getContext(), [=] (const Exception& ex) {
                Promise<Unit> p;
                self->synchronize()->subscribe(self->getContext(), make_promise_receiver(p,
                        {api::EventCode::SYNCHRONIZATION_SUCCEED, api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT},
                                                                                         {api::EventCode::SYNCHRONIZATION_FAILED}));
                return p.getFuture().mapPtr<Amount>(getContext(), [=] (const Unit&) { return queryLastLedgerAndGetReserve(); });
            });
        }

        void StellarLikeAccount::getFeeStats(const std::shared_ptr<api::StellarLikeFeeStatsCallback> &callback) {
            getFeeStats().callback(getContext(), callback);
        }

        Future<api::StellarLikeFeeStats> StellarLikeAccount::getFeeStats() {
            return _params.explorer->getRecommendedFees().map<api::StellarLikeFeeStats>(getContext(), [] (const std::shared_ptr<stellar::FeeStats>& stats) {
                return api::StellarLikeFeeStats(
                        stats->lastBaseFee.toInt64(),
                        stats->modeAcceptedFee.toInt64(),
                        stats->minAccepted.toInt64(),
                        stats->maxFee.toInt64()
                        );
            });
        }

        void StellarLikeAccount::getSequence(const std::shared_ptr<api::BigIntCallback> &callback) {
            getSequence().mapPtr<api::BigInt>(getContext(), [=] (const BigInt& i) -> std::shared_ptr<api::BigInt> {
                return std::make_shared<api::BigIntImpl>(i);
            }).callback(getContext(), callback);
        }

        Future<BigInt> StellarLikeAccount::getSequence() {
            auto self = getSelf();
            return async<BigInt>([=] () {
                stellar::Account account;
                soci::session sql(self->getWallet()->getDatabase()->getPool());
                if (StellarLikeAccountDatabaseHelper::getAccount(sql, self->getAccountUid(), account))
                    return BigInt::fromString(account.sequence);
                return BigInt::ZERO;
            });
        }

    }
}
