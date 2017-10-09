/*
 *
 * BitcoinLikeTransactionApi
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/07/2017.
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
#include "BitcoinLikeTransactionApi.h"
#include <wallet/common/Amount.h>
#include <wallet/common/AbstractAccount.hpp>

namespace ledger {
    namespace core {

        BitcoinLikeTransactionApi::BitcoinLikeTransactionApi(const std::shared_ptr<OperationApi> &operation) {

            auto& tx = operation->getBackend().bitcoinTransaction.getValue();
            _time = tx.receivedAt;
            _lockTime = tx.lockTime;
            if (tx.fees.nonEmpty())
                _fees = std::make_shared<Amount>(operation->getAccount()->getWallet()->getCurrency(), 0, tx.fees.getValue());
            else
                _fees = nullptr;

            auto inputCount = tx.inputs.size();
            for (auto index = 0; index < inputCount; index++) {
                _inputs.push_back(std::make_shared<BitcoinLikeInputApi>(operation, index));
            }

            auto outputCount = tx.outputs.size();
            for (auto index = 0; index < outputCount; index++) {
                _outputs.push_back(std::make_shared<BitcoinLikeOutputApi>(operation, index));
            }

            if (tx.block.nonEmpty()) {
                _block = std::make_shared<BitcoinLikeBlockApi>(tx.block.getValue());
            } else {
                _block = nullptr;
            }
            _hash = tx.hash;
        }

        std::vector<std::shared_ptr<api::BitcoinLikeInput>> BitcoinLikeTransactionApi::getInputs() {
            return _inputs;
        }

        std::vector<std::shared_ptr<api::BitcoinLikeOutput>> BitcoinLikeTransactionApi::getOutputs() {
            return _outputs;
        }

        std::shared_ptr<api::BitcoinLikeBlock> BitcoinLikeTransactionApi::getBlock() {
            return _block;
        }

        int64_t BitcoinLikeTransactionApi::getLockTime() {
            return _lockTime;
        }

        std::shared_ptr<api::Amount> BitcoinLikeTransactionApi::getFees() {
            return _fees;
        }

        std::chrono::system_clock::time_point BitcoinLikeTransactionApi::getTime() {
            return _time;
        }

        std::string BitcoinLikeTransactionApi::getHash() {
            return _hash;
        }
    }
}