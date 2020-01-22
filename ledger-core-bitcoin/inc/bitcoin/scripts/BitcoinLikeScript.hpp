/*
 *
 * BitcoinLikeScript.h
 * ledger-core
 *
 * Created by Pierre Pollastri on 03/04/2018.
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

#pragma once

#include <list>

#include <core/utils/Either.hpp>
#include <core/utils/Try.hpp>

#include <bitcoin/BitcoinLikeAddress.hpp>
#include <bitcoin/api/BitcoinLikeNetworkParameters.hpp>
#include <bitcoin/scripts/BitcoinLikeScriptOperators.hpp>

namespace ledger {
    namespace core {

        using BitcoinLikeScriptOpCode = btccore::opcodetype;

        class BitcoinLikeScriptChunk {
        public:
            explicit BitcoinLikeScriptChunk(BitcoinLikeScriptOpCode op);

            explicit BitcoinLikeScriptChunk(const std::vector<uint8_t> &bytes);

            const std::vector<uint8_t> &getBytes() const;

            bool isBytes() const;

            BitcoinLikeScriptOpCode getOpCode() const;

            bool isEqualTo(btccore::opcodetype code) const;

            bool sizeEqualsTo(std::size_t size) const;

            bool isOpCode() const;

        private:
            Either<std::vector<uint8_t>, BitcoinLikeScriptOpCode> _value;
        };

        struct BitcoinLikeScriptConfiguration {
            bool isSigned;
            std::string keychainEngine;

            BitcoinLikeScriptConfiguration(bool isSigned_,
                                           const std::string &keychainEngine_) : isSigned(isSigned_),
                                                                                 keychainEngine(keychainEngine_) {
                if (isSigned_ && keychainEngine_.empty()) {
                    throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                         "Keychain engine required when constructing signed scripts");
                }
            }

            BitcoinLikeScriptConfiguration(const BitcoinLikeScriptConfiguration &copy) {
                this->isSigned = copy.isSigned;
                this->keychainEngine = copy.keychainEngine;
            }

            BitcoinLikeScriptConfiguration &operator=(const BitcoinLikeScriptConfiguration &copy) {
                this->isSigned = copy.isSigned;
                this->keychainEngine = copy.keychainEngine;
                return *this;
            }
        };

        class BitcoinLikeScript {
        public:
            BitcoinLikeScript() : _configuration(BitcoinLikeScriptConfiguration(false, "")) {};

            BitcoinLikeScript(const BitcoinLikeScriptConfiguration &configuration) : _configuration(configuration) {};

            BitcoinLikeScript &operator<<(btccore::opcodetype op_code);

            BitcoinLikeScript &operator<<(const std::vector<uint8_t> &bytes);

            const BitcoinLikeScriptChunk &operator[](int index) const;

            std::size_t size() const;

            std::string toString() const;

            std::vector<uint8_t> serialize() const;

            const std::list<BitcoinLikeScriptChunk> &toList() const;

            bool isP2PKH() const;

            bool isP2SH() const;

            bool isP2WPKH() const;

            bool isP2WSH() const;

            Option<BitcoinLikeAddress> parseAddress(const api::Currency &currency) const;

            static Try<BitcoinLikeScript> parse(const std::vector<uint8_t> &script,
                                                const BitcoinLikeScriptConfiguration &configuration = BitcoinLikeScriptConfiguration(
                                                        false, ""));

            static BitcoinLikeScript fromAddress(const std::string &address, const api::Currency &currency);

        private:
            std::list<BitcoinLikeScriptChunk> _chunks;
            BitcoinLikeScriptConfiguration _configuration;
        };
    }
}
