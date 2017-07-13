/*
 *
 * BitcoinLikeOutputApi
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
#include "BitcoinLikeOutputApi.h"
#include <utils/hex.h>
#include <wallet/common/Amount.h>
#include <wallet/common/AbstractAccount.hpp>

namespace ledger {
    namespace core {

        BitcoinLikeOutputApi::BitcoinLikeOutputApi(const std::shared_ptr<OperationApi> &operation,
                                                   int32_t outputIndex) {
            _operation = operation;
            _outputIndex = outputIndex;
        }

        std::string BitcoinLikeOutputApi::getTransactionHash() {
            return _operation->getBackend().bitcoinTransaction.getValue().hash;
        }

        int32_t BitcoinLikeOutputApi::getOutputIndex() {
            return _outputIndex;
        }

        std::shared_ptr<api::Amount> BitcoinLikeOutputApi::getValue() {
            return std::make_shared<Amount>(_operation->getAccount()->getWallet()->getCurrency(), 0, getOuput().value);
        }

        std::vector<uint8_t> BitcoinLikeOutputApi::getScript() {
            return hex::toByteArray(getOuput().script);
        }

        optional<std::string> BitcoinLikeOutputApi::getAddress() {
            return getOuput().address.toOptional();
        }

        BitcoinLikeBlockchainExplorer::Output &BitcoinLikeOutputApi::getOuput() {
            return _operation->getBackend().bitcoinTransaction.getValue().outputs[_outputIndex];
        }
    }
}