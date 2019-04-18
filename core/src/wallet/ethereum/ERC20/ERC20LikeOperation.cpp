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


#include "ERC20LikeOperation.h"
#include <wallet/common/Amount.h>
#include <wallet/ethereum/ERC20/erc20Tokens.h>
#include <ethereum/EthereumLikeAddress.h>
#include <api_impl/BigIntImpl.hpp>
namespace ledger {
    namespace core {

            ERC20LikeOperation::ERC20LikeOperation(const std::string &accountAddress,
                                                   const std::string &operationUid,
                                                   const Operation &operation,
                                                   const ERC20Transaction &erc20Tx,
                                                   const api::Currency &currency) {

                auto& tx = operation.ethereumTransaction.getValue();
                _uid = operationUid;
                _ethUidOperation = operation.uid;
                _hash = tx.hash;
                _nonce = std::make_shared<api::BigIntImpl>(BigInt((int64_t)tx.nonce));
                _gasPrice = std::make_shared<api::BigIntImpl>(api::BigIntImpl(tx.gasPrice));
                _gasLimit = std::make_shared<api::BigIntImpl>(api::BigIntImpl(tx.gasLimit));
                _gasUsed = std::make_shared<api::BigIntImpl>(api::BigIntImpl(tx.gasUsed.getValue()));
                _status = tx.status;
                _receiver = erc20Tx.to;
                _sender = erc20Tx.from;
                _value = std::make_shared<api::BigIntImpl>(api::BigIntImpl(erc20Tx.value));
                _data = tx.inputData;
                _time = tx.receivedAt;
                _operationType = erc20Tx.type;
                _blockHeight = tx.block.hasValue() ? tx.block.getValue().height : 0;
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
                return _sender;
            }

            std::string ERC20LikeOperation::getReceiver() {
                return _receiver;
            }

            std::shared_ptr<api::BigInt> ERC20LikeOperation::getValue() {
                return _value;
            }

            std::vector<uint8_t> ERC20LikeOperation::getData() {
                return _data;
            }

            std::chrono::system_clock::time_point ERC20LikeOperation::getTime() {
                return _time;
            }

            api::OperationType ERC20LikeOperation::getOperationType() {
                return _operationType;
            }

            std::string ERC20LikeOperation::getOperationUid() {
                return _uid;
            }

            std::string ERC20LikeOperation::getETHOperationUid() {
                return _ethUidOperation;
            }

            int32_t ERC20LikeOperation::getStatus() {
                return _status;
            }

            std::experimental::optional<int64_t> ERC20LikeOperation::getBlockHeight() {
                return _blockHeight == 0 ? Option<int64_t>() : Option<int64_t>(_blockHeight);
            }
    }
}