/*
 *
 * BitcoinLikeInputApi
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
#include "BitcoinLikeInputApi.h"
#include <wallet/common/Amount.h>
#include <wallet/common/AbstractAccount.hpp>

namespace ledger {
    namespace core {

        BitcoinLikeInputApi::BitcoinLikeInputApi(const std::shared_ptr<OperationApi> &operation, int32_t inputIndex) {
            _operation = operation;
            _inputIndex = inputIndex;
        }

        optional<std::string> BitcoinLikeInputApi::getAddress() {
            return getInput().address.toOptional();
        }

        std::shared_ptr<api::Amount> BitcoinLikeInputApi::getValue() {
            if (getInput().value.isEmpty())
                return nullptr;
            return std::make_shared<Amount>(_operation->getAccount()->getWallet()->getCurrency(), 0, getInput().value.getValue());
        }

        bool BitcoinLikeInputApi::isCoinbase() {
            return getInput().coinbase.nonEmpty();
        }

        optional<std::string> BitcoinLikeInputApi::getCoinbase() {
            return getInput().coinbase.toOptional();
        }

        BitcoinLikeBlockchainExplorer::Input &BitcoinLikeInputApi::getInput() {
            return _operation->getBackend().bitcoinTransaction.getValue().inputs[_inputIndex];
        }

        optional<std::string> BitcoinLikeInputApi::getPreviousTxHash() {
            return getInput().previousTxHash.toOptional();
        }

        optional<int32_t> BitcoinLikeInputApi::getPreviousOutputIndex() {
            return getInput().previousTxOutputIndex.map<int32_t>([] (const uint32_t& v) {
                return (int32_t)v;
            }).toOptional();
        }
    }
}