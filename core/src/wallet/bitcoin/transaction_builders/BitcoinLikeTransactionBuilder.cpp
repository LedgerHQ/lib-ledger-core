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
#include <api/BitcoinLikeAddress.hpp>
#include <wallet/common/Amount.h>
#include <api/BitcoinLikeTransactionCallback.hpp>
#include <wallet/bitcoin/scripts/BitcoinLikeScript.h>
#include <wallet/bitcoin/api_impl/BitcoinLikeScriptApi.h>
#include <wallet/bitcoin/networks.hpp>
#include <utils/hex.h>
#include <wallet/currencies.hpp>

namespace ledger {
    namespace core {

        static const std::shared_ptr<BigInt> DEFAULT_MAX_AMOUNT = std::make_shared<BigInt>(BigInt::fromHex(
                "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
                "FFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFFF"
        )); // Max ui512

        BitcoinLikeTransactionBuilder::BitcoinLikeTransactionBuilder(const BitcoinLikeTransactionBuilder &cpy)
                : _request(std::make_shared<BigInt>(cpy._currency.bitcoinLikeNetworkParameters.value().DustAmount)) {
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
                _request(std::make_shared<BigInt>(currency.bitcoinLikeNetworkParameters.value().DustAmount)) {
            _currency = currency;
            _build = buildFunction;
            _context = context;
            _logger = logger;
            _request.wipe = false;
        }

        std::shared_ptr<api::BitcoinLikeTransactionBuilder>
        BitcoinLikeTransactionBuilder::addInput(const std::string &transactionHash, int32_t index, int32_t sequence) {
            _request.inputs.push_back(BitcoinLikeTransactionInputDescriptor{
                transactionHash, static_cast<uint64_t>(index), static_cast<uint64_t>(sequence)});

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
        BitcoinLikeTransactionBuilder::excludeUtxo(const std::string &transactionHash, int32_t outputIndex) {
            _request.excludedUtxos.insert(BitcoinLikeTransactionUtxoDescriptor{
                transactionHash, static_cast<uint64_t>(outputIndex)});
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
            UTXOPickerType new_utxo_picker{strategy, sequence};
            _request.utxoPicker = Option<UTXOPickerType>(std::move(new_utxo_picker));
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
            //We don't have the amount yet, will be set when we fill outputs in BitcoinLikeUtxoPicker
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
        BitcoinLikeTransactionBuilder::build(const std::shared_ptr<api::BitcoinLikeTransactionCallback> &callback) {
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
                    _currency.bitcoinLikeNetworkParameters.value().DustAmount)
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

            // BCH has a fake P2WPKH and P2WSH
            // So for cash addresses we should still use P2PKH or P2SH
            if (a->isP2PKH() || (a->isP2WPKH() && _currency.name == currencies::BITCOIN_CASH.name)) {
                script << btccore::OP_DUP << btccore::OP_HASH160 << a->getHash160() << btccore::OP_EQUALVERIFY
                       << btccore::OP_CHECKSIG;
            } else if (a->isP2SH() || (a->isP2WSH() && _currency.name == currencies::BITCOIN_CASH.name)) {
                script << btccore::OP_HASH160 << a->getHash160() << btccore::OP_EQUAL;
            } else if (a->isP2WPKH() || a->isP2WSH()) {
                script << btccore::OP_0 << a->getHash160();
            } else {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Cannot create output script from {}.", address);
            }

            auto &additionalBIPS = _currency.bitcoinLikeNetworkParameters.value().AdditionalBIPs;
            auto it = std::find(additionalBIPS.begin(), additionalBIPS.end(), "BIP115");
            if (it != additionalBIPS.end()) {
                script << hex::toByteArray(networks::BIP115_PARAMETERS.blockHash)
                       << networks::BIP115_PARAMETERS.blockHeight
                       << btccore::OP_CHECKBLOCKATHEIGHT;
            }
            return std::make_shared<BitcoinLikeScriptApi>(script);
        }

        BitcoinLikeTransactionBuildRequest::BitcoinLikeTransactionBuildRequest(
                const std::shared_ptr<BigInt> &minChange) {
            this->minChange = minChange;
            this->maxChange = DEFAULT_MAX_AMOUNT;
        }

        bool BitcoinLikeTransactionUtxoDescriptor::operator==(BitcoinLikeTransactionUtxoDescriptor const &other) const
        {
            return transactionHash == other.transactionHash && outputIndex == other.outputIndex;
        }

        size_t BitcoinLikeTransactionUtxoDescriptorHash::operator()(BitcoinLikeTransactionUtxoDescriptor const& utxo) const
        {
            return std::hash<std::string>()(utxo.transactionHash) ^ std::hash<uint32_t>()(utxo.outputIndex);
        }

    }
}
