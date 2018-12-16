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

#include <wallet/common/database/OperationDatabaseHelper.h>
#include <wallet/ethereum/explorers/EthereumLikeBlockchainExplorer.h>
#include <wallet/ethereum/keychains/EthereumLikeKeychain.hpp>
#include <wallet/ethereum/transaction_builders/EthereumLikeTransactionBuilder.h>
#include <wallet/ethereum/database/EthereumLikeTransactionDatabaseHelper.h>
#include <wallet/ethereum/api_impl/EthereumLikeTransactionApi.h>

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
            _keychain->getAllObservableAddresses(0, 40);
        }


        FuturePtr<EthereumLikeBlockchainExplorer::Transaction> EthereumLikeAccount::getTransaction(const std::string& hash) {
                auto self = std::dynamic_pointer_cast<EthereumLikeAccount>(shared_from_this());
                return async<std::shared_ptr<EthereumLikeBlockchainExplorer::Transaction>>([=] () -> std::shared_ptr<EthereumLikeBlockchainExplorer::Transaction> {
                    auto tx = std::make_shared<EthereumLikeBlockchainExplorer::Transaction>();
                    soci::session sql(self->getWallet()->getDatabase()->getPool());
                    if (!EthereumLikeTransactionDatabaseHelper::getTransactionByHash(sql, hash, *tx)) {
                            throw make_exception(api::ErrorCode::TRANSACTION_NOT_FOUND, "Transaction {} not found", hash);
                    }
                    return tx;
                });
        }

        int EthereumLikeAccount::putTransaction(soci::session &sql,
                                               const EthereumLikeBlockchainExplorer::Transaction &transaction) {
                auto wallet = getWallet();
                if (wallet == nullptr) {
                        throw Exception(api::ErrorCode::RUNTIME_ERROR, "Wallet reference is dead.");
                }
                if (transaction.block.nonEmpty())
                        putBlock(sql, transaction.block.getValue());
                auto nodeIndex = std::const_pointer_cast<const EthereumLikeKeychain>(_keychain)->getFullDerivationScheme().getPositionForLevel(DerivationSchemeLevel::NODE);

                uint64_t sentAmount = 0L;
                uint64_t receivedAmount = 0L;
                int result = 0x00;


                //std::stringstream snds;
                //strings::join(senders, snds, ",");

                Operation operation;
                //inflateOperation(operation, wallet, transaction);
                //operation.senders = std::move(senders);
                //operation.recipients = std::move(recipients);
                operation.trust = std::make_shared<TrustIndicator>();
                operation.date = transaction.receivedAt;

                OperationDatabaseHelper::putOperation(sql, operation);
                return result;
        }

        bool EthereumLikeAccount::putBlock(soci::session& sql,
                                           const EthereumLikeBlockchainExplorer::Block& block) {
                throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "EthereumLikeAccount::putBlock is not implemented yet");
        }
        FuturePtr<Amount> EthereumLikeAccount::getBalance() {

        }

        Future<AbstractAccount::AddressList> EthereumLikeAccount::getFreshPublicAddresses() {
                throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "EthereumLikeAccount::getFreshPublicAddresses is not implemented yet");
        }

        Future<std::vector<std::shared_ptr<api::Amount>>>
        EthereumLikeAccount::getBalanceHistory(const std::string & start,
                                               const std::string & end,
                                               api::TimePeriod precision) {

        }

        Future<api::ErrorCode> EthereumLikeAccount::eraseDataSince(const std::chrono::system_clock::time_point & date) {

        }

        bool EthereumLikeAccount::isSynchronizing() {

        }

        std::shared_ptr<api::EventBus> EthereumLikeAccount::synchronize() {
                throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "EthereumLikeAccount::synchronize is not implemented yet");
        }

        void EthereumLikeAccount::startBlockchainObservation() {

        }

        void EthereumLikeAccount::stopBlockchainObservation() {

        }

        bool EthereumLikeAccount::isObservingBlockchain() {

        }

        std::string EthereumLikeAccount::getRestoreKey() {

        }

        void EthereumLikeAccount::broadcastRawTransaction(const std::vector<uint8_t> & transaction,
                                                          const std::shared_ptr<api::StringCallback> & callback) {

        }

        void EthereumLikeAccount::broadcastTransaction(const std::shared_ptr<api::EthereumLikeTransaction> & transaction,
                                                       const std::shared_ptr<api::StringCallback> & callback) {

        }

        std::shared_ptr<api::EthereumLikeAccount> EthereumLikeAccount::asEthereumLikeAccount() {
                return std::dynamic_pointer_cast<EthereumLikeAccount>(shared_from_this());
        }

        std::shared_ptr<api::EthereumLikeTransactionBuilder> EthereumLikeAccount::buildTransaction() {
                //throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "EthereumLikeAccount::buildTransaction is not implemented yet");
                auto self = std::dynamic_pointer_cast<EthereumLikeAccount>(shared_from_this());

                auto getTransaction = [self] (const std::string& hash) -> FuturePtr<EthereumLikeBlockchainExplorer::Transaction> {
                    return self->getTransaction(hash);
                };

                //std::function<Future<std::shared_ptr<api::EthereumLikeTransaction>> (const EthereumLikeTransactionBuildRequest&)>;

                auto buildFunction = [self] (const EthereumLikeTransactionBuildRequest& request, const std::shared_ptr<EthereumLikeBlockchainExplorer> &explorer) -> Future<std::shared_ptr<api::EthereumLikeTransaction>> {
                    std::shared_ptr<api::EthereumLikeTransaction> mock;
                    //EthereumLikeTransactionApi(const std::shared_ptr<OperationApi>& operation);
                    auto tx = std::make_shared<EthereumLikeTransactionApi>(self->getWallet()->getCurrency());
                    tx->setValue(request.value);
                    tx->setData(request.inputData);
                    tx->setGasLimit(request.gasLimit);
                    tx->setGasPrice(request.gasPrice);
                    tx->setReceiver(request.toAddress);
                    //return Future<std::shared_ptr<api::EthereumLikeTransaction>>::successful(mock);
                    return explorer->getNonce(request.toAddress).map<std::shared_ptr<api::EthereumLikeTransaction>>(self->getContext(), [self, tx] (const std::shared_ptr<BigInt> &nonce) -> std::shared_ptr<api::EthereumLikeTransaction> {
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