/*
 *
 * EthereumLikeAddress
 * ledger-core
 *
 * Created by El Khalil Bellakrid on 08/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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

#include "EthereumLikeAddress.h"
#include <utils/Exception.hpp>
#include <math/Base58.hpp>
#include <collections/vector.hpp>
#include <utils/hex.h>
#include <crypto/Keccak.h>

namespace ledger {
    namespace core {

        EthereumLikeAddress::EthereumLikeAddress(const api::Currency& currency,
                            const std::vector<uint8_t>& keccak256,
                            const Option<std::string>& derivationPath) :
                _params(currency.ethereumLikeNetworkParameters.value()),
                _derivationPath(derivationPath),
                _keccak256(keccak256),
                AbstractAddress(currency, derivationPath)
        {

        }

        std::vector<uint8_t> EthereumLikeAddress::getVersion() {
                throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "EthereumLikeAddress::getVersion is not implemented yet");
        }

        std::vector<uint8_t> EthereumLikeAddress::getHash160() {
                throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "EthereumLikeAddress::getHash160 is not implemented yet");
        }

        api::EthereumLikeNetworkParameters EthereumLikeAddress::getNetworkParameters() {
                throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "EthereumLikeAddress::getNetworkParameters is not implemented yet");
        }

        std::string EthereumLikeAddress::toBase58() {

            auto byteToDigitEIP55 = [](uint8_t byte, uint8_t against) -> char {
                bool uppercase = against > 0x8;
                byte = (uint8_t) (0xF < byte ? 0xF : byte);
                if (byte < 0xA) {
                    return '0' + byte;
                } else if (uppercase) {
                    return (char)('A' + (byte - 0xA));
                } else {
                    return (char)('a' + (byte - 0xA));
                }
            };

            auto keccakHash = Keccak::keccak256(hex::toString(_keccak256));

            std::string addressEIP55(_keccak256.size() * 2, '0');
            for (int i = 0; i < _keccak256.size(); i++) {
                addressEIP55[i * 2] = byteToDigitEIP55(_keccak256[i] >> 4, keccakHash[i] >> 4);
                addressEIP55[i * 2 + 1] = byteToDigitEIP55((uint8_t) (_keccak256[i] & 0xF), (uint8_t) (keccakHash[i] & 0xF));
            }
            return "0x" + addressEIP55;
        }

        std::experimental::optional<std::string> EthereumLikeAddress::getDerivationPath() {
            return _derivationPath.toOptional();
        }

        std::string EthereumLikeAddress::toString() {
            return toBase58();
        }

        std::shared_ptr<AbstractAddress> EthereumLikeAddress::parse(const std::string& address, const api::Currency& currency,
                                                      const Option<std::string>& derivationPath) {
            auto result = Try<std::shared_ptr<ledger::core::AbstractAddress>>::from([&] () {
                return fromBase58(address, currency, derivationPath);
            });
            return std::dynamic_pointer_cast<AbstractAddress>(result.toOption().getValueOr(nullptr));
        }

        std::shared_ptr<EthereumLikeAddress> EthereumLikeAddress::fromBase58(const std::string& address,
                                                               const api::Currency& currency,
                                                               const Option<std::string>& derivationPath) {
            //Remove 0x
            auto tmpAddress = address.substr(2, address.length() - 2);
            //TODO: EIP55
            return std::make_shared<ledger::core::EthereumLikeAddress>(currency, hex::toByteArray(tmpAddress), derivationPath);
        }
    }
}
