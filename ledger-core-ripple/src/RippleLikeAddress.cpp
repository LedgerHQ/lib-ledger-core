/*
 *
 * RippleLikeAddress
 *
 * Created by El Khalil Bellakrid on 05/01/2019.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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


#include "RippleLikeAddress.h"
#include <utils/Exception.hpp>
#include <math/Base58.hpp>
#include <collections/vector.hpp>
#include <utils/hex.h>
#include <crypto/Keccak.h>
#include <wallet/ripple/rippleNetworks.h>
#include <collections/DynamicObject.hpp>
namespace ledger {
    namespace core {

        RippleLikeAddress::RippleLikeAddress(const ledger::core::api::Currency &currency,
                                             const std::vector<uint8_t> &hash160,
                                             const std::vector<uint8_t> &version,
                                             const Option<std::string> &derivationPath) :
                _params(currency.rippleLikeNetworkParameters.value()),
                _derivationPath(derivationPath),
                _hash160(hash160),
                _version(version),
                AbstractAddress(currency, derivationPath) {

        }

        std::vector<uint8_t> RippleLikeAddress::getVersion() {
            return _version;
        }

        std::vector<uint8_t> ledger::core::RippleLikeAddress::getHash160() {
            return _hash160;
        }

        api::RippleLikeNetworkParameters RippleLikeAddress::getNetworkParameters() {
            return _params;
        }

        std::string RippleLikeAddress::toBase58() {
            auto config = std::make_shared<DynamicObject>();
            config->putString("networkIdentifier", _params.Identifier);
            config->putString("base58Dictionary", networks::RIPPLE_DIGITS);
            config->putBoolean("useNetworkDictionary", true);
            return Base58::encodeWithChecksum(vector::concat(_version, _hash160), config);
        }

        std::experimental::optional<std::string> RippleLikeAddress::getDerivationPath() {
            return _derivationPath.toOptional();
        }

        std::string RippleLikeAddress::toString() {
            return toBase58();
        }

        std::shared_ptr<AbstractAddress>
        RippleLikeAddress::parse(const std::string &address, const api::Currency &currency,
                                 const Option<std::string> &derivationPath) {
            auto result = Try<std::shared_ptr<ledger::core::AbstractAddress>>::from([&]() {
                return fromBase58(address, currency, derivationPath);
            });
            return std::dynamic_pointer_cast<AbstractAddress>(result.toOption().getValueOr(nullptr));
        }

        std::shared_ptr<RippleLikeAddress> RippleLikeAddress::fromBase58(const std::string &address,
                                                                         const api::Currency &currency,
                                                                         const Option<std::string> &derivationPath) {
            auto& params = currency.rippleLikeNetworkParameters.value();
            auto config = std::make_shared<DynamicObject>();
            config->putString("networkIdentifier", params.Identifier);
            config->putString("base58Dictionary", networks::RIPPLE_DIGITS);
            config->putBoolean("useNetworkDictionary", true);
            auto decoded = Base58::checkAndDecode(address, config);
            if (decoded.isFailure()) {
                throw decoded.getFailure();
            }
            auto value = decoded.getValue();

            //Check decoded address size
            if (value.size() <= 20) {
                throw Exception(api::ErrorCode::INVALID_BASE58_FORMAT, "Invalid address : Invalid base 58 format");
            }

            std::vector<uint8_t> hash160(value.end() - 20, value.end());
            std::vector<uint8_t> version(value.begin(), value.end() - 20);
            return std::make_shared<ledger::core::RippleLikeAddress>(currency, hash160, version, derivationPath);
        }
    }
}