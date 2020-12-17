/*
 *
 * TezosLikeAccount
 *
 * Created by El Khalil Bellakrid on 27/04/2019.
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


#include "TezosLikeAccount.h"
#include "TezosLikeWallet.h"
#include <soci.h>
#include <database/soci-number.h>
#include <database/soci-date.h>
#include <database/soci-option.h>
#include <api/TezosLikeAddress.hpp>
#include <api/TezosOperationTag.hpp>
#include <async/Future.hpp>
#include <wallet/common/database/OperationDatabaseHelper.h>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <wallet/common/synchronizers/AbstractBlockchainExplorerAccountSynchronizer.h>
#include <wallet/pool/database/CurrenciesDatabaseHelper.hpp>
#include <wallet/pool/WalletPool.hpp>
#include <wallet/tezos/api_impl/TezosLikeTransactionApi.h>
#include <wallet/tezos/database/TezosLikeAccountDatabaseHelper.h>
#include <wallet/tezos/database/TezosLikeTransactionDatabaseHelper.h>
#include <wallet/tezos/explorers/TezosLikeBlockchainExplorer.h>
#include <wallet/tezos/transaction_builders/TezosLikeTransactionBuilder.h>
#include <wallet/tezos/delegation/TezosLikeOriginatedAccount.h>
#include <events/Event.hpp>
#include <math/Base58.hpp>
#include <utils/Option.hpp>
#include <utils/DateUtils.hpp>
#include <collections/vector.hpp>
#include <database/query/ConditionQueryFilter.h>

using namespace soci;

namespace ledger {
    namespace core {

        TezosLikeAccount::TezosLikeAccount(const std::shared_ptr<AbstractWallet> &wallet,
                                           int32_t index,
                                           const std::shared_ptr<TezosLikeBlockchainExplorer> &explorer,
                                           const std::shared_ptr<TezosLikeBlockchainObserver> &observer,
                                           const std::shared_ptr<TezosLikeAccountSynchronizer> &synchronizer,
                                           const std::shared_ptr<TezosLikeKeychain> &keychain) : AbstractAccount(wallet, index) {
            _explorer = explorer;
            _observer = observer;
            _synchronizer = synchronizer;
            _keychain = keychain;
            _accountAddress = keychain->getAddress()->toString();
        }

        void TezosLikeAccount::inflateOperation(Operation &out,
                                                const std::shared_ptr<const AbstractWallet> &wallet,
                                                const TezosLikeBlockchainExplorerTransaction &tx) {
            out.accountUid = getAccountUid();
            out.block = tx.block;
            out.tezosTransaction = Option<TezosLikeBlockchainExplorerTransaction>(tx);
            out.currencyName = getWallet()->getCurrency().name;
            out.walletType = getWalletType();
            out.walletUid = wallet->getWalletUid();
            out.date = tx.receivedAt;
            if (out.block.nonEmpty())
                out.block.getValue().currencyName = wallet->getCurrency().name;
            out.tezosTransaction.getValue().block = out.block;
        }

        int TezosLikeAccount::putTransaction(soci::session &sql,
                                             const TezosLikeBlockchainExplorerTransaction &transaction,
                                             const std::string &originatedAccountUid,
                                             const std::string &originatedAccountAddress) {
            auto wallet = getWallet();
            if (wallet == nullptr) {
                throw Exception(api::ErrorCode::RUNTIME_ERROR, "Wallet reference is dead.");
            }

            if (transaction.block.nonEmpty()) {
                putBlock(sql, transaction.block.getValue());
            }

            int result = FLAG_TRANSACTION_IGNORED;

            Operation operation;
            inflateOperation(operation, wallet, transaction);
            std::vector<std::string> senders{transaction.sender};
            operation.senders = std::move(senders);
            std::vector<std::string> receivers{transaction.receiver};
            operation.recipients = std::move(receivers);
            operation.fees = transaction.fees;
            operation.trust = std::make_shared<TrustIndicator>();
            operation.date = transaction.receivedAt;


            // Check if it's an operation related to an originated account
            // It can be the case if we are putting transaction operations
            // for originated account.
            if (!originatedAccountUid.empty() && !originatedAccountAddress.empty()) {
                operation.amount = transaction.value;
                operation.type = transaction.sender == originatedAccountAddress ? api::OperationType::SEND : api::OperationType::RECEIVE;
                operation.refreshUid(originatedAccountUid);
                if (OperationDatabaseHelper::putOperation(sql, operation)) {
                    // Update publicKey field for originated account
                    if (transaction.type == api::TezosOperationTag::OPERATION_TAG_REVEAL && transaction.publicKey.hasValue()) {
                        TezosLikeAccountDatabaseHelper::updatePubKeyField(sql, originatedAccountUid, transaction.publicKey.getValue());
                    }
                    // Add originated account ops
                    auto tezosTxUid = TezosLikeTransactionDatabaseHelper::createTezosTransactionUid(operation.accountUid, transaction.hash, transaction.type);
                    TezosLikeAccountDatabaseHelper::addOriginatedAccountOperation(sql, operation.uid, tezosTxUid, originatedAccountUid);
                    emitNewOperationEvent(operation);
                }
                result = static_cast<int>(transaction.type);
                return result;
            }

            if (_accountAddress == transaction.sender) {
                operation.amount = transaction.value;
                operation.type = api::OperationType::SEND;
                operation.refreshUid();
                if (OperationDatabaseHelper::putOperation(sql, operation)) {
                    emitNewOperationEvent(operation);
                }
                if (transaction.type == api::TezosOperationTag::OPERATION_TAG_ORIGINATION && transaction.status == 1) {
                    updateOriginatedAccounts(sql, operation);
                }
                result = static_cast<int>(transaction.type);
            }

            if (_accountAddress == transaction.receiver) {
                operation.amount = transaction.value;
                operation.type = api::OperationType::RECEIVE;
                operation.refreshUid();
                if (OperationDatabaseHelper::putOperation(sql, operation)) {
                    emitNewOperationEvent(operation);
                }
                result = static_cast<int>(transaction.type);
            }

            return result;
        }

        void TezosLikeAccount::updateOriginatedAccounts(soci::session &sql, const Operation &operation) {
            auto transaction = operation.tezosTransaction.getValue();
            auto self = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
            auto origAccount = transaction.originatedAccount.getValue();

            // If account in DB then it's already in _originatedAccounts
            auto count = 0;
            sql << "SELECT COUNT(*) FROM tezos_originated_accounts "
                   "WHERE address = :originated_address AND tezos_account_uid =:account_uid",
                   soci::use(origAccount.address), soci::use(getAccountUid()), soci::into(count);

            if (count == 0) {
                std::string pubKey;
                int spendable = origAccount.spendable, delegatable = origAccount.delegatable;
                auto originatedAccountUid = TezosLikeAccountDatabaseHelper::createOriginatedAccountUid(getAccountUid(), origAccount.address);
                sql << "INSERT INTO tezos_originated_accounts VALUES(:uid, :tezos_account_uid, :address, :spendable, :delegatable, :public_key)",
                        use(originatedAccountUid),
                        use(getAccountUid()),
                        use(origAccount.address),
                        use(spendable),
                        use(delegatable),
                        use(pubKey);

                _originatedAccounts.emplace_back(
                        std::make_shared<TezosLikeOriginatedAccount>(originatedAccountUid,
                                                                     origAccount.address,
                                                                     self,
                                                                     origAccount.spendable,
                                                                     origAccount.delegatable)
                );
            }
        }

        bool TezosLikeAccount::putBlock(soci::session &sql,
                                        const TezosLikeBlockchainExplorer::Block &block) {
            Block abstractBlock;
            abstractBlock.hash = block.hash;
            abstractBlock.currencyName = getWallet()->getCurrency().name;
            abstractBlock.height = block.height;
            abstractBlock.time = block.time;
            if (BlockDatabaseHelper::putBlock(sql, abstractBlock)) {
                emitNewBlockEvent(abstractBlock);
                return true;
            }
            return false;
        }

        std::shared_ptr<TezosLikeKeychain> TezosLikeAccount::getKeychain() const {
            return _keychain;
        }

        FuturePtr<Amount> TezosLikeAccount::getBalance() {
            auto cachedBalance = getWallet()->getBalanceFromCache(getIndex());
            if (cachedBalance.hasValue()) {
                return FuturePtr<Amount>::successful(std::make_shared<Amount>(cachedBalance.getValue()));
            }
            auto currency = getWallet()->getCurrency();
            auto self = getSelf();
            return _explorer->getBalance(_keychain->getAddress()->toBase58()).mapPtr<Amount>(getMainExecutionContext(), [self, currency](
                    const std::shared_ptr<BigInt> &balance) -> std::shared_ptr<Amount> {
                Amount b(currency, 0, BigInt(balance->toString()));
                self->getWallet()->updateBalanceCache(self->getIndex(), b);
                return std::make_shared<Amount>(b);
            });
        }

        std::shared_ptr<api::OperationQuery> TezosLikeAccount::queryOperations() {
            auto originatedFilter = std::make_shared<ConditionQueryFilter<std::string>>("uid", "IS NULL", "", "orig_op");
            auto headFilter = api::QueryFilter::accountEq(getAccountUid())->op_and(originatedFilter);
            auto query = std::make_shared<TezosOperationQuery>(
                    headFilter,
                    getWallet()->getDatabase(),
                    getWallet()->getPool()->getThreadPoolExecutionContext(),
                    getWallet()->getMainExecutionContext()
            );
            query->registerAccount(shared_from_this());
            return query;
        }

        void TezosLikeAccount::getEstimatedGasLimit(const std::string & address, const std::shared_ptr<api::BigIntCallback> & callback) {
            getEstimatedGasLimit(address).mapPtr<api::BigInt>(getMainExecutionContext(), [] (const std::shared_ptr<BigInt> &gasLimit) -> std::shared_ptr<api::BigInt> {
                return std::make_shared<api::BigIntImpl>(*gasLimit);
            }).callback(getMainExecutionContext(), callback);
        }

        FuturePtr<BigInt> TezosLikeAccount::getEstimatedGasLimit(const std::string &address) {
            return _explorer->getEstimatedGasLimit(address);
        }

        void TezosLikeAccount::getStorage(const std::string & address, const std::shared_ptr<api::BigIntCallback> & callback) {
            getStorage(address).mapPtr<api::BigInt>(getMainExecutionContext(), [] (const std::shared_ptr<BigInt> &storage) -> std::shared_ptr<api::BigInt> {
                return std::make_shared<api::BigIntImpl>(*storage);
            }).callback(getMainExecutionContext(), callback);
        }

        FuturePtr<BigInt> TezosLikeAccount::getStorage(const std::string &address) {
            return _explorer->getStorage(address);
        }

        std::vector<std::shared_ptr<api::TezosLikeOriginatedAccount>>
        TezosLikeAccount::getOriginatedAccounts() {
            return _originatedAccounts;
        }

        Future<AbstractAccount::AddressList> TezosLikeAccount::getFreshPublicAddresses() {
            auto keychain = getKeychain();
            return async<AbstractAccount::AddressList>([=]() -> AbstractAccount::AddressList {
                AbstractAccount::AddressList result{keychain->getAddress()};
                return result;
            });
        }

        Future<std::vector<std::shared_ptr<api::Amount>>>
        TezosLikeAccount::getBalanceHistory(const std::string &start,
                                            const std::string &end,
                                            api::TimePeriod precision) {
            auto self = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
            return Future<std::vector<std::shared_ptr<api::Amount>>>::async(getWallet()->getPool()->getThreadPoolExecutionContext(), [=]() -> std::vector<std::shared_ptr<api::Amount>> {

                auto startDate = DateUtils::fromJSON(start);
                auto endDate = DateUtils::fromJSON(end);
                if (startDate >= endDate) {
                    throw make_exception(api::ErrorCode::INVALID_DATE_FORMAT,
                                         "Start date should be strictly lower than end date");
                }

                const auto &uid = self->getAccountUid();
                soci::session sql(self->getWallet()->getDatabase()->getPool());
                std::vector<Operation> operations;

                auto keychain = self->getKeychain();
                std::function<bool(const std::string &)> filter = [&keychain](const std::string addr) -> bool {
                    return keychain->contains(addr);
                };

                //Get operations related to an account
                TezosLikeAccountDatabaseHelper::queryOperations(sql, uid, operations, filter);

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

    }
}
