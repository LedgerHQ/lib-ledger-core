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

#include <api/AlgorandAssetAmount.hpp>
#include <api/AlgorandAssetParams.hpp>

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

    bool Account::putBlock(soci::session& sql,
                           const api::Block& block)
    {
        // TODO
        return true;
    }

    int Account::putTransaction(soci::session& sql,
                                const model::Transaction& transaction)
    {
        const auto wallet = getWallet();
        if (wallet == nullptr) {
            throw Exception(api::ErrorCode::RUNTIME_ERROR, "Wallet reference is dead.");
        }

        const auto txuid =
            TransactionDatabaseHelper::putTransaction(sql, getAccountUid(), transaction);
        const auto operation = [&]() {
            auto operation = Operation(shared_from_this(), transaction);
            inflateOperation(operation, getWallet(), transaction);
            operation.refreshUid();
            return operation;
        }();

        OperationDatabaseHelper::putAlgorandOperation(sql, txuid, operation);

        return static_cast<int>(operation.getConstBackend().type);
    }

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

    } // namespace

    void Account::getAsset(
            const std::string& assetId,
            const std::shared_ptr<api::AlgorandAssetParamsCallback>& callback)
    {
        _explorer->getAssetById(std::stoull(assetId))
            .map<api::AlgorandAssetParams>(
                    getContext(),
                    [&assetId](const model::AssetParams& params) {
                        auto apiParams = model::toAPI(params);
                        apiParams.assetId = assetId;
                        return apiParams;
                    })
            .callback(getMainExecutionContext(), callback);
    }

    void Account::hasAsset(const std::string & assetId, const std::shared_ptr<api::BoolCallback> & callback)
    {
       getAccountInformation()
            .map<bool>(
                    getContext(),
                    [&assetId](const model::Account& account) {
                        const auto id = std::stoull(assetId);
                        return static_cast<bool>(account.assetsAmounts.find(id)->second.amount);
                    })
            .callback(getMainExecutionContext(), callback);
    }

    void Account::getAssetBalance(
            const std::string& assetId,
            const std::shared_ptr<api::AlgorandAssetAmountCallback>& callback)
    {
        getAccountInformation()
            .map<api::AlgorandAssetAmount>(
                    getContext(),
                    [&assetId](const model::Account& account) {
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
                amounts.emplace_back(std::string(), std::to_string(balance), false);
            }

            if (date <= upperDate) {
                const auto& header = transaction.header;

                const auto& details =
                    boost::get<model::AssetTransferTxnFields>(transaction.details);

                const auto amount = details.assetAmount.getValueOr(0);
                /// TODO: closeAmount
                /// The algorand API currently does not provide the close amount,
                /// this is why this amount is set to 0 for now.
                if (details.assetSender == _address ||
                    header.sender == _address) {
                    balance -= amount;
                }

                if (details.assetReceiver == _address) {
                    balance += amount;
                }

                // TODO
                if (details.assetCloseTo && details.assetCloseTo == _address) {
                    balance += /* closeAmount */0;
                }
            }

            if (lowerDate > endDate) {
                break;
            }
        }

        while (lowerDate < endDate) {
            lowerDate = DateUtils::incrementDate(lowerDate, period);
            amounts.emplace_back(std::string(), std::to_string(balance), false);
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
            const std::shared_ptr<api::AlgorandAssetAmountMapCallback>& callback)
    {
        getAccountInformation()
            .map<std::unordered_map<std::string, api::AlgorandAssetAmount>>(
                    getContext(),
                    [](const model::Account& account) {
                        auto balances = std::unordered_map<std::string, api::AlgorandAssetAmount>();
                        for (const auto& balance : account.assetsAmounts) {
                            const auto id = std::to_string(balance.first);
                            const auto amount = balance.second;
                            balances[id] = model::toAPI(amount);
                        }
                        return balances;
                    })
            .callback(getMainExecutionContext(), callback);
    }

    void Account::getCreatedAssets(
            const std::shared_ptr<api::AlgorandAssetParamsMapCallback>& callback)
    {
        getAccountInformation()
            .map<std::unordered_map<std::string, api::AlgorandAssetParams>>(
                    getContext(),
                    [](const model::Account& account) {
                        auto assets = std::unordered_map<std::string, api::AlgorandAssetParams>();
                        for (const auto& params : account.createdAssets) {
                            const auto id = std::to_string(params.first);
                            const auto assetParams = model::toAPI(params.second);
                            assets[id] = assetParams;
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
                    [&transaction](const model::TransactionParams& params) {
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
        // FIXME : check nullptr error  
        return nullptr;
    }

    bool Account::isSynchronizing()
    {
        std::lock_guard<std::mutex> lock(_synchronizationLock);
        return _currentSyncEventBus != nullptr;
    }

    std::shared_ptr<api::EventBus> Account::synchronize()
    {
        // TODO
        return nullptr;
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
                    balance -= transaction.header.fee;
                }

                if (header.type != model::constants::pay) {
                    continue;
                }

                const auto& details =
                    boost::get<model::PaymentTxnFields>(transaction.details);

                if (header.sender == _address) {
                    balance -= details.amount;
                    balance -= details.closeAmount.getValueOr(0);
                    balance += header.fromRewards.getValueOr(0);
                }
                if (details.receiverAddr == _address) {
                    balance += details.amount;
                    balance += details.receiverRewards.getValueOr(0);
                }
                if (details.closeAddr && *details.closeAddr == _address) {
                    balance += details.closeAmount.getValueOr(0);
                    balance += details.closeRewards.getValueOr(0);
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
        // TODO
        return async<Account::AddressList>([=]() {
            return Account::AddressList{ _address.shared_from_this() };
        });
    }

    Future<api::ErrorCode> Account::eraseDataSince(const std::chrono::system_clock::time_point& date)
    {
        // TODO
        return Future<api::ErrorCode>::failure(Exception(api::ErrorCode::ACCOUNT_ALREADY_EXISTS, "whatever"));
    }

    namespace {

        void setAmountAndRecipients(
                Operation& op,
                const model::Transaction& tx)
        {
            if (tx.header.type == model::constants::pay) {
                const auto& details = boost::get<model::PaymentTxnFields>(tx.details);
                op.getBackend().amount = BigInt(static_cast<unsigned long long>(details.amount));
                op.getBackend().recipients = {};
                op.getBackend().recipients.push_back(details.receiverAddr.toString());
                if (details.closeAddr) {
                    op.getBackend().recipients.push_back(details.closeAddr->toString());
                    // TODO : adjust amount in this case?
                }
            } else if (tx.header.type == model::constants::axfer) {
                const auto& details = boost::get<model::AssetTransferTxnFields>(tx.details);
                op.getBackend().amount = BigInt(static_cast<unsigned long long>(details.assetAmount));
                op.getBackend().recipients = {};
                op.getBackend().recipients.push_back(details.assetReceiver.toString());
                if (details.assetCloseTo) {
                    op.getBackend().recipients.push_back(details.assetCloseTo->toString());
                    // TODO : adjust amount in this case?
                }
            }
        }

        void setBlock(
                Operation& op,
                const std::shared_ptr<const AbstractWallet>& wallet,
                const model::Transaction& tx)
        {
            const auto block = [&tx, &wallet]() {
                api::Block block;
                block.currencyName = wallet->getCurrency().name;
                // TODO: add tx.time (when available)
                if (tx.header.round) {
                    if (*tx.header.round > std::numeric_limits<int64_t>::max()) {
                        throw make_exception(api::ErrorCode::OUT_OF_RANGE, "Block height exceeds maximum value");
                    }
                    block.height = static_cast<int64_t>(*tx.header.round);
                }
                return block;
            }();
            op.getBackend().block = block;
        }

    } // namespace

    void Account::inflateOperation(
            Operation& op,
            const std::shared_ptr<const AbstractWallet>& wallet,
            const model::Transaction& tx)
    {
        op.setTransaction(tx);
        op.getBackend().accountUid = getAccountUid();
        op.getBackend().walletUid = wallet->getWalletUid();
        op.getBackend().date =
            std::chrono::system_clock::time_point(
                    std::chrono::seconds(
                        tx.header.timestamp.getValueOr(0)
            ));
        op.getBackend().senders = { tx.header.sender.toString() };
        setAmountAndRecipients(op, tx);
        op.getBackend().fees = BigInt(static_cast<unsigned long long>(tx.header.fee));
        setBlock(op, wallet, tx);
        op.getBackend().currencyName = wallet->getCurrency().name;
        op.getBackend().type = api::OperationType::NONE;
        op.getBackend().trust = std::make_shared<TrustIndicator>();
    }

    std::shared_ptr<Account> Account::getSelf()
    {
        return std::dynamic_pointer_cast<Account>(shared_from_this());
    }

    Future<model::Account> Account::getAccountInformation() const
    {
        return _explorer->getAccount(_address.toString());
    }


} // namespace algorand
} // namespace core
} // namespace ledger

