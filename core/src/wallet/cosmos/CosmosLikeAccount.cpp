/*
 *
 * CosmosLikeAccount
 *
 * Created by El Khalil Bellakrid on 14/06/2019.
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


#include <wallet/cosmos/CosmosLikeAccount.hpp>

#include <soci.h>

#include <wallet/cosmos/CosmosLikeConstants.hpp>
#include <api/CosmosLikeAddress.hpp>
#include <wallet/cosmos/CosmosLikeWallet.hpp>
#include <wallet/cosmos/api_impl/CosmosLikeTransactionApi.hpp>
#include <wallet/cosmos/database/CosmosLikeAccountDatabaseHelper.hpp>
#include <wallet/cosmos/database/CosmosLikeTransactionDatabaseHelper.hpp>
#include <wallet/cosmos/database/CosmosLikeOperationDatabaseHelper.hpp>
#include <wallet/cosmos/explorers/CosmosLikeBlockchainExplorer.hpp>
#include <wallet/cosmos/transaction_builders/CosmosLikeTransactionBuilder.hpp>
#include <wallet/cosmos/synchronizers/CosmosLikeAccountSynchronizer.hpp>
#include <wallet/cosmos/CosmosLikeOperationQuery.hpp>
#include <wallet/common/Block.h>

#include <database/soci-number.h>
#include <database/soci-date.h>
#include <database/soci-option.h>
#include <database/query/ConditionQueryFilter.h>
#include <async/Future.hpp>
#include <events/Event.hpp>
#include <math/Base58.hpp>
#include <utils/Option.hpp>
#include <utils/DateUtils.hpp>
#include <collections/vector.hpp>
#include <wallet/common/database/OperationDatabaseHelper.h>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <wallet/pool/database/CurrenciesDatabaseHelper.hpp>

#include <api/CosmosLikeValidatorListCallback.hpp>

using namespace soci;

namespace ledger {
        namespace core {

                CosmosLikeAccount::CosmosLikeAccount(const std::shared_ptr<AbstractWallet> &wallet,
                                                     int32_t index,
                                                     const std::shared_ptr<CosmosLikeBlockchainExplorer> &explorer,
                                                     const std::shared_ptr<CosmosLikeBlockchainObserver> &observer,
                                                     const std::shared_ptr<CosmosLikeAccountSynchronizer> &synchronizer,
                                                     const std::shared_ptr<CosmosLikeKeychain> &keychain) : AbstractAccount(wallet, index) {
                        _explorer = explorer;
                        _observer = observer;
                        _synchronizer = synchronizer;
                        _keychain = keychain;
                        _accountAddress = keychain->getAddress()->toString();
                }

                std::shared_ptr<api::CosmosLikeAccount> CosmosLikeAccount::asCosmosLikeAccount() {
                        return std::dynamic_pointer_cast<CosmosLikeAccount>(shared_from_this());
                }

                static void computeAndSetTypeAmount(
                    CosmosLikeOperation &out,
                    const cosmos::Message &msg,
                    const std::string &address)
                {
                    switch (cosmos::stringToMsgType(msg.type.c_str())) {
                    case api::CosmosLikeMsgType::MSGSEND: {
                        const auto &coins = boost::get<cosmos::MsgSend>(msg.content).amount;
                        std::for_each(coins.begin(), coins.end(), [&](cosmos::Coin amount) {
                            out.amount = out.amount + BigInt::fromDecimal(amount.amount);
                        });
                        const auto &sender = boost::get<cosmos::MsgSend>(msg.content).fromAddress;
                        const auto &receiver = boost::get<cosmos::MsgSend>(msg.content).toAddress;
                        out.senders = {sender};
                        out.recipients = {receiver};
                        if (sender == address) {
                            out.type = api::OperationType::SEND;
                        }
                        else if (receiver == address) {
                            out.type = api::OperationType::RECEIVE;
                        }
                        else {
                            out.type = api::OperationType::NONE;
                        }
                    } break;
                    case api::CosmosLikeMsgType::MSGMULTISEND: {
                        // Check if the user has more inputs or outputs in the message

                        const auto &inputs = boost::get<cosmos::MsgMultiSend>(msg.content).inputs;
                        const auto &outputs = boost::get<cosmos::MsgMultiSend>(msg.content).outputs;
                        BigInt recv_amount;
                        BigInt sent_amount;
                        std::vector<std::string> senders;
                        std::vector<std::string> receivers;
                        std::for_each(
                            inputs.begin(), inputs.end(), [&](cosmos::MultiSendInput input) {
                                senders.push_back(input.fromAddress);
                                if (input.fromAddress == address) {
                                    std::for_each(
                                        input.coins.begin(),
                                        input.coins.end(),
                                        [&](const cosmos::Coin& amount) {
                                            sent_amount =
                                                sent_amount + BigInt::fromDecimal(amount.amount);
                                        });
                                }
                            });

                        std::for_each(
                            outputs.begin(), outputs.end(), [&](cosmos::MultiSendOutput output) {
                                receivers.push_back(output.toAddress);
                                if (output.toAddress == address) {
                                    std::for_each(
                                        output.coins.begin(),
                                        output.coins.end(),
                                        [&](const cosmos::Coin& amount) {
                                            recv_amount =
                                                recv_amount + BigInt::fromDecimal(amount.amount);
                                        });
                                }
                            });

                        if (recv_amount == sent_amount) {
                                out.amount = BigInt::ZERO;
                                out.type = api::OperationType::NONE;
                        } else if (recv_amount > sent_amount) {
                                out.amount = recv_amount - sent_amount;
                                out.type = api::OperationType::RECEIVE;
                        } else {   //  sent_amount > recv_amount
                               out.amount = sent_amount - recv_amount;
                               out.type = api::OperationType::SEND;
                        }

                        out.senders = senders;
                        out.recipients = receivers;

                    } break;
                    case api::CosmosLikeMsgType::MSGDELEGATE: {
                        out.senders = {address};
                        out.amount = boost::get<cosmos::MsgDelegate>(msg.content).amount.amount;
                        out.type = api::OperationType::NONE;
                    } break;
                    case api::CosmosLikeMsgType::MSGUNDELEGATE: {
                        out.senders = {address};
                        out.amount = boost::get<cosmos::MsgUndelegate>(msg.content).amount.amount;
                        out.type = api::OperationType::NONE;
                    } break;
                    case api::CosmosLikeMsgType::MSGBEGINREDELEGATE: {
                        out.senders = {address};
                        out.amount =
                            boost::get<cosmos::MsgBeginRedelegate>(msg.content).amount.amount;
                        out.type = api::OperationType::NONE;
                    } break;
                    case api::CosmosLikeMsgType::MSGSUBMITPROPOSAL: {
                        const auto &coins =
                            boost::get<cosmos::MsgSubmitProposal>(msg.content).initialDeposit;
                        std::for_each(coins.begin(), coins.end(), [&](const cosmos::Coin& amount) {
                            out.amount = out.amount + BigInt::fromDecimal(amount.amount);
                        });
                        const auto &sender =
                            boost::get<cosmos::MsgSubmitProposal>(msg.content).proposer;
                        out.senders = {sender};
                        if (sender == address) {
                            out.type = api::OperationType::SEND;
                        }
                        out.type = api::OperationType::NONE;
                    } break;
                    case api::CosmosLikeMsgType::MSGDEPOSIT: {
                        const auto &coins = boost::get<cosmos::MsgDeposit>(msg.content).amount;
                        std::for_each(coins.begin(), coins.end(), [&](cosmos::Coin amount) {
                            out.amount = out.amount + BigInt::fromDecimal(amount.amount);
                        });
                        const auto &sender = boost::get<cosmos::MsgDeposit>(msg.content).depositor;
                        out.senders = {sender};
                        if (sender == address) {
                            out.type = api::OperationType::SEND;
                        }
                        out.type = api::OperationType::NONE;
                    } break;
                    case api::CosmosLikeMsgType::MSGVOTE:
                    case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATIONREWARD:
                    case api::CosmosLikeMsgType::MSGCREATEVALIDATOR:
                    case api::CosmosLikeMsgType::MSGEDITVALIDATOR:
                    case api::CosmosLikeMsgType::MSGSETWITHDRAWADDRESS:
                    case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATORREWARD:
                    case api::CosmosLikeMsgType::MSGWITHDRAWVALIDATORCOMMISSION:
                    case api::CosmosLikeMsgType::MSGUNJAIL:
                    case api::CosmosLikeMsgType::UNSUPPORTED:
                        out.senders = {address};
                        out.type = api::OperationType::NONE;
                        break;
                    }
                }

                void CosmosLikeAccount::inflateOperation(CosmosLikeOperation &out,
                                                         const std::shared_ptr<const AbstractWallet> &wallet,
                                                         const cosmos::Transaction &tx,
                                                         const cosmos::Message &msg) {

                        out.setTransactionData(tx);
                        out.setMessageData(msg);

                        computeAndSetTypeAmount(out, msg, _accountAddress);


                        // out._account = shared_from_this();
                        out.cosmosTransaction = Option<cosmos::OperationQueryResult>({tx, msg});
                        out.accountUid = getAccountUid();

                        if (tx.block){
                                out.block = Option<Block>(Block(tx.block.getValue()));
                        } else {
                                out.block = Option<Block>();
                        }
                        if (out.block.nonEmpty()) {
                                out.block.getValue().currencyName = wallet->getCurrency().name;
                        }
                        out.cosmosTransaction.getValue().tx.block = out.block;
                        out.currencyName = getWallet()->getCurrency().name;
                        out.date = tx.timestamp;
                        out.trust = std::make_shared<TrustIndicator>();
                        auto fees = 0;
                        std::for_each(tx.fee.amount.begin(), tx.fee.amount.end(), [&] (cosmos::Coin amount) {
                                assert(amount.denom == "uatom"); // FIXME Temporary until all units correctly supported
                                fees += BigInt::fromDecimal(amount.amount).toInt();
                        });
                        out.fees = BigInt(fees);
                        out.walletUid = wallet->getWalletUid();
                }

                int CosmosLikeAccount::putTransaction(soci::session &sql, const cosmos::Transaction &transaction) {
                        // FIXME Design issue: 'transaction' being const (from AbstractBlockchainObserver::putTransaction)
                        // it makes it impossible to manage uids of nested objects (eg. cosmos::Message).
                        // Writable copy of tx to allow to add uids.
                        auto tx = transaction;

                        auto wallet = getWallet();
                        if (wallet == nullptr) {
                                throw Exception(api::ErrorCode::RUNTIME_ERROR, "Wallet reference is dead.");
                        }

                        if (tx.block.nonEmpty()) {
                                putBlock(sql, tx.block.getValue().toApiBlock());
                        }

                        int result = FLAG_TRANSACTION_IGNORED;
                        auto address = getKeychain()->getAddress()->toBech32();
                        CosmosLikeTransactionDatabaseHelper::putTransaction(sql, getAccountUid(), tx);

                        for (auto msgIndex = 0 ; msgIndex < tx.messages.size() ; msgIndex++) {
                                auto msg = tx.messages[msgIndex];

                                CosmosLikeOperation operation(tx, msg);
                                inflateOperation(operation, getWallet(), tx, msg);
                                operation.refreshUid(std::to_string(msgIndex));

                                auto inserted = CosmosLikeOperationDatabaseHelper::putOperation(sql, operation);
                                if (inserted) {
                                        CosmosLikeOperationDatabaseHelper::updateOperation(sql, operation.uid, msg.uid);
                                        emitNewOperationEvent(operation);
                                }
                       }


//
//             for (const auto& msg : tx.messages) {
//                 operation.senders = {msg.sender};
//                 operation.recipients = {msg.recipient};
//                 operation.amount = msg.amount;
//                 operation.fees = Option<BigInt>(msg.fees);
//
//                 if (msg.sender == address) {
//                     // Do the send operation
//                     operation.type = api::OperationType::SEND;
//                     operation.refreshUid();
//                     if (OperationDatabaseHelper::putOperation(sql, operation)) {
//                         emitNewOperationEvent(operation);
//                     }
//                     result = static_cast<int>(operation.type);
//                 }
//
//                 if (msg.recipient == address) {
//                     // Do the receive operation
//                     operation.type = api::OperationType::RECEIVE;
//                     operation.refreshUid();
//                     if (OperationDatabaseHelper::putOperation(sql, operation)) {
//                         emitNewOperationEvent(operation);
//                     }
//                     result = static_cast<int>(operation.type);
//                 }
//
//             }

                        // Operation operation;
                        // inflateOperation(operation, wallet, transaction);
                        // std::vector<std::string> senders{transaction.sender};
                        // operation.senders = std::move(senders);
                        // std::vector<std::string> receivers{transaction.receiver};
                        // operation.recipients = std::move(receivers);
                        // operation.fees = transaction.gasLimit * transaction.gasPrice;
                        // operation.trust = std::make_shared<TrustIndicator>();
                        // operation.date = transaction.receivedAt;

                        // if (_accountAddress == transaction.sender) {
                        //     operation.amount = transaction.value;
                        //     operation.type = api::OperationType::SEND;
                        //     operation.refreshUid();
                        //     if (OperationDatabaseHelper::putOperation(sql, operation)) {
                        //         emitNewOperationEvent(operation);
                        //     }
                        //     result = static_cast<int>(operation.type);
                        // }

                        // if (_accountAddress == transaction.receiver) {
                        //     operation.amount = transaction.value;
                        //     operation.type = api::OperationType::RECEIVE;
                        //     operation.refreshUid();
                        //     if (OperationDatabaseHelper::putOperation(sql, operation)) {
                        //         emitNewOperationEvent(operation);
                        //     }
                        //     result = static_cast<int>(operation.type);
                        // }

                        return result;
                }

                bool CosmosLikeAccount::putBlock(soci::session &sql,
                                                 const api::Block &block) {
                        if (BlockDatabaseHelper::putBlock(sql, block)) {
                                emitNewBlockEvent(block);
                                return true;
                        }
                        return false;
                }

                std::shared_ptr<CosmosLikeKeychain> CosmosLikeAccount::getKeychain() const {
                        return _keychain;
                }

                FuturePtr<Amount> CosmosLikeAccount::getBalance() {
                        return getTotalBalance();
                }

                std::shared_ptr<api::OperationQuery> CosmosLikeAccount::queryOperations() {
                        auto headFilter = api::QueryFilter::accountEq(getAccountUid());
                        auto query = std::make_shared<OperationQuery>(
                                headFilter,
                                getWallet()->getDatabase(),
                                getWallet()->getContext(),
                                getWallet()->getMainExecutionContext()
                        );
                        query->registerAccount(shared_from_this());
                        return query;
                }

                void CosmosLikeAccount::getEstimatedGasLimit(const std::shared_ptr<api::CosmosLikeTransaction> &transaction, const std::shared_ptr<api::BigIntCallback> &callback) {
                        // _explorer->getEstimatedGasLimit(transaction).mapPtr<api::BigInt>(getContext(), [] (const std::shared_ptr<BigInt> &gasLimit) -> std::shared_ptr<api::BigInt> {
                        //                 return std::make_shared<api::BigIntImpl>(*gasLimit);
                        //         }).callback(getContext(), callback);
                }

                Future<AbstractAccount::AddressList> CosmosLikeAccount::getFreshPublicAddresses() {
                        auto keychain = getKeychain();
                        return async<AbstractAccount::AddressList>([=]() -> AbstractAccount::AddressList {
                                AbstractAccount::AddressList result{keychain->getAddress()};
                                return result;
                        });
                }

                Future<std::vector<std::shared_ptr<api::Amount>>>
                CosmosLikeAccount::getBalanceHistory(const std::string &start,
                                                     const std::string &end,
                                                     api::TimePeriod precision) {
                        auto self = std::dynamic_pointer_cast<CosmosLikeAccount>(shared_from_this());
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
                                        CosmosLikeOperationDatabaseHelper::queryOperations(
                                            sql,
                                            uid,
                                            operations,
                                            filter);

                                        auto lowerDate = startDate;
                                        auto upperDate = DateUtils::incrementDate(startDate, precision);

                                        std::vector<std::shared_ptr<api::Amount>> amounts;
                                        std::size_t operationsCount = 0;
                                        BigInt sum;
                                        while (lowerDate <= endDate && operationsCount < operations.size()) {

                                                auto operation = operations[operationsCount];
                                                // Fill the time slices until next operation with the running value of the accumulator
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
                                                                        sum = sum - operation.amount - operation.fees.getValueOr(BigInt::ZERO);
                                                                        break;
                                                                }
                                                                default: {
                                                                        sum = sum - operation.fees.getValueOr(BigInt::ZERO);
                                                                }
                                                                        break;
                                                        }
                                                }
                                                operationsCount += 1;
                                        }

                                        // Fill the remainder of the period with constant time slices using end value of the accumulator
                                        while (lowerDate < endDate) {
                                                lowerDate = DateUtils::incrementDate(lowerDate, precision);
                                                amounts.emplace_back(
                                                        std::make_shared<ledger::core::Amount>(self->getWallet()->getCurrency(), 0, sum));
                                        }

                                        return amounts;
                                });
                }

                Future<api::ErrorCode> CosmosLikeAccount::eraseDataSince(const std::chrono::system_clock::time_point &date) {
                        auto log = logger();
                        log->debug(" Start erasing data of account : {}", getAccountUid());
                        soci::session sql(getWallet()->getDatabase()->getPool());
                        //Update account's internal preferences (for synchronization)
                        auto savedState = getInternalPreferences()->getSubPreferences("CosmosLikeAccountSynchronizer")->getObject<cosmos::AccountSynchronizationSavedState>("state");
                        if (savedState.nonEmpty()) {
                                //Reset batches to blocks mined before given date
                                auto previousBlock = BlockDatabaseHelper::getPreviousBlockInDatabase(sql,
                                                                                                     getWallet()->getCurrency().name,
                                                                                                     date);
                                for (auto &batch : savedState.getValue().batches) {
                                        if (previousBlock.nonEmpty() && batch.blockHeight > previousBlock.getValue().height) {
                                                batch.blockHeight = (uint32_t) previousBlock.getValue().height;
                                                batch.blockHash = previousBlock.getValue().blockHash;
                                        } else if (!previousBlock.nonEmpty()) {//if no previous block, sync should go back from genesis block
                                                batch.blockHeight = 0;
                                                batch.blockHash = "";
                                        }
                                }
                                getInternalPreferences()
                                        ->getSubPreferences("CosmosLikeAccountSynchronizer")
                                        ->editor()
                                        ->putObject<cosmos::AccountSynchronizationSavedState>("state", savedState.getValue())
                                        ->commit();
                        }
                        auto accountUid = getAccountUid();
                        sql << "DELETE FROM operations WHERE account_uid = :account_uid AND date >= :date ", soci::use(
                                accountUid), soci::use(date);
                        log->debug(" Finish erasing data of account : {}", accountUid);
                        return Future<api::ErrorCode>::successful(api::ErrorCode::FUTURE_WAS_SUCCESSFULL);

                }

                bool CosmosLikeAccount::isSynchronizing() {
                        std::lock_guard<std::mutex> lock(_synchronizationLock);
                        return _currentSyncEventBus != nullptr;
                }

                std::shared_ptr<api::EventBus> CosmosLikeAccount::synchronize() {
                        std::lock_guard<std::mutex> lock(_synchronizationLock);
                        if (_currentSyncEventBus)
                                return _currentSyncEventBus;
                        auto eventPublisher = std::make_shared<EventPublisher>(getContext());

                        _currentSyncEventBus = eventPublisher->getEventBus();
                        auto future = _synchronizer->synchronizeAccount(
                                std::static_pointer_cast<CosmosLikeAccount>(shared_from_this()))->getFuture();
                        auto self = std::static_pointer_cast<CosmosLikeAccount>(shared_from_this());

                        //Update current block height (needed to compute trust level)
                        _explorer->getCurrentBlock().onComplete(getContext(),
                                                                [self](const TryPtr<cosmos::Block> &block) mutable {
                                                                        if (block.isSuccess()) {
                                                                                self->_currentBlockHeight = block.getValue()->height;
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

                std::shared_ptr<CosmosLikeAccount> CosmosLikeAccount::getSelf() {
                        return std::dynamic_pointer_cast<CosmosLikeAccount>(shared_from_this());
                }

                void CosmosLikeAccount::startBlockchainObservation() {
                        _observer->registerAccount(getSelf());
                }

                void CosmosLikeAccount::stopBlockchainObservation() {
                        _observer->unregisterAccount(getSelf());
                }

                bool CosmosLikeAccount::isObservingBlockchain() {
                        return _observer->isRegistered(getSelf());
                }

                std::string CosmosLikeAccount::getRestoreKey() {
                        return _keychain->getRestoreKey();
                }

                void CosmosLikeAccount::broadcastRawTransaction(const std::string &transaction, const std::shared_ptr<api::StringCallback>& callback) {
                        std::vector<uint8_t> tx{transaction.begin(), transaction.end()};
                        _explorer->pushTransaction(tx)
                                   .map<std::string>(getContext(), [](const String &seq) -> std::string {
                                           //TODO: optimistic update
                                           return seq.str();
                                        })
                                   .callback(getMainExecutionContext(), callback);
                }

                void CosmosLikeAccount::broadcastTransaction(const std::shared_ptr<api::CosmosLikeTransaction> &transaction, const std::shared_ptr<api::StringCallback>& callback) {
                        broadcastRawTransaction(transaction->serialize(), callback);
                }

                std::shared_ptr<api::CosmosLikeTransactionBuilder> CosmosLikeAccount::buildTransaction() {
                        return buildTransaction(std::dynamic_pointer_cast<CosmosLikeAddress>(getKeychain()->getAddress())->toString());
                }

                std::shared_ptr<api::CosmosLikeTransactionBuilder> CosmosLikeAccount::buildTransaction(const std::string &senderAddress) {
                        auto self = std::dynamic_pointer_cast<CosmosLikeAccount>(shared_from_this());
                        auto buildFunction = [self, senderAddress](const CosmosLikeTransactionBuildRequest &request,
                                                                   const std::shared_ptr<CosmosLikeBlockchainExplorer> &explorer) {
                                auto currency = self->getWallet()->getCurrency();
                                auto tx = std::make_shared<CosmosLikeTransactionApi>();
                                tx->setAccountNumber(self->getAccountUid());
                                tx->setCurrency(self->getWallet()->getCurrency());
                                tx->setFee(request.fee);
                                tx->setGas(request.gas);
                                tx->setMessages(request.messages);
                                tx->setSigningPubKey(self->getKeychain()->getPublicKey());
                                tx->setSequence(request.sequence);
                                tx->setMemo(request.memo);
                                return Future<std::shared_ptr<api::CosmosLikeTransaction>>::successful(tx);
                        };
                        return std::make_shared<CosmosLikeTransactionBuilder>(getContext(),
                                                                              getWallet()->getCurrency(),
                                                                              _explorer,
                                                                              logger(),
                                                                              buildFunction);
                }

                FuturePtr<Amount> CosmosLikeAccount::getTotalBalance() const {
                    auto currency = getWallet()->getCurrency();
                    return _explorer->getTotalBalance(_keychain->getAddress()->toBech32())
                            .mapPtr<Amount>(
                                    getContext(),
                                    [currency](const auto& rawAmount){
                                return std::make_shared<Amount>(
                                    currency,
                                    0,
                                    *rawAmount);
                                    });
                }

                void CosmosLikeAccount::getTotalBalance(const std::shared_ptr<api::AmountCallback> &callback) {
                        getTotalBalance().callback(getContext(), callback);
                }


                FuturePtr<Amount> CosmosLikeAccount::getDelegatedBalance() const {
                        auto currency = getWallet()->getCurrency();
                        return _explorer->getDelegatedBalance(_keychain->getAddress()->toBech32())
                                .mapPtr<Amount>(
                                        getContext(),
                                        [currency](const auto& rawAmount){
                                                return std::make_shared<Amount>(
                                                        currency,
                                                        0,
                                                        *rawAmount);
                                        });
                }

                void CosmosLikeAccount::getDelegatedBalance(const std::shared_ptr<api::AmountCallback> &callback) {
                        getDelegatedBalance().callback(getContext(), callback);
                }

                FuturePtr<Amount> CosmosLikeAccount::getPendingRewardsBalance() const {
                        auto currency = getWallet()->getCurrency();
                        return _explorer->getPendingRewardsBalance(_keychain->getAddress()->toBech32())
                                .mapPtr<Amount>(
                                        getContext(),
                                        [currency](const auto& rawAmount){
                                                return std::make_shared<Amount>(
                                                        currency,
                                                        0,
                                                        *rawAmount);
                                        });
                }
                void CosmosLikeAccount::getPendingRewardsBalance(const std::shared_ptr<api::AmountCallback> &callback) {
                        getPendingRewardsBalance().callback(getContext(), callback);
                }

                FuturePtr<Amount> CosmosLikeAccount::getUnbondingBalance() const {
                        auto currency = getWallet()->getCurrency();
                        return _explorer->getUnbondingBalance(_keychain->getAddress()->toBech32())
                                .mapPtr<Amount>(
                                        getContext(),
                                        [currency](const auto& rawAmount){
                                                return std::make_shared<Amount>(
                                                        currency,
                                                        0,
                                                        *rawAmount);
                                        });
                }
                void CosmosLikeAccount::getUnbondingBalance(const std::shared_ptr<api::AmountCallback> &callback) {
                        getUnbondingBalance().callback(getContext(), callback);
                }

                FuturePtr<Amount> CosmosLikeAccount::getSpendableBalance() const {
                        auto currency = getWallet()->getCurrency();
                        return _explorer->getSpendableBalance(_keychain->getAddress()->toBech32())
                                .mapPtr<Amount>(
                                        getContext(),
                                        [currency](const auto& rawAmount){
                                                return std::make_shared<Amount>(
                                                        currency,
                                                        0,
                                                        *rawAmount);
                                        });
                }
                void CosmosLikeAccount::getSpendableBalance(const std::shared_ptr<api::AmountCallback> &callback) {
                        getSpendableBalance().callback(getContext(), callback);
                }

                Future<cosmos::ValidatorList> CosmosLikeAccount::getActiveValidatorSet() const {
                        return _explorer->getActiveValidatorSet();
                }
                void CosmosLikeAccount::getLatestValidatorSet(const std::shared_ptr<api::CosmosLikeValidatorListCallback> &callback) {
                        getActiveValidatorSet().callback(getContext(), callback);
                }

                Future<cosmos::Validator> CosmosLikeAccount::getValidatorInfo(const std::string& validatorAddress) const {
                        return _explorer->getValidatorInfo(validatorAddress);
                }
                void CosmosLikeAccount::getValidatorInfo(const std::string &validatorAddress, const std::shared_ptr<api::CosmosLikeValidatorCallback>&callback) {
                        getValidatorInfo(validatorAddress).callback(getContext(), callback);
                }

                std::vector<std::shared_ptr<api::CosmosLikeDelegation>> CosmosLikeAccount::getDelegations() {
                        std::vector<std::shared_ptr<api::CosmosLikeDelegation>> delegations;
                        // TODO COIN-244
                        return delegations;
                }
        }
}
