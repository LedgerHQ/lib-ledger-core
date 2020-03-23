    /*
    *
    * BitcoinLikeAccount
    * ledger-core
    *
    * Created by Pierre Pollastri on 28/04/2017.
    *
    * The MIT License (MIT)
    *
    * Copyright (c) 2016 Ledger
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

    #include <spdlog/logger.h>

    #include <core/database/query/QueryBuilder.hpp>
    #include <core/collections/Functional.hpp>
    #include <core/utils/DateUtils.hpp>
    #include <core/operation/Operation.hpp>
    #include <core/synchronizers/AbstractBlockchainExplorerAccountSynchronizer.hpp>
    #include <core/api/EventCode.hpp>
    #include <core/events/EventPublisher.hpp>
    #include <core/events/Event.hpp>
    #include <core/database/SociNumber.hpp>
    #include <core/database/SociDate.hpp>
    #include <core/database/SociOption.hpp>

    #include <bitcoin/database/BitcoinLikeUTXODatabaseHelper.hpp>
    #include <bitcoin/BitcoinLikeAccount.hpp>
    #include <bitcoin/database/BitcoinLikeBlockDatabaseHelper.hpp>
    #include <bitcoin/io/BitcoinLikeOutput.hpp>
    #include <bitcoin/api/BitcoinLikeInput.hpp>
    #include <bitcoin/database/BitcoinLikeBlockDatabaseHelper.hpp>
    #include <bitcoin/transactions/BitcoinLikeTransactionBuilder.hpp>
    #include <bitcoin/transactions/BitcoinLikeStrategyUTXOPicker.hpp>
    #include <bitcoin/database/BitcoinLikeTransactionDatabaseHelper.hpp>
    #include <bitcoin/transactions/BitcoinLikeTransaction.hpp>
    #include <bitcoin/operations/BitcoinLikeOperationQuery.hpp>
    #include <bitcoin/database/BitcoinLikeOperationDatabaseHelper.hpp>
    namespace ledger {
        namespace core {

            BitcoinLikeAccount::BitcoinLikeAccount(const std::shared_ptr<AbstractWallet> &wallet,
                                                int32_t index,
                                                const std::shared_ptr<BitcoinLikeBlockchainExplorer> &explorer,
                                                const std::shared_ptr<BitcoinLikeBlockchainObserver> &observer,
                                                const std::shared_ptr<BitcoinLikeAccountSynchronizer> &synchronizer,
                                                const std::shared_ptr<BitcoinLikeKeychain> &keychain)
                    : AbstractAccount(wallet->getServices(), wallet, index) {
                _explorer = explorer;
                _observer = observer;
                _synchronizer = synchronizer;
                _keychain = keychain;
                _keychain->getAllObservableAddresses(0, 40);
                _picker = std::make_shared<BitcoinLikeStrategyUTXOPicker>(getContext(), getWallet()->getCurrency());
                _currentBlockHeight = 0;
            }

            void
            BitcoinLikeAccount::inflateOperation(BitcoinLikeOperation &out,
                                                const BitcoinLikeBlockchainExplorerTransaction &tx) {
                out.accountUid = getAccountUid();
                out.block = tx.block;
                // Set accountUid of bitcoin outputs
                for (auto &output : out.getExplorerTransaction().outputs) {
                    if (output.address.hasValue() && getKeychain()->contains(output.address.getValue())) {
                        output.accountUid = getAccountUid();
                    }
                }
                out.currencyName = getWallet()->getCurrency().name;
                out.walletUid = getWallet()->getWalletUid();
                out.date = tx.receivedAt;

                if (out.block.nonEmpty())
                    out.block.getValue().currencyName = getWallet()->getCurrency().name;

                out.getExplorerTransaction().block = out.block;
            }

            int BitcoinLikeAccount::putTransaction(soci::session &sql,
                                                const BitcoinLikeBlockchainExplorerTransaction &transaction) {
                if (transaction.block.nonEmpty())
                    putBlock(sql, transaction.block.getValue());
                auto nodeIndex = std::const_pointer_cast<const BitcoinLikeKeychain>(_keychain)->getFullDerivationScheme().getPositionForLevel(DerivationSchemeLevel::NODE);
                std::list<std::pair<BitcoinLikeBlockchainExplorerInput *, DerivationPath>> accountInputs;
                std::list<std::pair<BitcoinLikeBlockchainExplorerOutput *, DerivationPath>> accountOutputs;
                uint64_t fees = 0L;
                uint64_t sentAmount = 0L;
                uint64_t receivedAmount = 0L;
                std::vector<std::string> senders;
                senders.reserve(transaction.inputs.size());
                std::vector<std::string> recipients;
                recipients.reserve(transaction.outputs.size());
                int result = 0x00;

                // Find inputs
                for (auto& input : transaction.inputs) {

                    if (input.address.nonEmpty()) {
                        senders.push_back(input.address.getValue());
                    }
                    // Extend input with derivation paths

                    if (input.address.nonEmpty() && input.value.nonEmpty()) {
                        auto path = _keychain->getAddressDerivationPath(input.address.getValue());
                        if (path.nonEmpty()) {
                            // This address is part of the account.
                            sentAmount += input.value.getValue().toUint64();
                            accountInputs.push_back(std::make_pair(const_cast<BitcoinLikeBlockchainExplorerInput *>(&input), DerivationPath(path.getValue())));
                            if (_keychain->markPathAsUsed(DerivationPath(path.getValue()))) {
                                result = result | FLAG_TRANSACTION_ON_PREVIOUSLY_EMPTY_ADDRESS;
                            } else {
                                result = result | FLAG_TRANSACTION_ON_USED_ADDRESS;
                            }
                        }
                    }
                    if (input.value.nonEmpty()) {
                        fees += input.value.getValue().toUint64();
                    }
                }

                // Find outputs
                auto hasSpentNothing = sentAmount == 0L;
                auto outputCount = transaction.outputs.size();
                for (auto index = 0; index < outputCount; index++) {
                    auto& output = transaction.outputs[index];
                    if (output.address.nonEmpty()) {
                        auto path = _keychain->getAddressDerivationPath(output.address.getValue());
                        if (path.nonEmpty()) {
                            DerivationPath p(path.getValue());
                            accountOutputs.push_back(std::make_pair(const_cast<BitcoinLikeBlockchainExplorerOutput *>(&output), p));
                            if (p.getNonHardenedChildNum(nodeIndex) == 1) {
                                if (hasSpentNothing) {
                                    receivedAmount +=  output.value.toUint64();
                                }
                                if ((recipients.size() == 0 && index + 1 >= outputCount) || hasSpentNothing) {
                                    recipients.push_back(output.address.getValue());
                                }
                            } else {
                                receivedAmount += output.value.toUint64();
                                recipients.push_back(output.address.getValue());
                            }
                            if (_keychain->markPathAsUsed(DerivationPath(path.getValue()))) {
                                result = result | FLAG_TRANSACTION_ON_PREVIOUSLY_EMPTY_ADDRESS;
                            } else {
                                result = result | FLAG_TRANSACTION_ON_USED_ADDRESS;
                            }
                        } else {
                            recipients.push_back(output.address.getValue());
                        }
                    }
                    fees = fees - output.value.toUint64();
                }
                std::stringstream snds;
                strings::join(senders, snds, ",");

                BitcoinLikeOperation operation(shared_from_this(), transaction);
                inflateOperation(operation, transaction);

                operation.senders = std::move(senders);
                operation.recipients = std::move(recipients);
                operation.fees = std::move(BigInt().assignI64(fees));
                operation.trust = std::make_shared<TrustIndicator>();
                operation.date = transaction.receivedAt;

                // Compute trust
                computeOperationTrust(operation, transaction);

                if (accountInputs.size() > 0) {
                    // Create a send operation
                    result = result | FLAG_TRANSACTION_CREATED_SENDING_OPERATION;

                    for (auto& accountOutput : accountOutputs) {
                        if (accountOutput.second.getNonHardenedChildNum(nodeIndex) == 1)
                            sentAmount -= accountOutput.first->value.toInt64();
                    }
                    sentAmount -= fees;

                    operation.amount.assignI64(sentAmount);
                    operation.type = api::OperationType::SEND;
                    operation.refreshUid();

                    auto inserted = BitcoinLikeOperationDatabaseHelper::putOperation(sql, operation);

                    if (inserted) {
                        emitNewOperationEvent(operation);
                    }

                    BitcoinLikeOperationDatabaseHelper::updateOperation(sql, operation, inserted);
                }

                if (accountOutputs.size() > 0) {
                    // Receive
                    BigInt amount;
                    auto flag = 0;
                    bool filterChangeAddresses = true;

                    if (accountInputs.size() == 0) {
                        filterChangeAddresses = false;
                    }

                    BigInt finalAmount;
                    auto accountOutputCount = 0;
                    for (auto& o : accountOutputs) {
                        if (filterChangeAddresses && o.second.getNonHardenedChildNum(nodeIndex) == 1)
                            continue;
                        finalAmount = finalAmount + o.first->value;
                        accountOutputCount += 1;
                    }
                    if (accountOutputCount > 0) {
                        operation.amount = finalAmount;
                        operation.type = api::OperationType::RECEIVE;
                        operation.refreshUid();

                        auto inserted = BitcoinLikeOperationDatabaseHelper::putOperation(sql, operation);

                        if (inserted) {
                            emitNewOperationEvent(operation);
                        }

                        BitcoinLikeOperationDatabaseHelper::updateOperation(sql, operation, inserted);
                    }

                }

                return result;
            }

            void
            BitcoinLikeAccount::computeOperationTrust(BitcoinLikeOperation &operation, const BitcoinLikeBlockchainExplorerTransaction &tx) {
                if (tx.block.nonEmpty()) {
                    auto txBlockHeight = tx.block.getValue().height;
                    if (_currentBlockHeight > txBlockHeight + 5 ) {
                        operation.trust->setTrustLevel(api::TrustLevel::TRUSTED);
                    } else if (_currentBlockHeight > txBlockHeight) {
                        operation.trust->setTrustLevel(api::TrustLevel::UNTRUSTED);
                    } else if (_currentBlockHeight == txBlockHeight) {
                        operation.trust->setTrustLevel(api::TrustLevel::PENDING);
                    }

                } else {
                    operation.trust->setTrustLevel(api::TrustLevel::DROPPED);
                }
            }

            std::shared_ptr<BitcoinLikeKeychain> BitcoinLikeAccount::getKeychain() const {
                return _keychain;
            }


            bool BitcoinLikeAccount::isSynchronizing() {
                std::lock_guard<std::mutex> lock(_synchronizationLock);
                return _currentSyncEventBus != nullptr;
            }

            std::shared_ptr<api::EventBus> BitcoinLikeAccount::synchronize() {
                std::lock_guard<std::mutex> lock(_synchronizationLock);
                if (_currentSyncEventBus)
                    return _currentSyncEventBus;
                auto eventPublisher = std::make_shared<EventPublisher>(getContext());
                auto wasEmpty = checkIfWalletIsEmpty();
                _currentSyncEventBus = eventPublisher->getEventBus();
                auto future = _synchronizer->synchronize(std::static_pointer_cast<BitcoinLikeAccount>(shared_from_this()))->getFuture();
                auto self = std::static_pointer_cast<BitcoinLikeAccount>(shared_from_this());

                //Update current block height (needed to compute trust level)
                _explorer->getCurrentBlock().onComplete(getContext(), [self] (const TryPtr<BitcoinLikeBlockchainExplorer::Block>& block) mutable {
                    if (block.isSuccess()) {
                        self->_currentBlockHeight = block.getValue()->height;
                    }
                });

                auto startTime = DateUtils::now();
                eventPublisher->postSticky(std::make_shared<Event>(api::EventCode::SYNCHRONIZATION_STARTED, api::DynamicObject::newInstance()), 0);
                future.onComplete(getContext(), [eventPublisher, self, wasEmpty, startTime] (const Try<Unit>& result) {
                    auto isEmpty = self->checkIfWalletIsEmpty();
                    api::EventCode code;
                    auto payload = std::make_shared<DynamicObject>();
                    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(DateUtils::now() - startTime).count();
                    payload->putLong(api::Account::EV_SYNC_DURATION_MS, duration);
                    if (result.isSuccess()) {
                        code = !isEmpty && wasEmpty ? api::EventCode::SYNCHRONIZATION_SUCCEED_ON_PREVIOUSLY_EMPTY_ACCOUNT
                                                    : api::EventCode::SYNCHRONIZATION_SUCCEED;
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

            void BitcoinLikeAccount::getUTXO(
				int32_t from,
				int32_t to,
				const std::shared_ptr<api::BitcoinLikeOutputListCallback> & callback
			) {
                getUTXO(from, to).callback(getMainExecutionContext(), callback);
            }

            Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>>
            BitcoinLikeAccount::getUTXO(int32_t from, int32_t to) {
                auto self = getSelf();
                return async<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>>([=] () -> std::vector<std::shared_ptr<api::BitcoinLikeOutput>> {
                    auto keychain = self->getKeychain();
                    soci::session sql(self->getWallet()->getDatabase()->getPool());
                    std::vector<BitcoinLikeBlockchainExplorerOutput> utxo;
                    BitcoinLikeUTXODatabaseHelper::queryUTXO(sql, self->getAccountUid(), from, to - from, utxo, [&keychain] (const std::string& addr) {
                        return keychain->contains(addr);
                    });
                    auto currency = self->getWallet()->getCurrency();
                    return functional::map<BitcoinLikeBlockchainExplorerOutput, std::shared_ptr<api::BitcoinLikeOutput>>(utxo, [&currency] (const BitcoinLikeBlockchainExplorerOutput& output) -> std::shared_ptr<api::BitcoinLikeOutput> {
                        return std::make_shared<BitcoinLikeOutput>(output, currency);
                    });
                });
            }

            void BitcoinLikeAccount::getUTXOCount(const std::shared_ptr<api::I32Callback> & callback) {
                getUTXOCount().callback(getMainExecutionContext(), callback);
            }

            bool BitcoinLikeAccount::putBlock(soci::session &sql, const BitcoinLikeBlockchainExplorer::Block& block) {
                api::Block abstractBlock;
                abstractBlock.blockHash = block.blockHash;
                abstractBlock.currencyName = getWallet()->getCurrency().name;
                abstractBlock.height = block.height;
                abstractBlock.time = block.time;
                if (BlockDatabaseHelper::putBlock(sql, abstractBlock)) {
                    BitcoinLikeBlockDatabaseHelper::putBlock(sql, block);
                    emitNewBlockEvent(abstractBlock);
                    return true;
                }
                return false;
            }

            Future<int32_t> BitcoinLikeAccount::getUTXOCount() {
                auto self = getSelf();
                return async<int32_t>([=] () -> int32_t {
                    auto keychain = self->getKeychain();
                    soci::session sql(self->getWallet()->getDatabase()->getPool());
                    return (int32_t) BitcoinLikeUTXODatabaseHelper::UTXOcount(sql, self->getAccountUid(), [keychain] (const std::string& addr) -> bool {
                        return keychain->contains(addr);
                    });
                });
            }

            std::shared_ptr<api::OperationQuery> BitcoinLikeAccount::queryOperations() {
                auto query = std::make_shared<BitcoinLikeOperationQuery>(
                        api::QueryFilter::accountEq(getAccountUid()),
                        getWallet()->getDatabase(),
                        getWallet()->getContext(),
                        getWallet()->getMainExecutionContext()
                );
                query->registerAccount(shared_from_this());
                return query;
            }

            bool BitcoinLikeAccount::checkIfWalletIsEmpty() {
                return _keychain->isEmpty();
            }

            Future<AbstractAccount::AddressList> BitcoinLikeAccount::getFreshPublicAddresses() {
                auto keychain = getKeychain();
                return async<AbstractAccount::AddressList>([=] () -> AbstractAccount::AddressList {
                    auto addrs = keychain->getFreshAddresses(BitcoinLikeKeychain::KeyPurpose::RECEIVE, keychain->getObservableRangeSize());
                    AbstractAccount::AddressList result(addrs.size());
                    auto i = 0;
                    for (auto& addr : addrs) {
                        result[i] = std::dynamic_pointer_cast<api::Address>(addr);
                        i += 1;
                    }
                    return result;
                });
            }

            Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> BitcoinLikeAccount::getUTXO() {
                auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
                return getUTXOCount().flatMap<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>>(getContext(), [=] (const int32_t& count) -> Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> {
                    return self->getUTXO(0, count);
                });
            }

            FuturePtr<ledger::core::Amount> BitcoinLikeAccount::getBalance() {
                auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
                return async<std::shared_ptr<Amount>>([=] () -> std::shared_ptr<Amount> {
                    const auto& uid = self->getAccountUid();
                    soci::session sql(self->getWallet()->getDatabase()->getPool());
                    std::vector<BitcoinLikeBlockchainExplorerOutput> utxos;
                    BigInt sum(0);
                    auto keychain = self->getKeychain();
                    std::function<bool (const std::string&)> filter = [&keychain] (const std::string addr) -> bool {
                        return keychain->contains(addr);
                    };
                    BitcoinLikeUTXODatabaseHelper::queryUTXO(sql, uid, 0, std::numeric_limits<int32_t>::max(), utxos, filter);
                    for (const auto& utxo : utxos) {
                        sum = sum + utxo.value;
                    }
                    return std::make_shared<Amount>(self->getWallet()->getCurrency(), 0, sum);
                });
            }

            Future<std::vector<std::shared_ptr<api::Amount>>>
            BitcoinLikeAccount::getBalanceHistory(const std::string &start,
                                                const std::string &end,
                                                api::TimePeriod precision) {
                auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
                return async<std::vector<std::shared_ptr<api::Amount>>>([=]() -> std::vector<std::shared_ptr<api::Amount>> {

                    auto startDate = DateUtils::fromJSON(start);
                    auto endDate = DateUtils::fromJSON(end);
                    if (startDate >= endDate) {
                        throw make_exception(api::ErrorCode::INVALID_DATE_FORMAT,
                                            "Start date should be strictly greater than end date");
                    }

                    const auto &uid = self->getAccountUid();
                    soci::session sql(self->getWallet()->getDatabase()->getPool());
                    std::vector<BitcoinLikeOperation> operations;

                    auto keychain = self->getKeychain();
                    std::function<bool(const std::string &)> filter = [&keychain](const std::string addr) -> bool {
                        return keychain->contains(addr);
                    };

                    //Get operations related to an account
                    BitcoinLikeOperationDatabaseHelper::queryOperations(sql, uid, operations, filter);

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
                                case api::OperationType::NONE:
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

            std::shared_ptr<BitcoinLikeAccount> BitcoinLikeAccount::getSelf() {
                return std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
            }

            void BitcoinLikeAccount::startBlockchainObservation() {
                _observer->registerAccount(getSelf());
            }

            void BitcoinLikeAccount::stopBlockchainObservation() {
                _observer->unregisterAccount(getSelf());
            }

            bool BitcoinLikeAccount::isObservingBlockchain() {
                return _observer->isRegistered(getSelf());
            }

            Future<std::string> BitcoinLikeAccount::broadcastTransaction(const std::vector<uint8_t> &transaction) {
                return _explorer->pushTransaction(transaction).map<std::string>(getContext(), [] (const String& hash) -> std::string {
                    return hash.str();
                });
            }

            // This method is used to get a block height only used to know which
            // update/fork (activated at certain block height) is used.
            // This explains why we take LLONG_MAX is we have no block in DB for
            // certain currency
            // WARNING: please don't use this method if you need an accurate value
            // of last block
            static uint64_t getLastBlockFromDB(soci::session &sql, const std::string &currencyName) {
                //Get last block from DB
                auto lastBlock = BlockDatabaseHelper::getLastBlock(sql, currencyName);
                // If we can not retrieve a last block for currency,
                // we set the returned block height to LLONG_MAX in order to
                // activate/apply last BIP/update/fork for this currency.
                return lastBlock.hasValue() ? static_cast<uint64_t >(lastBlock->height) : LLONG_MAX;
            }

            void BitcoinLikeAccount::broadcastRawTransaction(
				const std::vector<uint8_t> &transaction,
				const std::shared_ptr<api::StringCallback> & callback
			) {
                auto self = getSelf();
                _explorer->pushTransaction(transaction).map<std::string>(getContext(), [self, transaction] (const String& seq) -> std::string {
                    //Store newly broadcasted tx in db
                    //First parse it
                    auto txHash = seq.str();
                    auto optimisticUpdate = Try<int>::from([&] () -> int {
                        //Get last block from DB
                        soci::session sql(self->getWallet()->getDatabase()->getPool());
                        auto lastBlockHeight = getLastBlockFromDB(sql, self->getWallet()->getCurrency().name);

                        auto tx = BitcoinLikeTransaction::parseRawSignedTransaction(self->getWallet()->getCurrency(), transaction, lastBlockHeight);

                        //Get a BitcoinLikeBlockchainExplorerTransaction from a BitcoinLikeTransaction
                        BitcoinLikeBlockchainExplorerTransaction txExplorer;
                        txExplorer.hash = txHash;
                        txExplorer.lockTime = tx->getLockTime();
                        txExplorer.receivedAt = std::chrono::system_clock::now();
                        txExplorer.version = tx->getVersion();
                        txExplorer.confirmations = 0;

                        //Inputs
                        auto inputCount = tx->getInputs().size();
                        for (auto index = 0; index < inputCount; index++) {
                            auto input = tx->getInputs()[index];
                            BitcoinLikeBlockchainExplorerInput in;
                            in.index = index;
                            auto prevTxHash = input->getPreviousTxHash().value_or("");
                            auto prevTxOutputIndex = input->getPreviousOutputIndex().value_or(0);
                            BitcoinLikeBlockchainExplorerTransaction prevTx;
                            if (!BitcoinLikeTransactionDatabaseHelper::getTransactionByHash(sql, prevTxHash, self->getAccountUid(), prevTx) || prevTxOutputIndex >= prevTx.outputs.size()) {
                                throw make_exception(api::ErrorCode::TRANSACTION_NOT_FOUND, "Transaction {} not found while broadcasting", prevTxHash);
                            }
                            in.value = prevTx.outputs[prevTxOutputIndex].value;
                            in.signatureScript = hex::toString(input->getScriptSig());
                            in.previousTxHash = prevTxHash;
                            in.previousTxOutputIndex = prevTxOutputIndex;
                            in.sequence = input->getSequence();
                            in.address = prevTx.outputs[prevTxOutputIndex].address.getValueOr("");
                            txExplorer.inputs.push_back(in);
                        }

                        //Outputs
                        auto keychain = self->getKeychain();
                        auto outputCount = tx->getOutputs().size();
                        for (auto index = 0; index < outputCount; index++) {
                            auto output = tx->getOutputs()[index];
                            BitcoinLikeBlockchainExplorerOutput out;
                            out.value = BigInt(output->getValue()->toString());
                            out.time = DateUtils::toJSON(std::chrono::system_clock::now());
                            out.transactionHash = output->getTransactionHash();
                            out.index = output->getOutputIndex();
                            out.script = hex::toString(output->getScript());
                            out.address = output->getAddress().value_or("");
                            txExplorer.outputs.push_back(out);
                        }

                        //Store in DB
                        return self->putTransaction(sql, txExplorer);
                    });

                    // Failing optimistic update should not throw an exception
                    // because the tx was successfully broadcasted to the network,
                    // and the update will occur at next synchro ...
                    // But still let's log that !
                    if (optimisticUpdate.isFailure()) {
                        self->logger()->warn(" Optimistic update failed for broadcasted transaction : {}", txHash);
                    }

                    return txHash;
                }).callback(getContext(), callback);
            }

            void BitcoinLikeAccount::broadcastTransaction(
				const std::shared_ptr<api::BitcoinLikeTransaction> &transaction,
				const std::shared_ptr<api::StringCallback> & callback
			) {
                broadcastRawTransaction(transaction->serialize(), callback);
            }

            std::shared_ptr<api::BitcoinLikeTransactionBuilder> BitcoinLikeAccount::buildTransaction(std::experimental::optional<bool> partial) {
                auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
                auto getUTXO = [self] () -> Future<std::vector<std::shared_ptr<api::BitcoinLikeOutput>>> {
                    return self->getUTXO();
                };
                auto getTransaction = [self] (const std::string& hash) -> FuturePtr<BitcoinLikeBlockchainExplorerTransaction> {
                    return self->getTransaction(hash);
                };

                soci::session sql(self->getWallet()->getDatabase()->getPool());
                auto lastBlockHeight = getLastBlockFromDB(sql, self->getWallet()->getCurrency().name);

                return std::make_shared<BitcoinLikeTransactionBuilder>(
                        getContext(),
                        getWallet()->getCurrency(),
                        logger(),
                        _picker->getBuildFunction(getUTXO,
                                                getTransaction,
                                                _explorer,
                                                _keychain,
                                                lastBlockHeight,
                                                logger(),
                                                partial.value_or(false))
                );
            }

            const std::shared_ptr<BitcoinLikeBlockchainExplorer> &BitcoinLikeAccount::getExplorer() const {
                return _explorer;
            }

            FuturePtr<ledger::core::BitcoinLikeBlockchainExplorerTransaction> BitcoinLikeAccount::getTransaction(const std::string& hash) {
                auto self = std::dynamic_pointer_cast<BitcoinLikeAccount>(shared_from_this());
                return async<std::shared_ptr<BitcoinLikeBlockchainExplorerTransaction>>([=] () -> std::shared_ptr<BitcoinLikeBlockchainExplorerTransaction> {
                    auto tx = std::make_shared<BitcoinLikeBlockchainExplorerTransaction>();
                    soci::session sql(self->getWallet()->getDatabase()->getPool());
                    if (!BitcoinLikeTransactionDatabaseHelper::getTransactionByHash(sql, hash, self->getAccountUid(), *tx)) {
                        throw make_exception(api::ErrorCode::TRANSACTION_NOT_FOUND, "Transaction {} not found", hash);
                    }
                    return tx;
                });
            }

            std::string BitcoinLikeAccount::getRestoreKey() {
                return _keychain->getRestoreKey();
            }

            Future<api::ErrorCode> BitcoinLikeAccount::eraseDataSince(const std::chrono::system_clock::time_point & date) {
                auto log = logger();

                log->debug(" Start erasing data of account : {}", getAccountUid());

                std::lock_guard<std::mutex> lock(_synchronizationLock);
                 _currentSyncEventBus = nullptr;

                soci::session sql(getWallet()->getDatabase()->getPool());

                // Clear synchronizer state
                eraseSynchronizerDataSince(sql, date);

                auto accountUid = getAccountUid();
                sql << "DELETE FROM operations WHERE account_uid = :account_uid AND date >= :date ", soci::use(accountUid), soci::use(date);
                return Future<api::ErrorCode>::successful(api::ErrorCode::FUTURE_WAS_SUCCESSFULL);
            }

            void BitcoinLikeAccount::getFees(const std::shared_ptr<api::BigIntListCallback> & callback) {
                return _explorer->getFees().callback(getContext(), callback);;
            }

            std::shared_ptr<api::BitcoinLikeAccount> fromCoreAccount(const std::shared_ptr<api::Account> & coreAccount) {
              return std::dynamic_pointer_cast<api::BitcoinLikeAccount>(coreAccount);
            }
        }
    }
