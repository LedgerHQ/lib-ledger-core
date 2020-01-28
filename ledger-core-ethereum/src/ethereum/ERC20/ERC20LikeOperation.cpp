/*
 *
 * ERC20LikeTransaction
 *
 * Created by El Khalil Bellakrid on 26/08/2018.
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

#include <core/math/BigInt.hpp>
#include <core/wallet/Amount.hpp>

#include <ethereum/ERC20/ERC20LikeOperation.hpp>
#include <ethereum/ERC20/ERC20Tokens.hpp>
#include <ethereum/EthereumLikeAddress.hpp>
#include <ethereum/operations/EthereumLikeOperation.hpp>
#include <ethereum/EthereumLikeBlock.hpp>

namespace ledger {
    namespace core {
        ERC20LikeOperation::ERC20LikeOperation(
            std::shared_ptr<AbstractAccount> const& account
        ): Operation(account) {
        }

        ERC20LikeOperation::ERC20LikeOperation(
            const std::string &accountAddress,
            const std::string &operationUid,
            const EthereumLikeOperation &operation,
            const ERC20Transaction &erc20Tx,
            const api::Currency &currency
        ): Operation(operation.getAccount()) {
            auto const tx = operation.getTransaction();
            uid = operationUid;
            senders = { erc20Tx.from };
            recipients = { erc20Tx.to };
            amount = erc20Tx.value;
            type = erc20Tx.type;
            date = operation.getExplorerTransaction().receivedAt;
            block = api::Block();
            block->height = tx->getBlock()->getHeight();

            _ethUidOperation = operation.uid;
            _hash = tx->getHash();
            _nonce = std::make_shared<BigInt>(BigInt(static_cast<int64_t>(tx->getNonce())));
            _gasPrice = tx->getGasPrice()->toBigInt();
            _gasLimit = tx->getGasLimit()->toBigInt();
            _gasUsed = tx->getGasUsed()->toBigInt();
            _data = tx->getData().value_or(std::vector<uint8_t>{});
            _status = tx->getStatus();
        }

        std::string ERC20LikeOperation::getHash() {
            return _hash;
        }

        std::shared_ptr<api::BigInt> ERC20LikeOperation::getNonce() {
            return _nonce;
        }

        std::shared_ptr<api::BigInt> ERC20LikeOperation::getGasPrice() {
            return _gasPrice;
        }

        std::shared_ptr<api::BigInt> ERC20LikeOperation::getGasLimit() {
            return _gasLimit;
        }

        std::shared_ptr<api::BigInt> ERC20LikeOperation::getUsedGas() {
            return _gasUsed;
        }

        std::string ERC20LikeOperation::getSender() {
            return getSenders()[0];
        }

        std::string ERC20LikeOperation::getReceiver() {
            return getRecipients()[0];
        }

        std::shared_ptr<api::BigInt> ERC20LikeOperation::getValue() {
            return getAmount()->toBigInt();
        }

        std::vector<uint8_t> ERC20LikeOperation::getData() {
            return _data;
        }

        std::chrono::system_clock::time_point ERC20LikeOperation::getTime() {
            return getDate();
        }

        std::string ERC20LikeOperation::getOperationUid() {
            return getUid();
        }

        std::string ERC20LikeOperation::getETHOperationUid() {
            return _ethUidOperation;
        }

        int32_t ERC20LikeOperation::getStatus() {
            return _status;
        }

        void ERC20LikeOperation::refreshUid(std::string const&) {
            //uid =
        }
    }
}
