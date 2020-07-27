/*
 * AlgorandAccount
 *
 * Created by Rémi Barjon on 15/05/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include "AlgorandAccount.hpp"
#include "database/AlgorandTransactionDatabaseHelper.hpp"
#include "database/AlgorandOperationDatabaseHelper.hpp"
#include "model/AlgorandModelMapper.hpp"
#include "operations/AlgorandOperation.hpp"
#include "transactions/api_impl/AlgorandTransactionImpl.hpp"

#include <api/AlgorandAssetAmount.hpp>
#include <api/AlgorandAssetParams.hpp>

#include <database/soci-date.h>
#include <events/Event.hpp>
#include <api/BigInt.hpp>
#include <utils/DateUtils.hpp>
#include <utils/hex.h>

#include <soci.h>

#include <chrono>
#include <memory>
#include <unordered_map>
#include <vector>

namespace ledger {
namespace core {
namespace algorand {

    Account::Account(const std::shared_ptr<AbstractWallet>& wallet,
                     int32_t index,
                     const api::Currency& currency,
                     const std::string& address,
                     std::shared_ptr<BlockchainExplorer> explorer,
                     std::shared_ptr<BlockchainObserver> observer,
                     std::shared_ptr<AccountSynchronizer> synchronizer)
        : AbstractAccount(wallet, index)
        , _address(currency, address)
        , _explorer(std::move(explorer))
        , _observer(std::move(observer))
        , _synchronizer(std::move(synchronizer))
    {}

    namespace {

        auto u64ToAmount(const api::Currency& currency)
        {
            return [currency](uint64_t amount) {
                return std::make_shared<Amount>(
                        currency,
                        0,
                        BigInt(static_cast<unsigned long long>(amount))
                );
            };
        }

        uint64_t computeMinimumBalance(
                const model::Account& account,
                api::AlgorandOperationType operationType)
        {
            constexpr uint64_t base = 100000; // 0.1 algo = 100000 malgo
            const auto nbAssets =
                account.assetsAmounts.size() +
                (operationType == api::AlgorandOperationType::ASSET_OPT_IN ? 1 : 0);
            return base * (1 + nbAssets);
        }

    } // namespace

    bool Account::putBlock(soci::session& sql, const api::Block& block)
    {
        if (BlockDatabaseHelper::putBlock(sql, block)) {
            emitNewBlockEvent(block);
            return true;
        }
        return false;
    }

    int Account::putTransaction(soci::session &sql, const model::Transaction &transaction)
    {
        const auto wallet = getWallet();
        if (wallet == nullptr)
        {
            throw Exception(api::ErrorCode::RUNTIME_ERROR, "Wallet reference is dead.");
        }
        const auto txuid = TransactionDatabaseHelper::putTransaction(sql, getAccountUid(), transaction);
        const auto operation = Operation(shared_from_this(), transaction);

        if (OperationDatabaseHelper::putAlgorandOperation(sql, txuid, operation)) {
            emitNewOperationEvent(operation.getBackend());
        }
        return static_cast<int>(operation.getBackend().type);
    }

    void Account::getSpendableBalance(
            api::AlgorandOperationType operationType,
            const std::shared_ptr<api::AmountCallback>& callback)
    {
        getAccountInformation()
            .map<uint64_t>(
                    getContext(),
                    [operationType](const model::Account& account) {
                        const auto minBalance = computeMinimumBalance(account, operationType);
                        const auto balance = account.amount;
                        if (balance >= minBalance) {
                            return balance - minBalance;
                        }
                        return uint64_t{0};
                    }
            ).mapPtr<Amount>(
                    getContext(),
                    u64ToAmount(_address.getCurrency())
            ).callback(getMainExecutionContext(), callback);
    }

    void Account::getAsset(
            const std::string& assetId,
            const std::shared_ptr<api::AlgorandAssetParamsCallback>& callback)
    {
        _explorer->getAssetById(std::stoull(assetId))
            .map<api::AlgorandAssetParams>(
                    getContext(),
                    [assetId](const model::AssetParams& params) {
                        auto apiParams = model::toAPI(params);
                        apiParams.assetId = assetId;
                        return apiParams;
                    })
            .callback(getMainExecutionContext(), callback);
    }

    void Account::hasAsset(const std::string & addr, const std::string & assetId, const std::shared_ptr<api::BoolCallback> & callback)
    {
        _explorer->getAccount(addr)
            .map<bool>(
                    getContext(),
                    [assetId](const model::Account& account) {
                        const auto id = std::stoull(assetId);
                        return account.assetsAmounts.find(id) != std::end(account.assetsAmounts);
                    })
            .callback(getMainExecutionContext(), callback);
    }

    void Account::isAmountValid(const std::string & addr, const std::string & amount, const std::shared_ptr<api::BoolCallback> & callback) const
    {
        isAmountValid(addr, amount).callback(getMainExecutionContext(), callback);
    }

    Future<bool> Account::isAmountValid(const std::string & addr, const std::string & amount) const
    {
        return _explorer->getAccount(addr)
            .map<bool>(
                getContext(),
                [amount](const model::Account& account) {
                    if (account.amount == 0) {
                        constexpr uint64_t minValue = 100000;
                        return std::stoull(amount) >= minValue;
                    }
                    return true;
                });
    }

    void Account::getAssetBalance(
            const std::string& assetId,
            const std::shared_ptr<api::AlgorandAssetAmountCallback>& callback)
    {
        getAccountInformation()
            .map<api::AlgorandAssetAmount>(
                    getContext(),
                    [assetId](const model::Account& account) {
                        const auto id = std::stoull(assetId);
                        return model::toAPI(account.assetsAmounts.at(id));
                    })
            .callback(getMainExecutionContext(), callback);
    }

    Future<std::vector<api::AlgorandAssetAmount>> Account::getAssetBalanceHistory(
            const std::string& assetId,
            const std::string& start,
            const std::string& end,
            api::TimePeriod period)
    {
        const auto startDate = DateUtils::fromJSON(start);
        const auto endDate = DateUtils::fromJSON(end);
        if (startDate >= endDate) {
            throw make_exception(
                    api::ErrorCode::INVALID_DATE_FORMAT,
                    "Start date should be strictly before end date");
        }

        soci::session sql(getWallet()->getDatabase()->getPool());
        auto lowerDate = startDate;
        auto upperDate = DateUtils::incrementDate(startDate, period);

        const auto id = std::stoull(assetId);
        const auto transactions =
            TransactionDatabaseHelper::queryAssetTransferTransactionsInvolving(
                    sql, id, _address.toString());

        std::vector<api::AlgorandAssetAmount> amounts;
        uint64_t balance = 0;
        for (const auto& transaction : transactions) {
            const auto date =
                std::chrono::system_clock::time_point(
                        std::chrono::seconds(
                        transaction.header.timestamp.getValueOr(0)
            ));

            while (date > upperDate && lowerDate < endDate) {
                lowerDate = DateUtils::incrementDate(lowerDate, period);
                upperDate = DateUtils::incrementDate(upperDate, period);
                amounts.emplace_back(std::string(), std::to_string(balance), false, assetId);
            }

            if (date <= upperDate) {
                const auto& header = transaction.header;

                const auto& details =
                    boost::get<model::AssetTransferTxnFields>(transaction.details);

                const auto amount = details.assetAmount.getValueOr(0);
                const auto closeAmount = details.closeAmount.getValueOr(0);

                if (details.assetSender == _address ||
                    header.sender == _address) {
                    balance -= amount;
                }

                if (details.assetReceiver == _address) {
                    balance += amount;
                }

                if (details.assetCloseTo && details.assetCloseTo == _address) {
                    balance += closeAmount;
                }
            }

            if (lowerDate > endDate) {
                break;
            }
        }

        while (lowerDate < endDate) {
            lowerDate = DateUtils::incrementDate(lowerDate, period);
            amounts.emplace_back(std::string(), std::to_string(balance), false, assetId);
        }

        return Future<std::vector<api::AlgorandAssetAmount>>::successful(amounts);
    }

    void Account::getAssetBalanceHistory(
            const std::string& assetId,
            const std::string& start,
            const std::string& end,
            api::TimePeriod period,
            const std::shared_ptr<api::AlgorandAssetAmountListCallback>& callback)
    {
        getAssetBalanceHistory(assetId, start, end, period)
            .callback(getMainExecutionContext(), callback);
    }

    void Account::getAssetsBalances(
            const std::shared_ptr<api::AlgorandAssetAmountListCallback>& callback)
    {
        getAccountInformation()
            .map<std::vector<api::AlgorandAssetAmount>>(
                    getContext(),
                    [](const model::Account& account) {
                        auto balances = std::vector<api::AlgorandAssetAmount>();
                        for (const auto& balance : account.assetsAmounts) {
                            balances.push_back(model::toAPI(balance.second));
                        }
                        return balances;
                    })
            .callback(getMainExecutionContext(), callback);
    }

    void Account::getCreatedAssets(
            const std::shared_ptr<api::AlgorandAssetParamsListCallback>& callback)
    {
        getAccountInformation()
            .map<std::vector<api::AlgorandAssetParams>>(
                    getContext(),
                    [](const model::Account& account) {
                        auto assets = std::vector<api::AlgorandAssetParams>();
                        for (const auto& params : account.createdAssets) {
                            assets.push_back(model::toAPI(params.second));
                        }
                        return assets;
                    })
            .callback(getMainExecutionContext(), callback);
    }

    void Account::getPendingRewards(
            const std::shared_ptr<api::AmountCallback>& callback)
    {
        getAccountInformation()
            .map<uint64_t>(
                    getContext(),
                    [](const model::Account& account) {
                        return account.pendingRewards;
                    })
            .mapPtr<Amount>(
                    getContext(),
                    u64ToAmount(_address.getCurrency()))
            .callback(getMainExecutionContext(), callback);
    }

    void Account::getTotalRewards(
            const std::shared_ptr<api::AmountCallback>& callback)
    {
        getAccountInformation()
            .map<uint64_t>(
                    getContext(),
                    [](const model::Account& account) {
                        return account.rewards;
                    })
            .mapPtr<Amount>(
                    getContext(),
                    u64ToAmount(_address.getCurrency()))
            .callback(getMainExecutionContext(), callback);
    }

    void Account::getFeeEstimate(
            const std::shared_ptr<api::AlgorandTransaction>& transaction,
            const std::shared_ptr<api::AmountCallback>& callback)
    {
        _explorer->getTransactionParams()
            .map<uint64_t>(
                    getContext(),
                    [transaction](const model::TransactionParams& params) {
                        /// This is only true for a single signature.
                        /// If we ever support different type of signatures
                        /// (e.g multi signatures), we will have to find a way
                        /// to add the correct corresponding number of bytes.
                        const auto signatureSize = 71;
                        const auto suggestedFees =
                            params.suggestedFeePerByte * transaction->serialize().size()
                            + signatureSize;
                        return std::max(suggestedFees, params.minFee);
                    })
            .mapPtr<Amount>(
                    getContext(),
                    u64ToAmount(_address.getCurrency()))
            .callback(getMainExecutionContext(), callback);
    }

    std::vector<uint8_t> Account::buildRawSignedTransaction(
            const std::vector<uint8_t>& rawUnsignedTransaction,
            const std::vector<uint8_t>& signature) const
    {
        return model::SignedTransaction::serializeFromTxAndSig(
                rawUnsignedTransaction,
                signature
        );
    }

    void Account::broadcastRawTransaction(
            const std::vector<uint8_t>& transaction,
            const std::shared_ptr<api::StringCallback>& callback)
    {
        _explorer->pushTransaction(transaction)
            .callback(getMainExecutionContext(), callback);
    }

    void Account::broadcastTransaction(
            const std::shared_ptr<api::AlgorandTransaction>& transaction,
            const std::shared_ptr<api::StringCallback>& callback)
    {
        broadcastRawTransaction(transaction->serialize(), callback);
    }

    void Account::createTransaction(
            const std::shared_ptr<api::AlgorandTransactionCallback>& callback)
    {
        _explorer->getTransactionParams()
            .mapPtr<api::AlgorandTransaction>(
                    getMainExecutionContext(),
                    [this](const model::TransactionParams& params) {
                        auto txn = model::Transaction();
                        txn.header.firstValid = params.lastRound;
                        txn.header.lastValid = params.lastRound + 1000;
                        txn.header.genesisHash = B64String(params.genesisHash);
                        txn.header.genesisId = params.genesisID;
                        txn.header.sender = _address;

                        return std::make_shared<AlgorandTransactionImpl>(std::move(txn));
                    })
            .callback(getMainExecutionContext(), callback);
    }

    std::shared_ptr<api::OperationQuery> Account::queryOperations()
    {
        auto query = std::make_shared<OperationQuery>(
            api::QueryFilter::accountEq(getAccountUid()),
            getWallet()->getDatabase(),
            getWallet()->getContext(),
            getWallet()->getMainExecutionContext()
        );
        query->registerAccount(shared_from_this());
        return query;
    }

    std::shared_ptr<api::Keychain> Account::getAccountKeychain()
    {
        throw make_exception(api::ErrorCode::UNSUPPORTED_OPERATION, "Account Keychain is not supported for Algorand");
    }

    bool Account::isSynchronizing()
    {
        std::lock_guard<std::mutex> lock(_synchronizationLock);
        return _currentSyncEventBus != nullptr;
    }

    std::shared_ptr<api::EventBus> Account::synchronize()
    {
        std::lock_guard<std::mutex> lock(_synchronizationLock);
        if (_currentSyncEventBus) {
            return _currentSyncEventBus;
        }

        auto eventPublisher = std::make_shared<EventPublisher>(getContext());
        _currentSyncEventBus = eventPublisher->getEventBus();


        auto startTime = DateUtils::now();
        eventPublisher->postSticky(
            std::make_shared<Event>(api::EventCode::SYNCHRONIZATION_STARTED, api::DynamicObject::newInstance()), 0
        );

        _synchronizer->synchronizeAccount(getSelf())->getFuture()
            .onComplete(getContext(), [this, eventPublisher, startTime](const Try<Unit> &result) {
                api::EventCode code;
                auto payload = std::make_shared<DynamicObject>();
                auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(DateUtils::now() - startTime).count();
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
                std::lock_guard<std::mutex> lock(_synchronizationLock);
                _currentSyncEventBus = nullptr;
                return Future<Unit>::successful(result.getValue());
            });

        return eventPublisher->getEventBus();
    }

    void Account::startBlockchainObservation()
    {
        _observer->registerAccount(getSelf());
    }

    void Account::stopBlockchainObservation()
    {
        _observer->unregisterAccount(getSelf());
    }

    bool Account::isObservingBlockchain()
    {
        return _observer->isRegistered(getSelf());
    }

    std::string Account::getRestoreKey()
    {
        return hex::toString(_address.getPublicKey());
    }

    const Address& Account::getAddress() const
    {
        return _address;
    }

    FuturePtr<Amount> Account::getBalance()
    {
        return getAccountInformation()
            .map<uint64_t>(
                    getContext(),
                    [](const model::Account& account) {
                        return account.amount;
                    })
            .mapPtr<Amount>(
                    getContext(),
                    u64ToAmount(_address.getCurrency()));
    }

    Future<std::vector<std::shared_ptr<api::Amount>>>
    Account::getBalanceHistory(const std::string& start,
                               const std::string& end,
                               api::TimePeriod precision)
    {
        const auto startDate = DateUtils::fromJSON(start);
        const auto endDate = DateUtils::fromJSON(end);
        if (startDate >= endDate) {
            throw make_exception(
                    api::ErrorCode::INVALID_DATE_FORMAT,
                    "Start date should be strictly before end date");
        }

        auto makeAmount =
            [this](uint64_t amount) {
                return u64ToAmount(_address.getCurrency())(amount);
            };

        soci::session sql(getWallet()->getDatabase()->getPool());
        auto lowerDate = startDate;
        auto upperDate = DateUtils::incrementDate(startDate, precision);

        const auto transactions =
            TransactionDatabaseHelper::queryTransactionsInvolving(
                    sql, _address.toString());

        std::vector<std::shared_ptr<api::Amount>> amounts;
        uint64_t balance = 0;
        for (const auto& transaction : transactions) {
            const auto date =
                std::chrono::system_clock::time_point(
                        std::chrono::seconds(
                        transaction.header.timestamp.getValueOr(0)
            ));

            while (date > upperDate && lowerDate < endDate) {
                lowerDate = DateUtils::incrementDate(lowerDate, precision);
                upperDate = DateUtils::incrementDate(upperDate, precision);
                amounts.emplace_back(makeAmount(balance));
            }

            if (date <= upperDate) {
                const auto& header = transaction.header;

                if (header.sender == _address) {
                    balance += header.senderRewards.getValueOr(0);
                    balance -= header.fee;
                }

                if (header.type == model::constants::pay) {
                    const auto& details =
                        boost::get<model::PaymentTxnFields>(transaction.details);

                    if (header.sender == _address) {
                        balance -= details.amount;
                        balance -= details.closeAmount.getValueOr(0);
                    }
                    if (details.receiverAddr == _address) {
                        balance += details.amount;
                        balance += header.receiverRewards.getValueOr(0);
                    }
                    if (details.closeAddr && *details.closeAddr == _address) {
                        balance += details.closeAmount.getValueOr(0);
                        balance += header.closeRewards.getValueOr(0);
                    }
                }
            }

            if (lowerDate > endDate) {
                break;
            }
        }

        while (lowerDate < endDate) {
            lowerDate = DateUtils::incrementDate(lowerDate, precision);
            amounts.emplace_back(makeAmount(balance));
        }

        return Future<std::vector<std::shared_ptr<api::Amount>>>::successful(amounts);
    }

    Future<Account::AddressList> Account::getFreshPublicAddresses()
    {
        return async<AbstractAccount::AddressList>([=]() -> AbstractAccount::AddressList {
            return AbstractAccount::AddressList{ std::make_shared<Address>(_address) };
        });
    }

    Future<api::ErrorCode> Account::eraseDataSince(const std::chrono::system_clock::time_point& date)
    {
        auto accountUid = getAccountUid();

        auto log = logger();
        log->debug(" Start erasing data of account : {}", accountUid);

        std::lock_guard<std::mutex> lock(_synchronizationLock);
        _currentSyncEventBus = nullptr;

        soci::session sql(getWallet()->getDatabase()->getPool());

        // Update account's internal preferences (for synchronization)
        auto savedState = getInternalPreferences()->getSubPreferences("AlgorandAccountSynchronizer")->getObject<SavedState>("state");
        if (savedState.nonEmpty()) {
            // Reset saved state to block mined before given date
            auto previousBlock = BlockDatabaseHelper::getPreviousBlockInDatabase(sql, getWallet()->getCurrency().name, date);
            if (previousBlock.nonEmpty() && savedState.getValue().round > previousBlock.getValue().height) {
                savedState.getValue().round = static_cast<uint64_t>(previousBlock.getValue().height);
            } else if (!previousBlock.nonEmpty()) { // if no previous block, sync should go back from genesis block
                savedState.getValue().round = 0;
            }
             getInternalPreferences()->getSubPreferences("AlgorandAccountSynchronizer")->editor()->putObject<SavedState>("state", savedState.getValue())->commit();
        }
        sql << "DELETE FROM operations WHERE account_uid = :account_uid AND date >= :date ",
            soci::use(accountUid),
            soci::use(date);

        log->debug(" Finish erasing data of account : {}", accountUid);

        return Future<api::ErrorCode>::successful(api::ErrorCode::FUTURE_WAS_SUCCESSFULL);
    }

    std::shared_ptr<Account> Account::getSelf()
    {
        return std::static_pointer_cast<Account>(shared_from_this());
    }

    Future<model::Account> Account::getAccountInformation() const
    {
        return _explorer->getAccount(_address.toString());
    }


} // namespace algorand
} // namespace core
} // namespace ledger

