/*
 *
 * BitcoinLikeScript.cpp
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

#include "BitcoinLikeScript.h"
#include <bytes/BytesReader.h>
#include <bytes/BytesWriter.h>
#include <utils/hex.h>
#include <wallet/bitcoin/networks.hpp>
#include <api/KeychainEngines.hpp>
#include <crypto/HASH160.hpp>
#include <bitcoin/bech32/Bech32Factory.h>
#include <api/Currency.hpp>

namespace ledger {
    namespace core {

        Try<BitcoinLikeScript> BitcoinLikeScript::parse(const std::vector<uint8_t> &script,
                                                        const BitcoinLikeScriptConfiguration &configuration) {
            return Try<BitcoinLikeScript>::from([&]() -> BitcoinLikeScript {
                BytesReader reader(script);
                BitcoinLikeScript s(configuration);
                while (reader.hasNext()) {
                    auto byte = reader.readNextByte();
                    if (byte >= 0x01 && byte <= 0x4b) {
                        s << reader.read(byte);
                    } else if (byte == btccore::OP_PUSHDATA1) {
                        s << reader.read(reader.readNextByte());
                    } else if (byte == btccore::OP_PUSHDATA2) {
                        s << reader.read(reader.readNextBeBigInt(2).toUnsignedInt());
                    } else if (byte == btccore::OP_PUSHDATA4) {
                        s << reader.read(reader.readNextBeBigInt(4).toUnsignedInt());
                    } else {
                        s << (btccore::opcodetype) byte;
                    }
                }
                return s;
            });
        }

        BitcoinLikeScript &BitcoinLikeScript::operator<<(btccore::opcodetype op_code) {
            _chunks.push_back(BitcoinLikeScriptChunk(op_code));
            return *this;
        }

        BitcoinLikeScript &BitcoinLikeScript::operator<<(const std::vector<uint8_t> &bytes) {
            _chunks.push_back(BitcoinLikeScriptChunk(bytes));
            return *this;
        }

        std::string BitcoinLikeScript::toString() const {
            std::stringstream ss;
            bool first = true;
            for (auto &chunk : _chunks) {
                if (!first) {
                    ss << " ";
                } else {
                    first = false;
                }
                if (chunk.isBytes()) {
                    const auto &bytes = chunk.getBytes();
                    ss << "PUSHDATA(" << bytes.size() << ")[" << hex::toString(bytes) << "]";
                } else {
                    ss << btccore::GetOpName(chunk.getOpCode());
                }
            }
            return ss.str();
        }

        std::vector<uint8_t> BitcoinLikeScript::serialize() const {
            BytesWriter writer;
            for (auto &chunk : _chunks) {
                if (chunk.isBytes()) {
                    auto &bytes = chunk.getBytes();
                    auto size = bytes.size();
                    if (size <= 0x75) {
                        writer.writeByte((uint8_t) size).writeByteArray(bytes);
                    } else if (size <= 0xFF) {
                        writer.writeByte((uint8_t) size).writeByteArray(bytes);
                    } else if (size <= 0xFFFF) {
                        writer.writeLeValue<uint16_t>((uint16_t) size).writeByteArray(bytes);
                    } else {
                        writer.writeLeValue<uint32_t>((uint32_t) size).writeByteArray(bytes);
                    }
                } else {
                    writer.writeByte(chunk.getOpCode());
                }
            }
            return writer.toByteArray();
        }

        const std::list<BitcoinLikeScriptChunk> &BitcoinLikeScript::toList() const {
            return _chunks;
        }

        BitcoinLikeScript
        BitcoinLikeScript::fromAddress(const std::string &address, const api::Currency &currency) {
            auto a = BitcoinLikeAddress::parse(address, currency)->asBitcoinLikeAddress();
            BitcoinLikeScript script;
            if (a->isP2WPKH() || a->isP2WSH()) {
                script << btccore::OP_0 << a->getHash160();
            } else if (a->isP2PKH()) {
                script << btccore::OP_DUP << btccore::OP_HASH160 << a->getHash160() << btccore::OP_EQUALVERIFY
                       << btccore::OP_CHECKSIG;
            } else if (a->isP2SH()) {
                script << btccore::OP_HASH160 << a->getHash160() << btccore::OP_EQUAL;
            } else {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "Cannot create output script from {}.", address);
            }

            auto &additionalBIPS = currency.bitcoinLikeNetworkParameters.value().AdditionalBIPs;
            auto it = std::find(additionalBIPS.begin(), additionalBIPS.end(), "BIP115");
            if (it != additionalBIPS.end()) {
                script << hex::toByteArray(networks::BIP115_PARAMETERS.blockHash)
                       << networks::BIP115_PARAMETERS.blockHeight
                       << btccore::OP_CHECKBLOCKATHEIGHT;
            }

            return script;
        }

        bool BitcoinLikeScript::isP2PKH() const {
            if (_configuration.isSigned) {
                return _configuration.keychainEngine == api::KeychainEngines::BIP32_P2PKH;
            }
            return size() >= 5 && (*this)[0].isEqualTo(btccore::OP_DUP) && (*this)[1].isEqualTo(btccore::OP_HASH160)
                   && (*this)[2].sizeEqualsTo(20) && (*this)[3].isEqualTo(btccore::OP_EQUALVERIFY)
                   && (*this)[4].isEqualTo(btccore::OP_CHECKSIG);
        }

        bool BitcoinLikeScript::isP2SH() const {
            if (_configuration.isSigned) {
                return _configuration.keychainEngine == api::KeychainEngines::BIP49_P2SH;
            }
            return (size() >= 3 && (*this)[0].isEqualTo(btccore::OP_HASH160) && (*this)[1].sizeEqualsTo(20)
                    && (*this)[22].isEqualTo(btccore::OP_EQUAL));
        }

        bool BitcoinLikeScript::isP2WPKH() const {
            if (_configuration.isSigned) {
                return _configuration.keychainEngine == api::KeychainEngines::BIP173_P2WPKH;
            }
            return (size() == 2 && (*this)[0].isEqualTo(btccore::OP_0) && (*this)[2].sizeEqualsTo(20));
        }

        bool BitcoinLikeScript::isP2WSH() const {
            if (_configuration.isSigned) {
                return _configuration.keychainEngine == api::KeychainEngines::BIP173_P2WSH;
            }
            return (size() == 2 && (*this)[0].isEqualTo(btccore::OP_0) && (*this)[2].sizeEqualsTo(32));
        }

        std::size_t BitcoinLikeScript::size() const {
            return _chunks.size();
        }

        const BitcoinLikeScriptChunk &BitcoinLikeScript::operator[](int index) const {
            auto size = this->size();
            auto it = _chunks.begin();
            for (auto i = 0; i < index; i++)
                it++;
            return *it; // Make it breakable on purpose if you are trying to fetch something which doesn't exist the list will throw an exception
        }

        Option<BitcoinLikeAddress>
        BitcoinLikeScript::parseAddress(const api::Currency &currency) const {
            const auto &params = currency.bitcoinLikeNetworkParameters.value();
            HashAlgorithm hashAlgorithm(params.Identifier);
            if (isP2SH()) {
                //Signed : <ScriptSig> <PubKey>
                if (_configuration.isSigned) {
                    std::vector<uint8_t> script = {0x00, 0x14};
                    //Hash160 of public key
                    auto publicKeyHash160 = HASH160::hash((*this)[1].getBytes(), hashAlgorithm);
                    script.insert(script.end(), publicKeyHash160.begin(), publicKeyHash160.end());
                    return Option<BitcoinLikeAddress>(
                            BitcoinLikeAddress(currency,
                                               HASH160::hash(script, hashAlgorithm),
                                               api::KeychainEngines::BIP49_P2SH));
                }
                //Unsigned : OP_HASH160 <PubKeyHash> OP_EQUAL
                return Option<BitcoinLikeAddress>(
                        BitcoinLikeAddress(currency, (*this)[1].getBytes(), api::KeychainEngines::BIP49_P2SH));
            } else if (isP2PKH()) {
                // Signed : <ScriptSig> <PubKey>
                auto index = _configuration.isSigned ? 1 : 2;
                // Unsigned : OP_DUP OP_HASH160 <PubKeyHash> OP_EQUALVERIFY OP_CHECKSIG
                return Option<BitcoinLikeAddress>(
                        BitcoinLikeAddress(currency, (*this)[index].getBytes(), api::KeychainEngines::BIP32_P2PKH));
            } else if (isP2WPKH()) {
                // <OP_0> <PubKeyHash>
                return Option<BitcoinLikeAddress>(
                        BitcoinLikeAddress(currency, (*this)[1].getBytes(), api::KeychainEngines::BIP173_P2WPKH));
            } else if (isP2WSH()) {
                // <OP_0> <WitnessScript>
                return Option<BitcoinLikeAddress>(
                        BitcoinLikeAddress(currency, (*this)[1].getBytes(), api::KeychainEngines::BIP173_P2WSH));
            }
            return Option<BitcoinLikeAddress>();
        }


        BitcoinLikeScriptChunk::BitcoinLikeScriptChunk(BitcoinLikeScriptOpCode op) : _value(op) {

        }

        BitcoinLikeScriptChunk::BitcoinLikeScriptChunk(const std::vector<uint8_t> &bytes) : _value(bytes) {

        }

        const std::vector<uint8_t> &BitcoinLikeScriptChunk::getBytes() const {
            return _value.getLeft();
        }

        bool BitcoinLikeScriptChunk::isBytes() const {
            return _value.isLeft();
        }

        BitcoinLikeScriptOpCode BitcoinLikeScriptChunk::getOpCode() const {
            return _value.getRight();
        }

        bool BitcoinLikeScriptChunk::isOpCode() const {
            return _value.isRight();
        }

        bool BitcoinLikeScriptChunk::isEqualTo(btccore::opcodetype code) const {
            return isOpCode() && getOpCode() == code;
        }

        bool BitcoinLikeScriptChunk::sizeEqualsTo(std::size_t size) const {
            return isBytes() && getBytes().size() == size;
        }
    }
}