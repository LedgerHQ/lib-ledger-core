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

#include <core/math/BigInt.hpp>

#include <vector>

namespace ledger {
namespace core {
namespace algorand {

    Account::Account(const std::shared_ptr<AbstractWallet>& wallet,
                     int32_t index,
                     const api::Currency& currency,
                     const std::vector<uint8_t>& pubKey)
        : AbstractAccount(wallet->getServices(), wallet, index)
        , _address(currency, pubKey)
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

    void Account::getAssets(const std::shared_ptr<api::AlgorandAssetParamsListCallback>& callback)
    {
        // TODO
    }

    void Account::getCreatedAssets(const std::shared_ptr<api::AlgorandAssetParamsListCallback>& callback)
    {
        // TODO
    }

    void Account::getAssetBalance(const std::string& assetId,
                                  const std::shared_ptr<api::AlgorandAssetAmountCallback>& callback)
    {
        // TODO
    }

    void Account::getPendingReward(const std::shared_ptr<api::AmountCallback>& callback)
    {
        // TODO
    }

    void Account::getFeeEstimate(const std::shared_ptr<api::AlgorandTransaction>& transaction,
                                 const std::shared_ptr<api::AmountCallback>& callback)
    {
        // TODO
    }

    void Account::broadcastTransaction(const std::shared_ptr<api::AlgorandTransaction>& transaction)
    {
        // TODO
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

    bool Account::isSynchronizing()
    {
        // TODO
        return false;
    }

    std::shared_ptr<api::EventBus> Account::synchronize()
    {
        // TODO
        return nullptr;
    }

    void Account::startBlockchainObservation()
    {
        // TODO
    }

    void Account::stopBlockchainObservation()
    {
        // TODO
    }

    bool Account::isObservingBlockchain()
    {
        // TODO
        return false;
    }

    std::string Account::getRestoreKey()
    {
        // TODO
        return "";
    }


    FuturePtr<Amount> Account::getBalance()
    {
        return FuturePtr<Amount>::failure(Exception(api::ErrorCode::ACCOUNT_ALREADY_EXISTS, "whatever"));
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
        return Future<AddressList>::failure(Exception(api::ErrorCode::ACCOUNT_ALREADY_EXISTS, "whatever"));
    }

    Future<api::ErrorCode> Account::eraseDataSince(const std::chrono::system_clock::time_point& date)
    {
        // TODO
        return Future<api::ErrorCode>::failure(Exception(api::ErrorCode::ACCOUNT_ALREADY_EXISTS, "whatever"));
    }

    namespace {

        void setAmountAndRecipients(Operation& op,
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

        void setBlock(Operation& op,
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

    void Account::inflateOperation(Operation& op,
                                   const std::shared_ptr<const AbstractWallet>& wallet,
                                   const model::Transaction& tx)
    {
        op.setTransaction(tx);
        op.accountUid = getAccountUid();
        op.walletUid = wallet->getWalletUid();
        op.date = {}; // TODO (when date is available in the tx)
        op.senders = { tx.header.sender.toString() };
        setAmountAndRecipients(op, tx);
        op.fees = BigInt(static_cast<unsigned long long>(tx.header.fee));
        setBlock(op, wallet, tx);
        op.currencyName = wallet->getCurrency().name;
        op.type = api::OperationType::NONE;
        op.trust = std::make_shared<TrustIndicator>();
    }

} // namespace algorand
} // namespace core
} // namespace ledger

