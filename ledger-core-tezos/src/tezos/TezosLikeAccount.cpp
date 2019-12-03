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

#include <soci.h>

#include <tezos/TezosLikeAccount.hpp>
#include <tezos/TezosLikeWallet.hpp>
#include <tezos/api/TezosLikeAddress.hpp>
#include <tezos/api/TezosOperationTag.hpp>
#include <tezos/delegation/TezosLikeOriginatedAccount.hpp>
#include <tezos/database/TezosLikeAccountDatabaseHelper.hpp>
#include <tezos/database/TezosLikeTransactionDatabaseHelper.hpp>
#include <tezos/explorers/TezosLikeBlockchainExplorer.hpp>
#include <tezos/operations/TezosLikeOperationQuery.hpp>
#include <tezos/transactions/TezosLikeTransaction.hpp>
#include <tezos/transactions/TezosLikeTransactionBuilder.hpp>

#include <core/database/SociNumber.hpp>
#include <core/database/SociDate.hpp>
#include <core/database/SociOption.hpp>
#include <core/async/Future.hpp>
#include <core/operation/OperationDatabaseHelper.hpp>
#include <core/wallet//BlockDatabaseHelper.hpp>
#include <core/synchronizers/AbstractBlockchainExplorerAccountSynchronizer.hpp>
#include <core/wallet/CurrenciesDatabaseHelper.hpp>
#include <core/events/Event.hpp>
#include <core/math/Base58.hpp>
#include <core/utils/Option.hpp>
#include <core/utils/DateUtils.hpp>
#include <core/collections/Vector.hpp>
#include <core/database/query/ConditionQueryFilter.hpp>

using namespace soci;

namespace ledger {
    namespace core {

        TezosLikeAccount::TezosLikeAccount(const std::shared_ptr<AbstractWallet> &wallet,
                                           int32_t index,
                                           const std::shared_ptr<TezosLikeBlockchainExplorer> &explorer,
                                           const std::shared_ptr<TezosLikeBlockchainObserver> &observer,
                                           const std::shared_ptr<TezosLikeAccountSynchronizer> &synchronizer,
                                           const std::shared_ptr<TezosLikeKeychain> &keychain) 
                : AbstractAccount(wallet->getServices(), wallet, index) {
            _explorer = explorer;
            _observer = observer;
            _synchronizer = synchronizer;
            _keychain = keychain;
            _accountAddress = keychain->getAddress()->toString();
        }

        void TezosLikeAccount::inflateOperation(TezosLikeOperation &operation,
                                                const std::shared_ptr<const AbstractWallet> &wallet,
                                                const TezosLikeBlockchainExplorerTransaction &tx) {
            operation.accountUid = getAccountUid();
            operation.block = tx.block;
            operation.currencyName = getWallet()->getCurrency().name;
            operation.walletUid = wallet->getWalletUid();
            operation.date = tx.receivedAt;
            if (operation.block.nonEmpty())
                operation.block.getValue().currencyName = wallet->getCurrency().name;
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

            TezosLikeOperation operation(getWallet(), transaction);
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
                if (transaction.type == api::TezosOperationTag::OPERATION_TAG_ORIGINATION) {
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
                if (transaction.type == api::TezosOperationTag::OPERATION_TAG_ORIGINATION) {
                    updateOriginatedAccounts(sql, operation);
                }
                result = static_cast<int>(transaction.type);
            }

            return result;
        }

        void TezosLikeAccount::updateOriginatedAccounts(soci::session &sql, const TezosLikeOperation &operation) {
            auto transaction = operation.getTransaction();
            auto self = std::dynamic_pointer_cast<TezosLikeAccount>(shared_from_this());
            // If account in DB then it's already in _originatedAccounts
            auto count = 0;
            auto origAccount = operation.getExplorerTransaction().originatedAccount.getValue();
            sql << "SELECT COUNT(*) FROM tezos_originated_accounts WHERE address = :originated_address", soci::use(origAccount.address), soci::into(count);
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
            api::Block abstractBlock;
            abstractBlock.blockHash = block.blockHash;
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
            std::vector<TezosLikeKeychain::Address> listAddresses{_keychain->getAddress()};
            auto currency = getWallet()->getCurrency();
            return _explorer->getBalance(listAddresses).mapPtr<Amount>(getContext(), [currency](
                    const std::shared_ptr<BigInt> &balance) -> std::shared_ptr<Amount> {
                return std::make_shared<Amount>(currency, 0, BigInt(balance->toString()));
            });
        }

        std::shared_ptr<api::OperationQuery> TezosLikeAccount::queryOperations() {
            auto originatedFilter = std::make_shared<ConditionQueryFilter<std::string>>("uid", "IS NULL", "", "orig_op");
            auto headFilter = api::QueryFilter::accountEq(getAccountUid())->op_and(originatedFilter);
            auto query = std::make_shared<TezosLikeOperationQuery>(
                    headFilter,
                    getWallet()->getDatabase(),
                    getWallet()->getContext(),
                    getWallet()->getMainExecutionContext()
            );
            query->registerAccount(shared_from_this());
            return query;
        }

        void TezosLikeAccount::getEstimatedGasLimit(
            const std::string & address, 
            const std::function<void(std::experimental::optional<std::shared_ptr<api::BigInt>>, std::experimental::optional<api::Error>)> & callback) {
            getEstimatedGasLimit(address).mapPtr<api::BigInt>(getContext(), [] (const std::shared_ptr<BigInt> &gasLimit) -> std::shared_ptr<api::BigInt> {
                return std::make_shared<BigInt>(*gasLimit);
            }).callback(getContext(), callback);
        }

        FuturePtr<BigInt> TezosLikeAccount::getEstimatedGasLimit(const std::string &address) {
            return _explorer->getEstimatedGasLimit(address);
        }

        void TezosLikeAccount::getStorage(
            const std::string & address,
            const std::function<void(std::experimental::optional<std::shared_ptr<api::BigInt>>, std::experimental::optional<api::Error>)> & callback) {
            getStorage(address).mapPtr<api::BigInt>(getContext(), [] (const std::shared_ptr<BigInt> &storage) -> std::shared_ptr<api::BigInt> {
                return std::make_shared<BigInt>(*storage);
            }).callback(getContext(), callback);
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
            return async<std::vector<std::shared_ptr<api::Amount>>>([=]() -> std::vector<std::shared_ptr<api::Amount>> {

                auto startDate = DateUtils::fromJSON(start);
                auto endDate = DateUtils::fromJSON(end);
                if (startDate >= endDate) {
                    throw make_exception(api::ErrorCode::INVALID_DATE_FORMAT,
                                         "Start date should be strictly greater than end date");
                }

                const auto &uid = self->getAccountUid();
                soci::session sql(self->getWallet()->getDatabase()->getPool());
                std::vector<TezosLikeOperation> operations;

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
