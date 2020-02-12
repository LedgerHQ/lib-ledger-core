/*
 *
 * BitcoinLikeInput
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

#include <core/wallet/Amount.hpp>
#include <core/wallet/AbstractAccount.hpp>

#include <bitcoin/io/BitcoinLikeInput.hpp>
#include <bitcoin/operations/BitcoinLikeOperation.hpp>

namespace ledger {
    namespace core {

        BitcoinLikeInput::BitcoinLikeInput(
            const BitcoinLikeBlockchainExplorerTransaction &transaction,
            api::Currency const &currency,
            int32_t inputIndex) 
            : _transaction(transaction),  _inputIndex(inputIndex)
        {
            _currency = currency;
        }

        optional<std::string> BitcoinLikeInput::getAddress() {
            return getInput().address;
        }

        std::shared_ptr<api::Amount> BitcoinLikeInput::getValue() {
            if (getInput().value.isEmpty())
                return nullptr;
            return std::make_shared<Amount>(_currency, 0, getInput().value.getValue());
        }

        bool BitcoinLikeInput::isCoinbase() {
            return getInput().coinbase.nonEmpty();
        }

        optional<std::string> BitcoinLikeInput::getCoinbase() {
            return getInput().coinbase.toOptional();
        }

        BitcoinLikeBlockchainExplorerInput &BitcoinLikeInput::getInput() {
            return _transaction.inputs[_inputIndex];
        }

        optional<std::string> BitcoinLikeInput::getPreviousTxHash() {
            return getInput().previousTxHash.toOptional();
        }

        optional<int32_t> BitcoinLikeInput::getPreviousOutputIndex() {
            return getInput().previousTxOutputIndex.map<int32_t>([] (const uint32_t& v) {
                return (int32_t)v;
            }).toOptional();
        }

        std::vector<std::vector<uint8_t>> BitcoinLikeInput::getPublicKeys() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "std::vector<std::vector<uint8_t>> BitcoinLikeInput::getPublicKeys()");
        }

        std::vector<std::shared_ptr<api::DerivationPath>> BitcoinLikeInput::getDerivationPath() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "std::vector<std::shared_ptr<DerivationPath>> BitcoinLikeInput::getDerivationPath()");
        }

        std::shared_ptr<api::BitcoinLikeOutput> BitcoinLikeInput::getPreviousOuput() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "std::shared_ptr<api::BitcoinLikeOutput> BitcoinLikeInput::getPreviousOuput()");
        }

        std::vector<uint8_t> BitcoinLikeInput::getScriptSig() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "std::vector<uint8_t> BitcoinLikeInput::getScriptSig()");
        }

        std::shared_ptr<api::BitcoinLikeScript> BitcoinLikeInput::parseScriptSig() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "std::shared_ptr<api::BitcoinLikeScript> BitcoinLikeInput::parseScriptSig()");
        }

        void BitcoinLikeInput::setScriptSig(const std::vector<uint8_t> &scriptSig) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "void BitcoinLikeInput::setScriptSig(const std::vector<uint8_t> &scriptSig)");
        }

        void BitcoinLikeInput::pushToScriptSig(const std::vector<uint8_t> &data) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "void BitcoinLikeInput::pushToScriptSig(const std::vector<uint8_t> &data)");
        }

        void BitcoinLikeInput::setSequence(int32_t sequence) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "void BitcoinLikeInput::setSequence(int32_t sequence)");
        }

        int64_t BitcoinLikeInput::getSequence() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "int32_t BitcoinLikeInput::getSequence()");
        }

        void BitcoinLikeInput::setP2PKHSigScript(const std::vector<uint8_t> &signature) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "void BitcoinLikeInput::setP2PKHSigScript(const std::vector<uint8_t> &signature)");
        }

        void BitcoinLikeInput::getPreviousTransaction(const std::shared_ptr<api::BinaryCallback> & callback) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "void BitcoinLikeInput::getPreviousTransaction(const std::shared_ptr<api::BinaryCallback> &callback)");
        }
    }
}
