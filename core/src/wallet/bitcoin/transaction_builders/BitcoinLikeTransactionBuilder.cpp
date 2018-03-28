/*
 *
 * BitcoinLikeTransactionBuilder.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/03/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#include "BitcoinLikeTransactionBuilder.h"
#include <wallet/common/Amount.h>

namespace ledger {
    namespace core {

        static const std::shared_ptr<BigInt> DEFAULT_MAX_AMOUNT = std::make_shared<BigInt>(BigInt::fromHex(
                "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
        )); // Max ui512

        BitcoinLikeTransactionBuilder::BitcoinLikeTransactionBuilder(const BitcoinLikeTransactionBuilder &cpy)
                : _request(std::make_shared<BigInt>(cpy._params.DustAmount)) {
            _params = cpy._params;
            _build = cpy._build;
            _request = cpy._request;
        }

        BitcoinLikeTransactionBuilder::BitcoinLikeTransactionBuilder(
                const api::BitcoinLikeNetworkParameters& params,
                const BitcoinLikeTransactionBuildFunction &buildFunction)
                : _request(std::make_shared<BigInt>(params.DustAmount)) {
            _params = params;
            _build = buildFunction;
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::addInput(const std::string &transactionHash, int32_t index, int32_t sequence) {
            _request.inputs.push_back({transactionHash, index, sequence});
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::addOutput(const std::shared_ptr<api::Amount> &amount,
                                                 const std::shared_ptr<api::BitcoinLikeScript> &script) {
            auto a = std::dynamic_pointer_cast<BigInt>(std::dynamic_pointer_cast<Amount>(amount)->toBigInt());
            _request.outputs.push_back(std::tuple<std::shared_ptr<BigInt>, std::shared_ptr<api::BitcoinLikeScript>>(a, script));
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::addChangePath(const std::string &path) {
            _request.changePaths.push_back(path);
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::excludeUtxo(const std::string &transactionHash, int32_t outputIndex) {
            _request.excludedUtxo.push_back({transactionHash, outputIndex});
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::setNumberOfChangeAddresses(int32_t count) {
            _request.changeCount = count;
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::pickInputs(api::BitcoinLikePickingStrategy strategy, int32_t sequence) {
            _request.utxoPicker = Option<std::tuple<api::BitcoinLikePickingStrategy, uint32_t>>({strategy, sequence});
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::sendToAddress(const std::shared_ptr<api::Amount> &amount,
                                                     const std::string &address) {
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::setFeesPerByte(const std::shared_ptr<api::Amount> &fees) {
            return shared_from_this();
        }

        void
        BitcoinLikeTransactionBuilder::build(const std::shared_ptr<api::BitcoinLikeTransactionCallback> &callback) {

        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::setMaxAmountOnChange(const std::shared_ptr<api::Amount> &amount) {
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::setMinAmountOnChange(const std::shared_ptr<api::Amount> &amount) {
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder> BitcoinLikeTransactionBuilder::clone() {
            return std::make_shared<BitcoinLikeTransactionBuilder>(*this);
        }

        void BitcoinLikeTransactionBuilder::reset() {
            _request = BitcoinLikeTransactionBuildRequest(std::make_shared<BigInt>(_params.DustAmount));
        }

        BitcoinLikeTransactionBuildRequest::BitcoinLikeTransactionBuildRequest(
                const std::shared_ptr<BigInt> &minChange) {
            this->minChange = minChange;
            this->maxChange = DEFAULT_MAX_AMOUNT;
        }
    }
}