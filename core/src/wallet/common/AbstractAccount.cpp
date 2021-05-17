/*
 *
 * AbstractAccount
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
#include <wallet/common/database/AccountDatabaseHelper.h>
#include "AbstractAccount.hpp"
#include <wallet/common/OperationQuery.h>
#include <api/AmountCallback.hpp>
#include <utils/Exception.hpp>
#include <api/ErrorCode.hpp>
#include <events/Event.hpp>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <wallet/pool/WalletPool.hpp>
#include <wallet/stellar/StellarLikeAccount.hpp>
#include <wallet/common/synchronizers/AbstractBlockchainExplorerAccountSynchronizer.h>

namespace ledger {
    namespace core {

        AbstractAccount::AbstractAccount(const std::shared_ptr<AbstractWallet> &wallet, int32_t index)
                : DedicatedContext(wallet->getMainExecutionContext())
        {
            _uid = AccountDatabaseHelper::createAccountUid(wallet->getWalletUid(), index);
            _logger = wallet->logger();
            _index = index;
            _internalPreferences = wallet->getAccountInternalPreferences(index);
            _externalPreferences = wallet->getAccountExternalPreferences(index);
            _loggerApi = wallet->getLogger();
            _wallet = wallet;
            _mainExecutionContext = wallet->getMainExecutionContext();
            _logger = wallet->logger();
            _type = wallet->getWalletType();
            _publisher = std::make_shared<EventPublisher>(getContext());
        }

        int32_t AbstractAccount::getIndex() {
            return _index;
        }

        std::shared_ptr<api::Preferences> AbstractAccount::getPreferences() {
            return _externalPreferences;
        }

        std::shared_ptr<api::Logger> AbstractAccount::getLogger() {
            return _loggerApi;
        }

        bool AbstractAccount::isInstanceOfBitcoinLikeAccount() {
            return _type == api::WalletType::BITCOIN;
        }

        bool AbstractAccount::isInstanceOfCosmosLikeAccount() {
            return _type == api::WalletType::COSMOS;
        }

        bool AbstractAccount::isInstanceOfEthereumLikeAccount() {
            return _type == api::WalletType::ETHEREUM;
        }

        bool AbstractAccount::isInstanceOfRippleLikeAccount() {
            return _type == api::WalletType::RIPPLE;
        }

        api::WalletType AbstractAccount::getWalletType() {
            return _type;
        }

        std::shared_ptr<api::Preferences> AbstractAccount::getOperationPreferences(const std::string &uid) {
            return getOperationExternalPreferences(uid);
        }

        std::shared_ptr<api::BitcoinLikeAccount> AbstractAccount::asBitcoinLikeAccount() {
            return std::dynamic_pointer_cast<api::BitcoinLikeAccount>(shared_from_this());
        }

        std::shared_ptr<api::CosmosLikeAccount> AbstractAccount::asCosmosLikeAccount() {
            return std::dynamic_pointer_cast<api::CosmosLikeAccount>(shared_from_this());
        }

        std::shared_ptr<api::EthereumLikeAccount> AbstractAccount::asEthereumLikeAccount() {
            return std::dynamic_pointer_cast<api::EthereumLikeAccount>(shared_from_this());
        }

        std::shared_ptr<api::RippleLikeAccount> AbstractAccount::asRippleLikeAccount() {
            return std::dynamic_pointer_cast<api::RippleLikeAccount>(shared_from_this());
        }

        std::shared_ptr<api::TezosLikeAccount> AbstractAccount::asTezosLikeAccount() {
            return std::dynamic_pointer_cast<api::TezosLikeAccount>(shared_from_this());
        }

        std::shared_ptr<api::AlgorandAccount> AbstractAccount::asAlgorandAccount() {
            return std::dynamic_pointer_cast<api::AlgorandAccount>(shared_from_this());
        }

        std::shared_ptr<spdlog::logger> AbstractAccount::logger() const {
            return _logger;
        }

        std::shared_ptr<Preferences> AbstractAccount::getOperationExternalPreferences(const std::string &uid) {
            return _externalPreferences->getSubPreferences(fmt::format("operation_{}", uid));
        }

        std::shared_ptr<Preferences> AbstractAccount::getOperationInternalPreferences(const std::string &uid) {
            return _internalPreferences->getSubPreferences(fmt::format("operation_{}", uid));
        }

        const std::string &AbstractAccount::getAccountUid() const {
            return _uid;
        }

        std::shared_ptr<AbstractWallet> AbstractAccount::getWallet() const {
            auto wallet = _wallet.lock();
            if (!wallet) {
                throw make_exception(api::ErrorCode::NULL_POINTER, "Wallet was already released");
            }
            return wallet;
        }

		std::shared_ptr<AbstractWallet> AbstractAccount::getWallet() {
            auto wallet = _wallet.lock();
            if (!wallet) {
                throw make_exception(api::ErrorCode::NULL_POINTER, "Wallet was already released");
            }
            return wallet;
		}

        const std::shared_ptr<api::ExecutionContext> AbstractAccount::getMainExecutionContext() const {
            return _mainExecutionContext;
        }

        std::shared_ptr<api::OperationQuery> AbstractAccount::queryOperations() {
            return std::make_shared<OperationQuery>(
                    api::QueryFilter::accountEq(getAccountUid()),
                    getWallet()->getDatabase(),
                    getWallet()->getPool()->getThreadPoolExecutionContext(),
                    getMainExecutionContext()
            );
        }

        std::shared_ptr<Preferences> AbstractAccount::getInternalPreferences() const {
            return _internalPreferences;
        }

        std::shared_ptr<Preferences> AbstractAccount::getExternalPreferences() const {
            return _externalPreferences;
        }

        void AbstractAccount::getFreshPublicAddresses(const std::shared_ptr<api::AddressListCallback> &callback) {
            getFreshPublicAddresses().callback(getMainExecutionContext(), callback);
        }

        void AbstractAccount::getBalance(const std::shared_ptr<api::AmountCallback> &callback) {
            getBalance().callback(getMainExecutionContext(), callback);
        }

        void AbstractAccount::getBalanceHistory(const std::string & start,
                               const std::string & end,
                               api::TimePeriod precision,
                               const std::shared_ptr<api::AmountListCallback> & callback) {
            getBalanceHistory(start, end, precision).callback(getMainExecutionContext(), callback);
        }

        std::shared_ptr<api::EventBus> AbstractAccount::getEventBus() {
            return _publisher->getEventBus();
        }

        void AbstractAccount::emitNewOperationEvent(const Operation &operation) {
            auto payload = DynamicObject::newInstance();
            payload->putString(api::Account::EV_NEW_OP_UID, operation.uid);
            payload->putString(api::Account::EV_NEW_OP_WALLET_NAME, getWallet()->getName());
            payload->putLong(api::Account::EV_NEW_OP_ACCOUNT_INDEX, getIndex());
            auto event = Event::newInstance(api::EventCode::NEW_OPERATION, payload);
            pushEvent(event);
        }

        void AbstractAccount::emitNewBlockEvent(const Block &block) {
            auto payload = DynamicObject::newInstance();
            payload->putLong(api::Account::EV_NEW_BLOCK_HEIGHT, block.height);
            payload->putString(api::Account::EV_NEW_BLOCK_HASH, block.hash);
            payload->putString(api::Account::EV_NEW_BLOCK_CURRENCY_NAME, block.currencyName);
            auto event = Event::newInstance(api::EventCode::NEW_BLOCK, payload);
            pushEvent(event);
        }

        void AbstractAccount::emitEventsNow() {
            auto self = shared_from_this();
            run([self] () {
                std::list<std::shared_ptr<api::Event>> events;
                {
                    std::lock_guard<std::mutex> lock(self->_eventsLock);
                    std::swap(events, self->_events);
                }
                for (auto& event : events) {
                    self->_publisher->post(event);
                }
            });
        }

        void AbstractAccount::pushEvent(const std::shared_ptr<api::Event> &event) {
            auto self = shared_from_this();
            run([event, self] () {
                std::lock_guard<std::mutex> lock(self->_eventsLock);
                self->_events.push_back(std::move(event));
            });
        }

        Future<api::Block> AbstractAccount::getLastBlock() {
            return getWallet()->getLastBlock();
        }

        void AbstractAccount::getLastBlock(const std::shared_ptr<api::BlockCallback> &callback) {
            getLastBlock().callback(getMainExecutionContext(), callback);
        }

        void AbstractAccount::eraseDataSince(const std::chrono::system_clock::time_point & date, const std::shared_ptr<api::ErrorCodeCallback> & callback) {
            eraseDataSince(date).callback(getMainExecutionContext(), callback);
        }

        void AbstractAccount::dropTransaction(const std::string & txId, const std::shared_ptr<api::ErrorCodeCallback> & callback) {
            dropTransaction(txId).callback(getMainExecutionContext(), callback);
        }

        std::shared_ptr<api::StellarLikeAccount> AbstractAccount::asStellarLikeAccount() {
            return std::dynamic_pointer_cast<api::StellarLikeAccount>(shared_from_this());
        }

        bool AbstractAccount::isInstanceOfStellarLikeAccount() const {
            return _type == api::WalletType::STELLAR;
        }

		void AbstractAccount::eraseSynchronizerDataSince(soci::session &sql, const std::chrono::system_clock::time_point &date) {
            //Update account's internal preferences (for synchronization)
            auto savedState = getInternalPreferences()->getSubPreferences("AbstractBlockchainExplorerAccountSynchronizer")->getObject<BlockchainExplorerAccountSynchronizationSavedState>("state");
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
                getInternalPreferences()->getSubPreferences("AbstractBlockchainExplorerAccountSynchronizer")->editor()->putObject<BlockchainExplorerAccountSynchronizationSavedState>("state", savedState.getValue())->commit();
            }
        }

    }
}
