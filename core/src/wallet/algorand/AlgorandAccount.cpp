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
#include <algorand/model/AlgorandModelMapper.hpp>

#include <algorand/api/AlgorandAssetAmount.hpp>
#include <algorand/api/AlgorandAssetParams.hpp>

#include <api/BigInt.hpp>
#include <core/utils/Hex.hpp>

#include <chrono>
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
        // TODO
        return 0;
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

    void Account::getAssetBalanceHistory(
            const std::string& assetId,
            const std::string& start,
            const std::string& end,
            api::TimePeriod period,
            const std::shared_ptr<api::AlgorandAssetAmountListCallback>& callback)
    {
        // TODO
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
        // TODO
        // auto query = std::make_shared<OperationQuery>(
        //     api::QueryFilter::accountEq(getAccountUid());
        //     getWallet()->getDatabase(),
        //     getWallet()->getContext(),
        //     getWallet()->getMainExecutionContext()
        // );
        // query->registerAccount(shared_from_this());
        // return query;
        return nullptr;
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
        // TODO
        return Future<std::vector<std::shared_ptr<api::Amount>>>::failure(Exception(api::ErrorCode::ACCOUNT_ALREADY_EXISTS, "whatever"));
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
                op.amount = BigInt(static_cast<unsigned long long>(details.amount));
                op.recipients = {};
                op.recipients.push_back(details.receiverAddr.toString());
                if (details.closeAddr) {
                    op.recipients.push_back(details.closeAddr->toString());
                    // TODO : adjust amount in this case?
                }
            } else if (tx.header.type == model::constants::axfer) {
                const auto& details = boost::get<model::AssetTransferTxnFields>(tx.details);
                op.amount = BigInt(static_cast<unsigned long long>(details.assetAmount));
                op.recipients = {};
                op.recipients.push_back(details.assetReceiver.toString());
                if (details.assetCloseTo) {
                    op.recipients.push_back(details.assetCloseTo->toString());
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
            op.block = block;
        }

    } // namespace

    void Account::inflateOperation(
            Operation& op,
            const std::shared_ptr<const AbstractWallet>& wallet,
            const model::Transaction& tx)
    {
        op.setTransaction(tx);
        op.accountUid = getAccountUid();
        op.walletUid = wallet->getWalletUid();
        op.date =
            std::chrono::system_clock::time_point(
                    std::chrono::seconds(
                        tx.header.timestamp.getValueOr(0)
            ));
        op.senders = { tx.header.sender.toString() };
        setAmountAndRecipients(op, tx);
        op.fees = BigInt(static_cast<unsigned long long>(tx.header.fee));
        setBlock(op, wallet, tx);
        op.currencyName = wallet->getCurrency().name;
        op.type = api::OperationType::NONE;
        op.trust = std::make_shared<TrustIndicator>();
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

