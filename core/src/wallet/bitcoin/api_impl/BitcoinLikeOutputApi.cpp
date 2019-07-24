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
#include <utils/Exception.hpp>
#include <wallet/bitcoin/scripts/BitcoinLikeScript.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeScriptApi.h>

namespace ledger {
    namespace core {

        BitcoinLikeOutputApi::BitcoinLikeOutputApi(const std::shared_ptr<OperationApi> &operation,
                                                   int32_t outputIndex) : _backend(operation) {
            _outputIndex = outputIndex;
            _currency = operation->getAccount()->getWallet()->getCurrency();
        }

        BitcoinLikeOutputApi::BitcoinLikeOutputApi(const BitcoinLikeBlockchainExplorerOutput &output, const api::Currency& currency) : _backend(output) {
            _outputIndex = static_cast<int32_t>(output.index);
            _currency = currency;
        }

        std::string BitcoinLikeOutputApi::getTransactionHash() {
            if (_backend.isLeft())
                return _backend.getLeft()->getBackend().bitcoinTransaction.getValue().hash;
            else
                return _backend.getRight().transactionHash;
        }

        int32_t BitcoinLikeOutputApi::getOutputIndex() {
            return _outputIndex;
        }

        std::shared_ptr<api::Amount> BitcoinLikeOutputApi::getValue() {
            return std::make_shared<Amount>(_currency, 0, getOutput().value);
        }

        std::vector<uint8_t> BitcoinLikeOutputApi::getScript() {
            return hex::toByteArray(getOutput().script);
        }

        optional<std::string> BitcoinLikeOutputApi::getAddress() {
            return getOutput().address.toOptional();
        }

        BitcoinLikeBlockchainExplorerOutput &BitcoinLikeOutputApi::getOutput() {
            if (_backend.isLeft())
                return _backend.getLeft()->getBackend().bitcoinTransaction.getValue().outputs[_outputIndex];
            return _backend.getRight();
        }

        std::shared_ptr<api::BitcoinLikeScript> BitcoinLikeOutputApi::parseScript() {
            auto result = BitcoinLikeScript::parse(getScript());
            if (result.isFailure())
                throw result.getFailure();
            return std::make_shared<BitcoinLikeScriptApi>(result.getValue());
        }

        std::shared_ptr<api::DerivationPath> BitcoinLikeOutputApi::getDerivationPath() {
            return _path;
        }

        std::experimental::optional<int64_t> BitcoinLikeOutputApi::getBlockHeight() {
            return static_cast<int64_t>(getOutput().blockHeight.getValueOr(std::numeric_limits<uint64_t>::max()));
        }

        BitcoinLikeOutputApi::BitcoinLikeOutputApi(const BitcoinLikeBlockchainExplorerOutput &output,
                                                   const api::Currency &currency,
                                                   const std::shared_ptr<api::DerivationPath> &path)
                : BitcoinLikeOutputApi(output, currency) {
            _path = path;
        }

        const BigInt &BitcoinLikeOutputApi::value() {
            return getOutput().value;
        }
    }
}