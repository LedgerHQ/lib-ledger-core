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

#include <api/BigIntCallback.hpp>
#include <api/CosmosLikeAddress.hpp>
#include <api/CosmosLikeValidatorListCallback.hpp>
#include <api/StringCallback.hpp>
#include <async/Future.hpp>
#include <cmath>
#include <collections/vector.hpp>
#include <database/query/ConditionQueryFilter.h>
#include <database/soci-date.h>
#include <database/soci-number.h>
#include <database/soci-option.h>
#include <events/Event.hpp>
#include <math/Base58.hpp>
#include <numeric>
#include <soci.h>
#include <utils/DateUtils.hpp>
#include <utils/Option.hpp>
#include <wallet/common/Block.h>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <wallet/common/database/OperationDatabaseHelper.h>
#include <wallet/cosmos/CosmosLikeAccount.hpp>
#include <wallet/cosmos/CosmosLikeConstants.hpp>
#include <wallet/cosmos/CosmosLikeOperationQuery.hpp>
#include <wallet/cosmos/CosmosLikeWallet.hpp>
#include <wallet/cosmos/api_impl/CosmosLikeTransactionApi.hpp>
#include <wallet/cosmos/database/CosmosLikeAccountDatabaseHelper.hpp>
#include <wallet/cosmos/database/CosmosLikeOperationDatabaseHelper.hpp>
#include <wallet/cosmos/database/CosmosLikeTransactionDatabaseHelper.hpp>
#include <wallet/cosmos/explorers/CosmosLikeBlockchainExplorer.hpp>
#include <wallet/cosmos/synchronizers/CosmosLikeAccountSynchronizer.hpp>
#include <wallet/cosmos/transaction_builders/CosmosLikeTransactionBuilder.hpp>
#include <wallet/pool/database/CurrenciesDatabaseHelper.hpp>

using namespace soci;

namespace ledger {
namespace core {

CosmosLikeAccount::CosmosLikeAccount(
    const std::shared_ptr<AbstractWallet> &wallet,
    int32_t index,
    const std::shared_ptr<CosmosLikeBlockchainExplorer> &explorer,
    const std::shared_ptr<CosmosLikeBlockchainObserver> &observer,
    const std::shared_ptr<CosmosLikeAccountSynchronizer> &synchronizer,
    const std::shared_ptr<CosmosLikeKeychain> &keychain) :
    AbstractAccount(wallet, index),
    _explorer(explorer),
    _observer(observer),
    _synchronizer(synchronizer),
    _keychain(keychain),
    _accountData(std::make_shared<cosmos::Account>())
{
    _accountData->pubkey = keychain->getRestoreKey();
}

std::shared_ptr<api::CosmosLikeAccount> CosmosLikeAccount::asCosmosLikeAccount()
{
    auto account = std::dynamic_pointer_cast<CosmosLikeAccount>(shared_from_this());
    account->updateFromDb();
    return account;
}

void CosmosLikeAccount::updateFromDb()
{
    soci::session sql(this->getWallet()->getDatabase()->getPool());
    CosmosLikeAccountDatabaseEntry dbAccount;
    bool const existingAccount =
        CosmosLikeAccountDatabaseHelper::queryAccount(sql, getAccountUid(), dbAccount);
    if (existingAccount) {
        *_accountData = dbAccount.details;
    }
}

std::string CosmosLikeAccount::getAddress() const
{
    return getKeychain()->getAddress()->toBech32();
}

void CosmosLikeAccount::fillOperationTypeAmountFromSend(
    CosmosLikeOperation &out, const cosmos::MsgSend &innerSendMsg) const
{
    const auto address = getAddress();
    const auto &coins = innerSendMsg.amount;
    std::for_each(coins.begin(), coins.end(), [&](cosmos::Coin amount) {
        out.amount = out.amount + BigInt::fromDecimal(amount.amount);
    });
    const auto &sender = innerSendMsg.fromAddress;
    const auto &receiver = innerSendMsg.toAddress;
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
}

void CosmosLikeAccount::fillOperationTypeAmountFromMultiSend(
    CosmosLikeOperation &out, const cosmos::MsgMultiSend &innerMultiSendMsg) const
{
    const auto address = getAddress();
    // Check if the user has more inputs or outputs in the message

    const auto &inputs = innerMultiSendMsg.inputs;
    const auto &outputs = innerMultiSendMsg.outputs;
    BigInt recv_amount;
    BigInt sent_amount;
    std::vector<std::string> senders;
    std::vector<std::string> receivers;
    std::for_each(inputs.begin(), inputs.end(), [&](cosmos::MultiSendInput input) {
        senders.push_back(input.fromAddress);
        if (input.fromAddress == address) {
            std::for_each(input.coins.begin(), input.coins.end(), [&](const cosmos::Coin &amount) {
                sent_amount = sent_amount + BigInt::fromDecimal(amount.amount);
            });
        }
    });

    std::for_each(outputs.begin(), outputs.end(), [&](cosmos::MultiSendOutput output) {
        receivers.push_back(output.toAddress);
        if (output.toAddress == address) {
            std::for_each(
                output.coins.begin(), output.coins.end(), [&](const cosmos::Coin &amount) {
                    recv_amount = recv_amount + BigInt::fromDecimal(amount.amount);
                });
        }
    });

    if (recv_amount == sent_amount) {
        out.amount = BigInt::ZERO;
        out.type = api::OperationType::NONE;
    }
    else if (recv_amount > sent_amount) {
        out.amount = recv_amount - sent_amount;
        out.type = api::OperationType::RECEIVE;
    }
    else {  //  sent_amount > recv_amount
        out.amount = sent_amount - recv_amount;
        out.type = api::OperationType::SEND;
    }

    out.senders = senders;
    out.recipients = receivers;
}

void CosmosLikeAccount::fillOperationTypeAmountFromDelegate(
    CosmosLikeOperation &out, const cosmos::MsgDelegate &innerDelegateMsg) const
{
    out.senders = {getAddress()};
    out.amount = innerDelegateMsg.amount.amount;
    out.type = api::OperationType::NONE;
}

void CosmosLikeAccount::fillOperationTypeAmountFromUndelegate(
    CosmosLikeOperation &out, const cosmos::MsgUndelegate &innerUndelegateMsg) const
{
    out.senders = {getAddress()};
    out.amount = innerUndelegateMsg.amount.amount;
    out.type = api::OperationType::NONE;
}

void CosmosLikeAccount::fillOperationTypeAmountFromBeginRedelegate(
    CosmosLikeOperation &out, const cosmos::MsgBeginRedelegate &innerBeginRedelegateMsg) const
{
    out.senders = {getAddress()};
    out.amount = innerBeginRedelegateMsg.amount.amount;
    out.type = api::OperationType::NONE;
}

void CosmosLikeAccount::fillOperationTypeAmountFromSubmitProposal(
    CosmosLikeOperation &out, const cosmos::MsgSubmitProposal &innerSubmitProposalMsg) const
{
    const auto address = getAddress();
    const auto &coins = innerSubmitProposalMsg.initialDeposit;
    std::for_each(coins.begin(), coins.end(), [&](const cosmos::Coin &amount) {
        out.amount = out.amount + BigInt::fromDecimal(amount.amount);
    });
    const auto &sender = innerSubmitProposalMsg.proposer;
    out.senders = {sender};
    if (sender == address) {
        out.type = api::OperationType::SEND;
    }
    out.type = api::OperationType::NONE;
}

void CosmosLikeAccount::fillOperationTypeAmountFromDeposit(
    CosmosLikeOperation &out, const cosmos::MsgDeposit &innerDepositMsg) const
{
    const auto address = getAddress();
    const auto &coins = innerDepositMsg.amount;
    std::for_each(coins.begin(), coins.end(), [&](cosmos::Coin amount) {
        out.amount = out.amount + BigInt::fromDecimal(amount.amount);
    });
    const auto &sender = innerDepositMsg.depositor;
    out.senders = {sender};
    if (sender == address) {
        out.type = api::OperationType::SEND;
    }
    out.type = api::OperationType::NONE;
}

void CosmosLikeAccount::fillOperationTypeAmountFromFees(
    CosmosLikeOperation &out, const cosmos::MsgFees &innerFeesMsg) const
{
    const auto address = getAddress();
    out.amount = BigInt::fromDecimal(innerFeesMsg.fees.amount);
    if (innerFeesMsg.payerAddress == address) {
        out.type = api::OperationType::SEND;
    }
    else {
        out.type = api::OperationType::NONE;
    }
    out.senders = {innerFeesMsg.payerAddress};
}

void CosmosLikeAccount::setOperationTypeAndAmount(
    CosmosLikeOperation &out, const cosmos::Message &msg) const
{
    const auto address = getAddress();
    switch (cosmos::stringToMsgType(msg.type.c_str())) {
    case api::CosmosLikeMsgType::MSGSEND: {
        fillOperationTypeAmountFromSend(out, boost::get<cosmos::MsgSend>(msg.content));
    } break;
    case api::CosmosLikeMsgType::MSGMULTISEND: {
        fillOperationTypeAmountFromMultiSend(out, boost::get<cosmos::MsgMultiSend>(msg.content));
    } break;
    case api::CosmosLikeMsgType::MSGDELEGATE: {
        fillOperationTypeAmountFromDelegate(out, boost::get<cosmos::MsgDelegate>(msg.content));
    } break;
    case api::CosmosLikeMsgType::MSGUNDELEGATE: {
        fillOperationTypeAmountFromUndelegate(out, boost::get<cosmos::MsgUndelegate>(msg.content));
    } break;
    case api::CosmosLikeMsgType::MSGBEGINREDELEGATE: {
        fillOperationTypeAmountFromBeginRedelegate(
            out, boost::get<cosmos::MsgBeginRedelegate>(msg.content));
    } break;
    case api::CosmosLikeMsgType::MSGSUBMITPROPOSAL: {
        fillOperationTypeAmountFromSubmitProposal(
            out, boost::get<cosmos::MsgSubmitProposal>(msg.content));
    } break;
    case api::CosmosLikeMsgType::MSGDEPOSIT: {
        fillOperationTypeAmountFromDeposit(out, boost::get<cosmos::MsgDeposit>(msg.content));
    } break;
    case api::CosmosLikeMsgType::MSGFEES: {
        fillOperationTypeAmountFromFees(out, boost::get<cosmos::MsgFees>(msg.content));
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

uint32_t CosmosLikeAccount::computeFeesForTransaction(const cosmos::Transaction &tx)
{
    // Get the maximum fees advertised on transaction (if all of "GasWanted" is used)
    auto fees = std::accumulate(
        std::begin(tx.fee.amount),
        std::end(tx.fee.amount),
        0,
        [](uint32_t sum, const cosmos::Coin &subamount) {
            // FIXME This locks the CosmosLikeAccount code in cosmoshub chains on debug builds.
            // And it also serves as reminder that most code in this module assumes cosmoshub.
            assert((subamount.denom == "uatom"));
            return sum + BigInt::fromDecimal(subamount.amount).toInt();
        });

    return fees;
}

void CosmosLikeAccount::inflateOperation(
    CosmosLikeOperation &out,
    const std::shared_ptr<const AbstractWallet> &wallet,
    const cosmos::Transaction &tx,
    const cosmos::Message &msg)
{
    out.setTransactionData(tx);
    out.setMessageData(msg);

    setOperationTypeAndAmount(out, msg);

    // out._account = shared_from_this();
    out.cosmosTransaction = Option<cosmos::OperationQueryResult>({tx, msg});
    out.accountUid = getAccountUid();

    if (tx.block) {
        out.block = Option<Block>(Block(tx.block.getValue()));
    }
    else {
        out.block = Option<Block>();
    }
    if (out.block.nonEmpty()) {
        out.block.getValue().currencyName = wallet->getCurrency().name;
    }
    out.cosmosTransaction.getValue().tx.block = out.block;
    out.currencyName = getWallet()->getCurrency().name;
    out.date = tx.timestamp;
    out.trust = std::make_shared<TrustIndicator>();
    // Fees are computed as the amount only on the MSGFEES Message Type.
    if (cosmos::stringToMsgType(msg.type.c_str()) == api::CosmosLikeMsgType::MSGFEES) {
        auto txFees = computeFeesForTransaction(tx);
        out.amount = BigInt(txFees);
    }
    out.fees = BigInt::ZERO;
    out.walletUid = wallet->getWalletUid();
}

int CosmosLikeAccount::putTransaction(soci::session &sql, const cosmos::Transaction &transaction)
{
    // FIXME Design issue: 'transaction' being const (from
    // AbstractBlockchainObserver::putTransaction) it makes it impossible to manage uids of nested
    // objects (eg. cosmos::Message). Writable copy of tx to allow to add uids.
    auto tx = transaction;

    auto wallet = getWallet();
    if (wallet == nullptr) {
        throw Exception(api::ErrorCode::RUNTIME_ERROR, "Wallet reference is dead.");
    }

    if (tx.block.nonEmpty()) {
        putBlock(sql, tx.block.getValue().toApiBlock());
    }

    int result = FLAG_TRANSACTION_IGNORED;
    CosmosLikeTransactionDatabaseHelper::putTransaction(sql, getAccountUid(), tx);

    for (auto msgIndex = 0; msgIndex < tx.messages.size(); msgIndex++) {
        auto msg = tx.messages[msgIndex];

        // Ignore the operation if this is a Fee operation, that _this_ Account did not pay
        // inflateOperation adds the AccountUID in the operation otherwise, so when querying
        // operations for this account, the fees that _this_ didn't pay will appear.
        if (cosmos::stringToMsgType(msg.type.c_str()) == cosmos::MsgType::MSGFEES &&
            boost::get<cosmos::MsgFees>(msg.content).payerAddress != this->getAddress()) {
            continue;
        }

        CosmosLikeOperation operation(tx, msg);
        inflateOperation(operation, getWallet(), tx, msg);
        operation.refreshUid(std::to_string(msgIndex));

        auto inserted = CosmosLikeOperationDatabaseHelper::putOperation(sql, operation);
        if (inserted) {
            CosmosLikeOperationDatabaseHelper::updateOperation(sql, operation.uid, msg.uid);
            emitNewOperationEvent(operation);
        }
        result = static_cast<int>(operation.type);
    }

    return result;
}

bool CosmosLikeAccount::putBlock(soci::session &sql, const api::Block &block)
{
    if (BlockDatabaseHelper::putBlock(sql, block)) {
        emitNewBlockEvent(block);
        return true;
    }
    return false;
}

std::shared_ptr<CosmosLikeKeychain> CosmosLikeAccount::getKeychain() const
{
    return _keychain;
}

std::shared_ptr<api::Keychain> CosmosLikeAccount::getAccountKeychain()
{
    return this->getKeychain();
}

FuturePtr<Amount> CosmosLikeAccount::getBalance()
{
    return getTotalBalanceWithoutPendingRewards();
}

std::shared_ptr<api::OperationQuery> CosmosLikeAccount::queryOperations()
{
    auto headFilter = api::QueryFilter::accountEq(getAccountUid());
    auto query = std::make_shared<OperationQuery>(
        headFilter,
        getWallet()->getDatabase(),
        getWallet()->getContext(),
        getWallet()->getMainExecutionContext());
    query->registerAccount(shared_from_this());
    return query;
}

void CosmosLikeAccount::getEstimatedGasLimit(
    const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
    const std::shared_ptr<api::BigIntCallback> &callback)
{
    _explorer->getEstimatedGasLimit(transaction)
        .mapPtr<api::BigInt>(
            getContext(),
            [](const std::shared_ptr<BigInt> &gasLimit) -> std::shared_ptr<api::BigInt> {
                return std::make_shared<api::BigIntImpl>(*gasLimit);
            })
        .callback(getContext(), callback);
}

Future<AbstractAccount::AddressList> CosmosLikeAccount::getFreshPublicAddresses()
{
    auto keychain = getKeychain();
    return async<AbstractAccount::AddressList>([=]() -> AbstractAccount::AddressList {
        AbstractAccount::AddressList result{keychain->getAddress()};
        return result;
    });
}

Future<std::vector<std::shared_ptr<api::Amount>>> CosmosLikeAccount::getBalanceHistory(
    const std::string &start, const std::string &end, api::TimePeriod precision)
{
    auto self = std::dynamic_pointer_cast<CosmosLikeAccount>(shared_from_this());
    return async<std::vector<std::shared_ptr<api::Amount>>>(
        [=]() -> std::vector<std::shared_ptr<api::Amount>> {
            auto startDate = DateUtils::fromJSON(start);
            auto endDate = DateUtils::fromJSON(end);
            if (startDate >= endDate) {
                throw make_exception(
                    api::ErrorCode::INVALID_DATE_FORMAT,
                    "Start date should be strictly before end date");
            }

            const auto &uid = self->getAccountUid();
            soci::session sql(self->getWallet()->getDatabase()->getPool());
            std::vector<Operation> operations;

            auto keychain = self->getKeychain();
            std::function<bool(const std::string &)> filter =
                [&keychain](const std::string addr) -> bool {
                return keychain->contains(addr);
            };

            // Get operations related to an account
            CosmosLikeOperationDatabaseHelper::queryOperations(sql, uid, operations, filter);

            auto lowerDate = startDate;
            auto upperDate = DateUtils::incrementDate(startDate, precision);

            std::vector<std::shared_ptr<api::Amount>> amounts;
            std::size_t operationsCount = 0;
            BigInt sum;
            while (lowerDate <= endDate && operationsCount < operations.size()) {
                auto operation = operations[operationsCount];
                // Fill the time slices until next operation with the running value of the
                // accumulator
                while (operation.date > upperDate && lowerDate < endDate) {
                    lowerDate = DateUtils::incrementDate(lowerDate, precision);
                    upperDate = DateUtils::incrementDate(upperDate, precision);
                    amounts.emplace_back(std::make_shared<ledger::core::Amount>(
                        self->getWallet()->getCurrency(), 0, sum));
                }

                if (operation.date <= upperDate) {
                    switch (operation.type) {
                    case api::OperationType::RECEIVE: {
                        sum = sum + operation.amount;
                        break;
                    }
                    case api::OperationType::SEND: {
                        // NOTE : in Cosmos, FEES is a SEND operation with fees as amount and 0 fees
                        sum = sum - operation.amount - operation.fees.getValueOr(BigInt::ZERO);
                        break;
                    }
                    default: {
                        // NOTE : we ignore the fees field here as well, since the fees paid by the Account
                        // were already included in an OperationType::SEND
                        // See CosmosLikeAccount::fillOperationTypeAmountFromFees
                        // Therefore, nothing to do on default: case.
                    } break;
                    }
                }
                operationsCount += 1;
            }

            // Fill the remainder of the period with constant time slices using end value of the
            // accumulator
            while (lowerDate < endDate) {
                lowerDate = DateUtils::incrementDate(lowerDate, precision);
                amounts.emplace_back(std::make_shared<ledger::core::Amount>(
                    self->getWallet()->getCurrency(), 0, sum));
            }

            return amounts;
        });
}

Future<api::ErrorCode> CosmosLikeAccount::eraseDataSince(
    const std::chrono::system_clock::time_point &date)
{
    auto log = logger();
    log->debug(" Start erasing data of account : {}", getAccountUid());
    soci::session sql(getWallet()->getDatabase()->getPool());
    // Update account's internal preferences (for synchronization)
    auto savedState = getInternalPreferences()
                          ->getSubPreferences("CosmosLikeAccountSynchronizer")
                          ->getObject<cosmos::AccountSynchronizationSavedState>("state");
    if (savedState.nonEmpty()) {
        // Reset batches to blocks mined before given date
        auto previousBlock = BlockDatabaseHelper::getPreviousBlockInDatabase(
            sql, getWallet()->getCurrency().name, date);
        for (auto &batch : savedState.getValue().batches) {
            if (previousBlock.nonEmpty() && batch.blockHeight > previousBlock.getValue().height) {
                batch.blockHeight = (uint32_t)previousBlock.getValue().height;
                batch.blockHash = previousBlock.getValue().blockHash;
            }
            else if (!previousBlock.nonEmpty()) {  // if no previous block, sync should go back from
                                                   // genesis block
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
    sql << "DELETE FROM operations WHERE account_uid = :account_uid AND date >= :date ",
        soci::use(accountUid), soci::use(date);
    log->debug(" Finish erasing data of account : {}", accountUid);
    return Future<api::ErrorCode>::successful(api::ErrorCode::FUTURE_WAS_SUCCESSFULL);
}

bool CosmosLikeAccount::isSynchronizing()
{
    std::lock_guard<std::mutex> lock(_synchronizationLock);
    return _currentSyncEventBus != nullptr;
}

std::shared_ptr<api::EventBus> CosmosLikeAccount::synchronize()
{
    std::lock_guard<std::mutex> lock(_synchronizationLock);
    if (_currentSyncEventBus)
        return _currentSyncEventBus;
    auto eventPublisher = std::make_shared<EventPublisher>(getContext());

    _currentSyncEventBus = eventPublisher->getEventBus();
    auto future =
        _synchronizer
            ->synchronizeAccount(std::static_pointer_cast<CosmosLikeAccount>(shared_from_this()))
            ->getFuture();
    auto self = std::static_pointer_cast<CosmosLikeAccount>(shared_from_this());

    // Update current block height (needed to compute trust level)
    _explorer->getCurrentBlock().onComplete(
        getContext(), [self](const TryPtr<cosmos::Block> &block) mutable {
            if (block.isSuccess()) {
                self->_currentBlockHeight = block.getValue()->height;
            }
        });

    // Update account level data (sequence, accountnumber...)
    // Example result from Gaia explorer :
    // base_url/auth/accounts/{address} with a valid, 0 transaction address :
    // {
    //  "height": "1296656",
    //  "result": {
    //    "type": "cosmos-sdk/Account",
    //    "value": {
    //      "address": "",
    //      "coins": [],
    //      "public_key": null,
    //      "account_number": "0",
    //      "sequence": "0"
    //    }
    //  }
    //}
    _explorer->getAccount(getAddress())
        .onComplete(getContext(), [self](const TryPtr<cosmos::Account> &accountData) mutable {
            if (accountData.isSuccess()) {
                self->_accountData = accountData.getValue();
                const CosmosLikeAccountDatabaseEntry update = {
                    0,  // unused in
                        // CosmosLikeAccountDatabaseHelper::updateAccount
                    "",  // unused in
                         // CosmosLikeAccountDatabaseHelper::updateAccount
                    *(self->_accountData),
                    std::chrono::system_clock::now()};
                soci::session sql(self->getWallet()->getDatabase()->getPool());
                CosmosLikeAccountDatabaseHelper::updateAccount(sql, self->getAccountUid(), update);
            }
        });

    auto startTime = DateUtils::now();
    eventPublisher->postSticky(
        std::make_shared<Event>(
            api::EventCode::SYNCHRONIZATION_STARTED, api::DynamicObject::newInstance()),
        0);
    future.onComplete(getContext(), [eventPublisher, self, startTime](const Try<Unit> &result) {
        api::EventCode code;
        auto payload = std::make_shared<DynamicObject>();
        auto duration =
            std::chrono::duration_cast<std::chrono::milliseconds>(DateUtils::now() - startTime)
                .count();
        payload->putLong(api::Account::EV_SYNC_DURATION_MS, duration);
        if (result.isSuccess()) {
            code = api::EventCode::SYNCHRONIZATION_SUCCEED;
        }
        else {
            code = api::EventCode::SYNCHRONIZATION_FAILED;
            payload->putString(
                api::Account::EV_SYNC_ERROR_CODE,
                api::to_string(result.getFailure().getErrorCode()));
            payload->putInt(
                api::Account::EV_SYNC_ERROR_CODE_INT, (int32_t)result.getFailure().getErrorCode());
            payload->putString(
                api::Account::EV_SYNC_ERROR_MESSAGE, result.getFailure().getMessage());
        }
        eventPublisher->postSticky(std::make_shared<Event>(code, payload), 0);
        std::lock_guard<std::mutex> lock(self->_synchronizationLock);
        self->_currentSyncEventBus = nullptr;
    });
    return eventPublisher->getEventBus();
}

std::shared_ptr<CosmosLikeAccount> CosmosLikeAccount::getSelf()
{
    return std::dynamic_pointer_cast<CosmosLikeAccount>(shared_from_this());
}

void CosmosLikeAccount::startBlockchainObservation()
{
    _observer->registerAccount(getSelf());
}

void CosmosLikeAccount::stopBlockchainObservation()
{
    _observer->unregisterAccount(getSelf());
}

bool CosmosLikeAccount::isObservingBlockchain()
{
    return _observer->isRegistered(getSelf());
}

std::string CosmosLikeAccount::getRestoreKey()
{
    return _keychain->getRestoreKey();
}

void CosmosLikeAccount::broadcastRawTransaction(
    const std::string &transaction, const std::shared_ptr<api::StringCallback> &callback)
{
    std::vector<uint8_t> tx{transaction.begin(), transaction.end()};
    _explorer->pushTransaction(tx)
        .map<std::string>(
            getContext(),
            [](const String &seq) -> std::string {
                // With 7-10 seconds blocks, we skip optimistic update
                // client code can just use the "block" mechanism to get
                // all transaction information back from the push
                return seq.str();
            })
        .callback(getMainExecutionContext(), callback);
}

void CosmosLikeAccount::broadcastTransaction(
    const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
    const std::shared_ptr<api::StringCallback> &callback)
{
    broadcastRawTransaction(transaction->serializeForBroadcast("block"), callback);
}

std::shared_ptr<api::CosmosLikeTransactionBuilder> CosmosLikeAccount::buildTransaction()
{
    return buildTransaction(
        std::dynamic_pointer_cast<CosmosLikeAddress>(getKeychain()->getAddress())->toString());
}

std::shared_ptr<api::CosmosLikeTransactionBuilder> CosmosLikeAccount::buildTransaction(
    const std::string &senderAddress)
{
    auto self = std::dynamic_pointer_cast<CosmosLikeAccount>(shared_from_this());
    auto buildFunction = [self](const CosmosLikeTransactionBuildRequest &request) {
        auto currency = self->getWallet()->getCurrency();
        auto tx = std::make_shared<CosmosLikeTransactionApi>();
        tx->setAccountNumber(self->_accountData->accountNumber);
        tx->setCurrency(self->getWallet()->getCurrency());
        tx->setFee(request.fee);
        tx->setMemo(request.memo);
        tx->setMessages(request.messages);
        tx->setSequence(
            request.sequence.empty() ? std::to_string(std::stoi(self->_accountData->sequence) + 1)
                                     : request.sequence);
        tx->setSigningPubKey(self->getKeychain()->getPublicKey());
        if (request.gas && !(request.gas->isZero())) {
            tx->setGas(request.gas);
            return Future<std::shared_ptr<api::CosmosLikeTransaction>>::successful(tx);
        }
        return self->_explorer->getEstimatedGasLimit(tx, request.gasAdjustment)
            .mapPtr<api::CosmosLikeTransaction>(
                self->getContext(), [tx](const std::shared_ptr<BigInt> &estimateValue) {
                    tx->setGas(estimateValue);
                    return tx;
                });
    };

    return std::make_shared<CosmosLikeTransactionBuilder>(getContext(), buildFunction);
}

void CosmosLikeAccount::estimateGas(
    const api::CosmosGasLimitRequest &request, const std::shared_ptr<api::BigIntCallback> &callback)
{
    auto tx = std::make_shared<CosmosLikeTransactionApi>();
    tx->setAccountNumber(_accountData->accountNumber);
    tx->setCurrency(getWallet()->getCurrency());
    tx->setMessages(request.messages);
    // Sequence needs to be set or explorer might return an error.
    // The value here doesn't matter at all.
    tx->setSequence("0");
    if (request.memo) {
        tx->setMemo(request.memo.value());
    }
    return _explorer->getEstimatedGasLimit(tx, request.amplifier.value_or(1.0))
        .mapPtr<api::BigInt>(
            getContext(),
            [](const std::shared_ptr<BigInt> &gasLimit) -> std::shared_ptr<api::BigInt> {
                return std::make_shared<api::BigIntImpl>(*gasLimit);
            })
        .callback(getContext(), callback);
}

void CosmosLikeAccount::getSequence(const std::shared_ptr<api::StringCallback> &callback)
{
    if (!_accountData) {
        throw make_exception(api::ErrorCode::ILLEGAL_STATE, "account must be synchronized first");
    }
    return Future<std::string>::successful(_accountData->sequence).callback(getContext(), callback);
}

void CosmosLikeAccount::getAccountNumber(const std::shared_ptr<api::StringCallback> &callback)
{
    if (!_accountData) {
        throw make_exception(api::ErrorCode::ILLEGAL_STATE, "account must be synchronized first");
    }
    return Future<std::string>::successful(_accountData->accountNumber)
        .callback(getContext(), callback);
}

void CosmosLikeAccount::getWithdrawAddress(const std::shared_ptr<api::StringCallback> &callback)
{
    if (!_accountData) {
        throw make_exception(api::ErrorCode::ILLEGAL_STATE, "account must be synchronized first");
    }
    return Future<std::string>::successful(_accountData->withdrawAddress)
        .callback(getContext(), callback);
}

cosmos::Account CosmosLikeAccount::getInfo() const
{
    return cosmos::Account(*_accountData);
}

FuturePtr<Amount> CosmosLikeAccount::getTotalBalance() const
{
    auto currency = getWallet()->getCurrency();
    return _explorer->getTotalBalance(_keychain->getAddress()->toBech32())
        .mapPtr<Amount>(getContext(), [currency](const auto &rawAmount) {
            return std::make_shared<Amount>(currency, 0, *rawAmount);
        });
}

void CosmosLikeAccount::getTotalBalance(const std::shared_ptr<api::AmountCallback> &callback)
{
    getTotalBalance().callback(getContext(), callback);
}

FuturePtr<Amount> CosmosLikeAccount::getTotalBalanceWithoutPendingRewards() const
{
    auto currency = getWallet()->getCurrency();
    return _explorer->getTotalBalanceWithoutPendingRewards(_keychain->getAddress()->toBech32())
        .mapPtr<Amount>(getContext(), [currency](const auto &rawAmount) {
            return std::make_shared<Amount>(currency, 0, *rawAmount);
        });
}

FuturePtr<Amount> CosmosLikeAccount::getDelegatedBalance() const
{
    auto currency = getWallet()->getCurrency();
    return _explorer->getDelegatedBalance(_keychain->getAddress()->toBech32())
        .mapPtr<Amount>(getContext(), [currency](const auto &rawAmount) {
            return std::make_shared<Amount>(currency, 0, *rawAmount);
        });
}

void CosmosLikeAccount::getDelegatedBalance(const std::shared_ptr<api::AmountCallback> &callback)
{
    getDelegatedBalance().callback(getContext(), callback);
}

FuturePtr<Amount> CosmosLikeAccount::getPendingRewardsBalance() const
{
    auto currency = getWallet()->getCurrency();
    return _explorer->getPendingRewardsBalance(_keychain->getAddress()->toBech32())
        .mapPtr<Amount>(getContext(), [currency](const auto &rawAmount) {
            return std::make_shared<Amount>(currency, 0, *rawAmount);
        });
}

void CosmosLikeAccount::getPendingRewardsBalance(
    const std::shared_ptr<api::AmountCallback> &callback)
{
    getPendingRewardsBalance().callback(getContext(), callback);
}

FuturePtr<Amount> CosmosLikeAccount::getUnbondingBalance() const
{
    auto currency = getWallet()->getCurrency();
    return _explorer->getUnbondingBalance(getAddress())
        .mapPtr<Amount>(getContext(), [currency](const auto &rawAmount) {
            return std::make_shared<Amount>(currency, 0, *rawAmount);
        });
}
void CosmosLikeAccount::getUnbondingBalance(const std::shared_ptr<api::AmountCallback> &callback)
{
    getUnbondingBalance().callback(getContext(), callback);
}

FuturePtr<Amount> CosmosLikeAccount::getSpendableBalance() const
{
    auto currency = getWallet()->getCurrency();
    return _explorer->getSpendableBalance(getAddress())
        .mapPtr<Amount>(getContext(), [currency](const auto &rawAmount) {
            return std::make_shared<Amount>(currency, 0, *rawAmount);
        });
}
void CosmosLikeAccount::getSpendableBalance(const std::shared_ptr<api::AmountCallback> &callback)
{
    getSpendableBalance().callback(getContext(), callback);
}

Future<cosmos::ValidatorList> CosmosLikeAccount::getActiveValidatorSet() const
{
    return _explorer->getActiveValidatorSet();
}
void CosmosLikeAccount::getLatestValidatorSet(
    const std::shared_ptr<api::CosmosLikeValidatorListCallback> &callback)
{
    getActiveValidatorSet().callback(getContext(), callback);
}

Future<cosmos::Validator> CosmosLikeAccount::getValidatorInfo(
    const std::string &validatorAddress) const
{
    return _explorer->getValidatorInfo(validatorAddress);
}

void CosmosLikeAccount::getValidatorInfo(
    const std::string &validatorAddress,
    const std::shared_ptr<api::CosmosLikeValidatorCallback> &callback)
{
    getValidatorInfo(validatorAddress).callback(getContext(), callback);
}

void CosmosLikeAccount::getDelegations(
    const std::shared_ptr<api::CosmosLikeDelegationListCallback> &callback)
{
    getDelegations().callback(getMainExecutionContext(), callback);
}

Future<std::vector<std::shared_ptr<api::CosmosLikeDelegation>>> CosmosLikeAccount::getDelegations()
{
    return _explorer->getDelegations(getAddress())
        .map<std::vector<std::shared_ptr<api::CosmosLikeDelegation>>>(
            getContext(), [](auto &delegations) {
                std::vector<std::shared_ptr<api::CosmosLikeDelegation>> delegationList;
                for (auto &delegation : *delegations) {
                    delegationList.push_back(std::make_shared<CosmosLikeDelegation>(delegation));
                }
                return delegationList;
            });
}

void CosmosLikeAccount::getPendingRewards(
    const std::shared_ptr<api::CosmosLikeRewardListCallback> &callback)
{
    getPendingRewards().callback(getMainExecutionContext(), callback);
}

Future<std::vector<std::shared_ptr<api::CosmosLikeReward>>> CosmosLikeAccount::getPendingRewards()
{
    return _explorer->getPendingRewards(getAddress())
        .map<std::vector<std::shared_ptr<api::CosmosLikeReward>>>(getContext(), [&](auto &rewards) {
            std::vector<std::shared_ptr<api::CosmosLikeReward>> rewardList;
            for (auto &reward : *rewards) {
                rewardList.push_back(
                    std::make_shared<CosmosLikeReward>(reward, this->getAddress()));
            }
            return rewardList;
        });
}

Future<std::vector<std::shared_ptr<api::CosmosLikeUnbonding>>> CosmosLikeAccount::getUnbondings()
    const
{
    return _explorer->getUnbondingsByDelegator(getAddress())
        .map<std::vector<std::shared_ptr<api::CosmosLikeUnbonding>>>(
            getContext(), [&](const auto &explorerUnbondings) {
                std::vector<std::shared_ptr<api::CosmosLikeUnbonding>> unbondingList;
                for (const auto &unbonding : explorerUnbondings) {
                    unbondingList.push_back(std::make_shared<CosmosLikeUnbonding>(*unbonding));
                }
                return unbondingList;
            });
}

void CosmosLikeAccount::getUnbondings(
    const std::shared_ptr<api::CosmosLikeUnbondingListCallback> &callback)
{
    getUnbondings().callback(getContext(), callback);
}

Future<std::vector<std::shared_ptr<api::CosmosLikeRedelegation>>>
CosmosLikeAccount::getRedelegations() const
{
    return _explorer->getRedelegationsByDelegator(getAddress())
        .map<std::vector<std::shared_ptr<api::CosmosLikeRedelegation>>>(
            getContext(), [&](const auto &explorerRedelegations) {
                std::vector<std::shared_ptr<api::CosmosLikeRedelegation>> redelegationList;
                for (const auto &redelegation : explorerRedelegations) {
                    redelegationList.push_back(
                        std::make_shared<CosmosLikeRedelegation>(*redelegation));
                }
                return redelegationList;
            });
}

void CosmosLikeAccount::getRedelegations(
    const std::shared_ptr<api::CosmosLikeRedelegationListCallback> &callback)
{
    getRedelegations().callback(getContext(), callback);
}
}  // namespace core
}  // namespace ledger
