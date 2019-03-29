/*
 *
 * EthereumLikeAccount
 *
 * Created by El Khalil Bellakrid on 12/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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


#include "EthereumLikeAccount.h"
#include "EthereumLikeWallet.h"
#include <api/ERC20Token.hpp>
#include <wallet/common/database/OperationDatabaseHelper.h>
#include <wallet/common/synchronizers/AbstractBlockchainExplorerAccountSynchronizer.h>
#include <wallet/ethereum/database/EthereumLikeAccountDatabaseHelper.h>
#include <wallet/ethereum/explorers/EthereumLikeBlockchainExplorer.h>
#include <wallet/ethereum/keychains/EthereumLikeKeychain.hpp>
#include <wallet/ethereum/transaction_builders/EthereumLikeTransactionBuilder.h>
#include <wallet/ethereum/database/EthereumLikeTransactionDatabaseHelper.h>
#include <wallet/ethereum/api_impl/EthereumLikeTransactionApi.h>
#include <wallet/ethereum/ERC20/erc20Tokens.h>
#include <wallet/ethereum/ERC20/ERC20LikeOperation.h>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <wallet/pool/database/CurrenciesDatabaseHelper.hpp>
#include <events/Event.hpp>
#include <math/Base58.hpp>
#include <utils/Option.hpp>
#include <utils/DateUtils.hpp>

#include <database/soci-number.h>
#include <database/soci-date.h>
#include <database/soci-option.h>

namespace ledger {
    namespace core {

        EthereumLikeAccount::EthereumLikeAccount(const std::shared_ptr<AbstractWallet>& wallet,
                                                 int32_t index,
                                                 const std::shared_ptr<EthereumLikeBlockchainExplorer>& explorer,
                                                 const std::shared_ptr<EthereumLikeBlockchainObserver>& observer,
                                                 const std::shared_ptr<EthereumLikeAccountSynchronizer>& synchronizer,
                                                 const std::shared_ptr<EthereumLikeKeychain>& keychain): AbstractAccount(wallet, index) {
            _explorer = explorer;
            _observer = observer;
            _synchronizer = synchronizer;
            _keychain = keychain;
            _accountAddress = keychain->getAddress()->toString();
        }


        FuturePtr<EthereumLikeBlockchainExplorerTransaction> EthereumLikeAccount::getTransaction(const std::string& hash) {
                auto self = std::dynamic_pointer_cast<EthereumLikeAccount>(shared_from_this());
                return async<std::shared_ptr<EthereumLikeBlockchainExplorerTransaction>>([=] () -> std::shared_ptr<EthereumLikeBlockchainExplorerTransaction> {
                    auto tx = std::make_shared<EthereumLikeBlockchainExplorerTransaction>();
                    soci::session sql(self->getWallet()->getDatabase()->getPool());
                    if (!EthereumLikeTransactionDatabaseHelper::getTransactionByHash(sql, hash, *tx)) {
                            throw make_exception(api::ErrorCode::TRANSACTION_NOT_FOUND, "Transaction {} not found", hash);
                    }
                    return tx;
                });
        }

        void EthereumLikeAccount::inflateOperation(Operation &out,
                                                   const std::shared_ptr<const AbstractWallet>& wallet,
                                                   const EthereumLikeBlockchainExplorerTransaction &tx) {
                out.accountUid = getAccountUid();
                out.block = tx.block;
                out.ethereumTransaction = Option<EthereumLikeBlockchainExplorerTransaction>(tx);
                out.currencyName = getWallet()->getCurrency().name;
                out.walletType = getWalletType();
                out.walletUid = wallet->getWalletUid();
                out.date = tx.receivedAt;
                if (out.block.nonEmpty())
                        out.block.getValue().currencyName = wallet->getCurrency().name;
                out.ethereumTransaction.getValue().block = out.block;
        }

        int EthereumLikeAccount::putTransaction(soci::session &sql,
                                               const EthereumLikeBlockchainExplorerTransaction &transaction) {
                auto wallet = getWallet();
                if (wallet == nullptr) {
                        throw Exception(api::ErrorCode::RUNTIME_ERROR, "Wallet reference is dead.");
                }

                if (transaction.block.nonEmpty())
                        putBlock(sql, transaction.block.getValue());

                int result = 0x00;

                Operation operation;
                inflateOperation(operation, wallet, transaction);
                std::vector<std::string> senders{transaction.sender};
                operation.senders = std::move(senders);
                std::vector<std::string> receivers{transaction.receiver};
                operation.recipients = std::move(receivers);
                operation.fees = transaction.gasPrice * transaction.gasUsed.getValueOr(BigInt::ZERO);
                operation.trust = std::make_shared<TrustIndicator>();
                operation.date = transaction.receivedAt;

                if (_accountAddress == transaction.sender) {
                    operation.amount = transaction.value;
                    operation.type = api::OperationType::SEND;
                    operation.refreshUid();
                    OperationDatabaseHelper::putOperation(sql, operation);
                    updateERC20Accounts(sql, operation);
                    result = EthereumLikeAccount::FLAG_TRANSACTION_CREATED_SENDING_OPERATION;
                }

                if (_accountAddress == transaction.receiver) {
                    operation.amount = transaction.value;
                    operation.type = api::OperationType::RECEIVE;
                    operation.refreshUid();
                    OperationDatabaseHelper::putOperation(sql, operation);
                    updateERC20Accounts(sql, operation);
                    result = EthereumLikeAccount::FLAG_TRANSACTION_CREATED_RECEPTION_OPERATION;
                }

                return result;
        }

        void EthereumLikeAccount::updateERC20Accounts(soci::session &sql,
                                                      const Operation &operation) {
            auto transaction = operation.ethereumTransaction.getValue();
            if (transaction.erc20.nonEmpty() && !transaction.erc20.getValue().contractAddress.empty()) {
                auto accountAddress = (operation.type == api::OperationType::SEND)? transaction.sender : transaction.receiver;

                auto erc20Address = transaction.erc20.getValue().contractAddress;
                auto count = 0;
                sql << "SELECT COUNT(*) FROM erc20_tokens WHERE contract_address = :contract_address", soci::use(erc20Address), soci::into(count);
                api::ERC20Token erc20Token;
                if (count > 0) {
                    auto contractAddress = transaction.erc20.getValue().contractAddress;
                    soci::rowset<soci::row> rows = (sql.prepare << "SELECT name, symbol, number_of_decimal FROM erc20_tokens WHERE contract_address = :contract_address", soci::use(contractAddress));
                    for (auto& row : rows) {
                        auto name = row.get<std::string>(0);
                        auto symbol = row.get<std::string>(1);
                        auto numberOfDecimals = row.get<int32_t>(2);
                        erc20Token = api::ERC20Token(name, symbol, erc20Address, numberOfDecimals);
                    }
                } else {
                    erc20Token = api::ERC20Token("UNKNOWN_TOKEN", "UNKNOWN", erc20Address, 0);
                }

                auto erc20OperationUid = OperationDatabaseHelper::createUid(operation.uid, erc20Token.contractAddress, operation.type);
                auto erc20Operation = std::make_shared<ERC20LikeOperation>(accountAddress, erc20OperationUid, operation, getWallet()->getCurrency());
                auto erc20AccountUid = AccountDatabaseHelper::createERC20AccountUid(getAccountUid(), erc20Token.contractAddress);

                auto erc20OpCount = 0;
                sql << "SELECT COUNT(*) FROM erc20_operations WHERE uid = :uid", soci::use(erc20OperationUid), soci::into(erc20OpCount);
                auto newOperation = erc20OpCount == 0;
                //Check if account already exists
                auto needNewAccount = true;
                for (auto& account : _erc20LikeAccounts) {
                    auto erc20Account = std::static_pointer_cast<ERC20LikeAccount>(account);
                    if (erc20Account->getToken().contractAddress == erc20Token.contractAddress &&
                            erc20Account->getAddress() == accountAddress) {
                        //Update account
                        erc20Account->putOperation(sql, erc20Operation, newOperation);
                        needNewAccount = false;
                    }
                }

                //Create a new account
                if (needNewAccount) {
                    auto newAccount = std::make_shared<ERC20LikeAccount>(erc20AccountUid,
                                                                         erc20Token,
                                                                         accountAddress,
                                                                         getWallet()->getCurrency(),
                                                                         std::dynamic_pointer_cast<EthereumLikeAccount>(shared_from_this()));
                    _erc20LikeAccounts.push_back(newAccount);
                    //Persist erc20 account
                    int erc20AccountCount = 0;
                    sql << "SELECT COUNT(*) FROM erc20_accounts WHERE uid = :uid", soci::use(erc20AccountUid), soci::into(erc20AccountCount);
                    if (erc20AccountCount == 0) {
                        EthereumLikeAccountDatabaseHelper::createERC20Account(sql, getAccountUid(), erc20AccountUid, erc20Token.contractAddress);
                    }
                    newAccount->putOperation(sql, erc20Operation, newOperation);
                    //Update erc20 accounts table
                    if (count == 0) {
                        CurrenciesDatabaseHelper::insertERC20Token(sql, erc20Token);
                    }
                }
            }
        }
        bool EthereumLikeAccount::putBlock(soci::session& sql,
                                           const EthereumLikeBlockchainExplorer::Block& block) {
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

        std::shared_ptr<EthereumLikeKeychain> EthereumLikeAccount::getKeychain() const {
                return _keychain;
        }

        FuturePtr<Amount> EthereumLikeAccount::getBalance() {
            std::vector<EthereumLikeKeychain::Address> listAddresses{_keychain->getAddress()};
                auto currency = getWallet()->getCurrency();
                return _explorer->getBalance(listAddresses).mapPtr<Amount>(getContext(), [currency] (const std::shared_ptr<BigInt> &balance) -> std::shared_ptr<Amount> {
                    return std::make_shared<Amount>(currency, 0, BigInt(balance->toString()));
                });
        }

        std::shared_ptr<api::OperationQuery> EthereumLikeAccount::queryOperations() {
            auto query = std::make_shared<OperationQuery>(
                    api::QueryFilter::accountEq(getAccountUid()),
                    getWallet()->getDatabase(),
                    getWallet()->getContext(),
                    getWallet()->getMainExecutionContext()
            );
            query->registerAccount(shared_from_this());
            return query;
        }

        Future<AbstractAccount::AddressList> EthereumLikeAccount::getFreshPublicAddresses() {
                auto keychain = getKeychain();
                return async<AbstractAccount::AddressList>([=] () -> AbstractAccount::AddressList {
                    AbstractAccount::AddressList result{keychain->getAddress()};
                    return result;
                });
        }

        Future<std::vector<std::shared_ptr<api::Amount>>>
        EthereumLikeAccount::getBalanceHistory(const std::string & start,
                                               const std::string & end,
                                               api::TimePeriod precision) {
                auto self = std::dynamic_pointer_cast<EthereumLikeAccount>(shared_from_this());
                return async<std::vector<std::shared_ptr<api::Amount>>>([=] () -> std::vector<std::shared_ptr<api::Amount>> {

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

        Future<api::ErrorCode> EthereumLikeAccount::eraseDataSince(const std::chrono::system_clock::time_point & date) {
                auto log = logger();

                log->debug(" Start erasing data of account : {}", getAccountUid());
                soci::session sql(getWallet()->getDatabase()->getPool());
                //Update account's internal preferences (for synchronization)
                auto savedState = getInternalPreferences()->getSubPreferences("BlockchainExplorerAccountSynchronizer")->getObject<BlockchainExplorerAccountSynchronizationSavedState>("state");
                if (savedState.nonEmpty()) {
                        //Reset batches to blocks mined before given date
                        auto previousBlock = BlockDatabaseHelper::getPreviousBlockInDatabase(sql, getWallet()->getCurrency().name, date);
                        for (auto& batch : savedState.getValue().batches) {
                                if (previousBlock.nonEmpty() && batch.blockHeight > previousBlock.getValue().height) {
                                        batch.blockHeight = (uint32_t) previousBlock.getValue().height;
                                        batch.blockHash = previousBlock.getValue().blockHash;
                                } else if (!previousBlock.nonEmpty()) {//if no previous block, sync should go back from genesis block
                                        batch.blockHeight = 0;
                                        batch.blockHash = "";
                                }
                        }
                        getInternalPreferences()->getSubPreferences("BlockchainExplorerAccountSynchronizer")->editor()->putObject<BlockchainExplorerAccountSynchronizationSavedState>("state", savedState.getValue())->commit();
                }
                auto accountUid = getAccountUid();
                sql << "DELETE FROM operations WHERE account_uid = :account_uid AND date >= :date ", soci::use(accountUid), soci::use(date);
                return Future<api::ErrorCode>::successful(api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

        }

        bool EthereumLikeAccount::isSynchronizing() {
                std::lock_guard<std::mutex> lock(_synchronizationLock);
                return _currentSyncEventBus != nullptr;
        }

        std::shared_ptr<api::EventBus> EthereumLikeAccount::synchronize() {
                std::lock_guard<std::mutex> lock(_synchronizationLock);
                if (_currentSyncEventBus)
                        return _currentSyncEventBus;
                auto eventPublisher = std::make_shared<EventPublisher>(getContext());

                _currentSyncEventBus = eventPublisher->getEventBus();
                auto future = _synchronizer->synchronize(std::static_pointer_cast<EthereumLikeAccount>(shared_from_this()))->getFuture();
                auto self = std::static_pointer_cast<EthereumLikeAccount>(shared_from_this());

                //Update current block height (needed to compute trust level)
                _explorer->getCurrentBlock().onComplete(getContext(), [self] (const TryPtr<EthereumLikeBlockchainExplorer::Block>& block) mutable {
                    if (block.isSuccess()) {
                            self->_currentBlockHeight = block.getValue()->height;
                    }
                });

                auto startTime = DateUtils::now();
                eventPublisher->postSticky(std::make_shared<Event>(api::EventCode::SYNCHRONIZATION_STARTED, api::DynamicObject::newInstance()), 0);
                future.onComplete(getContext(), [eventPublisher, self, startTime] (const Try<Unit>& result) {
                    api::EventCode code;
                    auto payload = std::make_shared<DynamicObject>();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(DateUtils::now() - startTime).count();
                    payload->putLong(api::Account::EV_SYNC_DURATION_MS, duration);
                    if (result.isSuccess()) {
                            code = api::EventCode::SYNCHRONIZATION_SUCCEED;
                    } else {
                            code = api::EventCode::SYNCHRONIZATION_FAILED;
                            payload->putString(api::Account::EV_SYNC_ERROR_CODE, api::to_string(result.getFailure().getErrorCode()));
                            payload->putInt(api::Account::EV_SYNC_ERROR_CODE_INT, (int32_t)result.getFailure().getErrorCode());
                            payload->putString(api::Account::EV_SYNC_ERROR_MESSAGE, result.getFailure().getMessage());
                    }
                    eventPublisher->postSticky(std::make_shared<Event>(code, payload), 0);
                    std::lock_guard<std::mutex> lock(self->_synchronizationLock);
                    self->_currentSyncEventBus = nullptr;

                });
                return eventPublisher->getEventBus();
        }

        std::shared_ptr<EthereumLikeAccount> EthereumLikeAccount::getSelf() {
                return std::dynamic_pointer_cast<EthereumLikeAccount>(shared_from_this());
        }

        void EthereumLikeAccount::startBlockchainObservation() {
                _observer->registerAccount(getSelf());
        }

        void EthereumLikeAccount::stopBlockchainObservation() {
                _observer->unregisterAccount(getSelf());
        }

        bool EthereumLikeAccount::isObservingBlockchain() {
                return _observer->isRegistered(getSelf());
        }

        std::string EthereumLikeAccount::getRestoreKey() {
                return _keychain->getRestoreKey();
        }

        void EthereumLikeAccount::broadcastRawTransaction(const std::vector<uint8_t> & transaction,
                                                          const std::shared_ptr<api::StringCallback> & callback) {
                _explorer->pushTransaction(transaction).map<std::string>(getContext(), [] (const String& seq) -> std::string {
                    //TODO: optimistic update
                    return seq.str();
                }).callback(getContext(), callback);
        }

        void EthereumLikeAccount::broadcastTransaction(const std::shared_ptr<api::EthereumLikeTransaction> & transaction,
                                                       const std::shared_ptr<api::StringCallback> & callback) {
                broadcastRawTransaction(transaction->serialize(), callback);
        }

        std::shared_ptr<api::EthereumLikeAccount> EthereumLikeAccount::asEthereumLikeAccount() {
                return std::dynamic_pointer_cast<EthereumLikeAccount>(shared_from_this());
        }

        std::vector<std::shared_ptr<api::ERC20LikeAccount>>
        EthereumLikeAccount::getERC20Accounts() {
                return _erc20LikeAccounts;
        }
        std::shared_ptr<api::EthereumLikeTransactionBuilder> EthereumLikeAccount::buildTransaction() {

                auto self = std::dynamic_pointer_cast<EthereumLikeAccount>(shared_from_this());

                auto getTransaction = [self] (const std::string& hash) -> FuturePtr<EthereumLikeBlockchainExplorerTransaction> {
                    return self->getTransaction(hash);
                };

                auto buildFunction = [self] (const EthereumLikeTransactionBuildRequest& request, const std::shared_ptr<EthereumLikeBlockchainExplorer> &explorer) -> Future<std::shared_ptr<api::EthereumLikeTransaction>> {
                    auto tx = std::make_shared<EthereumLikeTransactionApi>(self->getWallet()->getCurrency());
                    tx->setValue(request.value);
                    tx->setData(request.inputData);
                    tx->setGasLimit(request.gasLimit);
                    tx->setGasPrice(request.gasPrice);
                    tx->setReceiver(request.toAddress);
                    return explorer->getNonce(self->getKeychain()->getAddress()->toString()).map<std::shared_ptr<api::EthereumLikeTransaction>>(self->getContext(), [self, tx] (const std::shared_ptr<BigInt> &nonce) -> std::shared_ptr<api::EthereumLikeTransaction> {
                        tx->setNonce(nonce);
                        return tx;
                    });
                };

                return std::make_shared<EthereumLikeTransactionBuilder>(getContext(),
                                                                        getWallet()->getCurrency(),
                                                                        _explorer,
                                                                        logger(),
                                                                        buildFunction);
        }

    }
}