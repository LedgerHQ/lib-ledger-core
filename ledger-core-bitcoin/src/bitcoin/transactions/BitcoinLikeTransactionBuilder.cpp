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

#include <core/utils/Hex.hpp>
#include <core/wallet/Amount.hpp>

#include <bitcoin/BitcoinNetworks.hpp>
#include <bitcoin/api/BitcoinLikeAddress.hpp>
#include <bitcoin/scripts/BitcoinLikeScript.hpp>
#include <bitcoin/scripts/BitcoinLikeInternalScript.hpp>
#include <bitcoin/transactions/BitcoinLikeTransactionBuilder.hpp>

namespace ledger {
    namespace core {

        static const std::shared_ptr<BigInt> DEFAULT_MAX_AMOUNT = std::make_shared<BigInt>(BigInt::fromHex(
                "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
        )); // Max ui512

        BitcoinLikeTransactionBuilder::BitcoinLikeTransactionBuilder(const BitcoinLikeTransactionBuilder &cpy)
                : _request(std::make_shared<BigInt>(networks::getBitcoinLikeNetworkParameters(cpy._currency.name).DustAmount)) {
            _currency = cpy._currency;
            _build = cpy._build;
            _request = cpy._request;
            _context = cpy._context;
            _logger = cpy._logger;
        }

        BitcoinLikeTransactionBuilder::BitcoinLikeTransactionBuilder(
                const std::shared_ptr<api::ExecutionContext> &context, const api::Currency &currency,
                const std::shared_ptr<spdlog::logger> &logger,
                const BitcoinLikeTransactionBuildFunction &buildFunction) :
                _request(std::make_shared<BigInt>(networks::getBitcoinLikeNetworkParameters(currency.name).DustAmount)) {
            _currency = currency;
            _build = buildFunction;
            _context = context;
            _logger = logger;
            _request.wipe = false;
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::addInput(const std::string &transactionHash, int32_t index, int32_t sequence) {
            //Fix: use uniform initialization
            using InputType = std::tuple<std::string, int32_t, uint32_t>;
            InputType new_input{transactionHash, index, sequence};
            _request.inputs.emplace_back(std::move(new_input));
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::addOutput(const std::shared_ptr<api::Amount> &amount,
                                                 const std::shared_ptr<api::BitcoinLikeScript> &script) {
            auto a = std::dynamic_pointer_cast<Amount>(std::dynamic_pointer_cast<Amount>(amount))->value();
            _request.outputs.push_back(std::tuple<std::shared_ptr<BigInt>, std::shared_ptr<api::BitcoinLikeScript>>(a, script));
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::addChangePath(const std::string &path) {
            _request.changePaths.push_back(path);
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::excludeUTXO(const std::string &transactionHash, int32_t outputIndex) {
            //Fix: use uniform initialization
            using UTXOType = std::tuple<std::string, int32_t>;
            UTXOType excluded_UTXO{transactionHash, outputIndex};
            _request.excludedUTXO.push_back(excluded_UTXO);
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::setNumberOfChangeAddresses(int32_t count) {
            _request.changeCount = count;
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::pickInputs(api::BitcoinLikePickingStrategy strategy, int32_t sequence) {
            //Fix: use uniform initialization
            using UTXOPickerType = std::tuple<api::BitcoinLikePickingStrategy, uint32_t>;
            UTXOPickerType new_UTXO_picker{strategy, sequence};
            _request.UTXOPicker = Option<UTXOPickerType>(std::move(new_UTXO_picker));
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::sendToAddress(const std::shared_ptr<api::Amount> &amount,
                                                     const std::string &address) {
            addOutput(amount, createSendScript(address));
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::wipeToAddress(const std::string &address) {
            //First reset request
            reset();
            //Wipe mode
            _request.wipe = true;
            //We don't have the amount yet, will be set when we fill outputs in BitcoinLikeUTXOPicker
            auto a = std::shared_ptr<BigInt>();
            auto script = createSendScript(address);
            _request.outputs.push_back(std::tuple<std::shared_ptr<BigInt>, std::shared_ptr<api::BitcoinLikeScript>>(a, script));
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::setFeesPerByte(const std::shared_ptr<api::Amount> &fees) {
            _request.feePerByte = std::dynamic_pointer_cast<Amount>(fees)->value();
            return shared_from_this();
        }

        void
        BitcoinLikeTransactionBuilder::build(const std::function<void(std::shared_ptr<api::BitcoinLikeTransaction>, std::experimental::optional<api::Error>)> & callback) {
            build().callback(_context, callback);
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::setMaxAmountOnChange(const std::shared_ptr<api::Amount> &amount) {
            _request.maxChange = std::dynamic_pointer_cast<Amount>(amount)->value();
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::setMinAmountOnChange(const std::shared_ptr<api::Amount> &amount) {
            _request.minChange = std::dynamic_pointer_cast<Amount>(amount)->value();
            return shared_from_this();
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder> BitcoinLikeTransactionBuilder::clone() {
            return std::make_shared<BitcoinLikeTransactionBuilder>(*this);
        }

        void BitcoinLikeTransactionBuilder::reset() {
            _request = BitcoinLikeTransactionBuildRequest(std::make_shared<BigInt>(
                    networks::getBitcoinLikeNetworkParameters(_currency.name).DustAmount)
            );
        }

        Future<std::shared_ptr<api::BitcoinLikeTransaction>> BitcoinLikeTransactionBuilder::build() {
            return _build(_request);
        }

        std::shared_ptr<api::BitcoinLikeScript>
        BitcoinLikeTransactionBuilder::createSendScript(const std::string &address) {
            auto a = std::dynamic_pointer_cast<BitcoinLikeAddress>(BitcoinLikeAddress::parse(address, _currency));
            if (a == nullptr) {
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Invalid address {}", address);
            }
            BitcoinLikeScript script;
            if (a->isP2PKH()) {
                script << btccore::OP_DUP << btccore::OP_HASH160 << a->getHash160() << btccore::OP_EQUALVERIFY
                       << btccore::OP_CHECKSIG;
            } else if (a->isP2SH()) {
                script << btccore::OP_HASH160 << a->getHash160() << btccore::OP_EQUAL;
            } else if (a->isP2WPKH() || a->isP2WSH()) {
                script << btccore::OP_0 << a->getHash160();
            } else {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Cannot create output script from {}.", address);
            }

            auto &additionalBIPS = networks::getBitcoinLikeNetworkParameters(_currency.name).AdditionalBIPs;
            auto it = std::find(additionalBIPS.begin(), additionalBIPS.end(), "BIP115");
            if (it != additionalBIPS.end()) {
                script << hex::toByteArray(networks::BIP115_PARAMETERS.blockHash)
                       << networks::BIP115_PARAMETERS.blockHeight
                       << btccore::OP_CHECKBLOCKATHEIGHT;
            }
            return std::make_shared<BitcoinLikeInternalScript>(script);
        }

        BitcoinLikeTransactionBuildRequest::BitcoinLikeTransactionBuildRequest(
                const std::shared_ptr<BigInt> &minChange) {
            this->minChange = minChange;
            this->maxChange = DEFAULT_MAX_AMOUNT;
        }
    }
}