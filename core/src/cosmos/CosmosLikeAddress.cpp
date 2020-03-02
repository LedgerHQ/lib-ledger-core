/*
 *
 * CosmosLikeAddress
 *
 * Created by El Khalil Bellakrid on 13/06/2019.
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


#include <cosmos/CosmosLikeAddress.hpp>

#include <utils/Exception.hpp>
#include <math/Base58.hpp>
#include <collections/vector.hpp>
#include <utils/hex.h>
#include <crypto/Keccak.h>
#include <collections/DynamicObject.hpp>

#include <wallet/cosmos/CosmosNetworks.hpp>
#include <cosmos/bech32/CosmosBech32.hpp>
#include <api/CosmosBech32Type.hpp>
#include <cosmos/bech32/CosmosLikeBech32ParametersHelpers.hpp>

namespace ledger {
    namespace core {

        CosmosLikeAddress::CosmosLikeAddress(const ledger::core::api::Currency &currency,
                                             const std::vector<uint8_t> &hash160,
                                             const std::vector<uint8_t> &version,
                                             api::CosmosBech32Type type,
                                             const Option<std::string> &derivationPath) :
            _params(networks::getCosmosLikeNetworkParameters(currency.name)),
            _derivationPath(derivationPath),
            _hash160(hash160),
            _version(version),
            _type(type),
            AbstractAddress(currency, derivationPath) {

        }

        std::vector<uint8_t> CosmosLikeAddress::getVersion() {
            return _version;
        }

        std::vector<uint8_t> ledger::core::CosmosLikeAddress::getHash160() {
            return _hash160;
        }

        api::CosmosLikeNetworkParameters CosmosLikeAddress::getNetworkParameters() {
            return _params;
        }

        std::string CosmosLikeAddress::toBech32() {
            auto bech32 = std::make_shared<CosmosBech32>(_type);
            return bech32->encode(_hash160, std::vector<uint8_t>());
        }

        std::experimental::optional<std::string> CosmosLikeAddress::getDerivationPath() {
            return _derivationPath.toOptional();
        }

        std::string CosmosLikeAddress::toString() {
            return toBech32();
        }

        std::shared_ptr<AbstractAddress>
        CosmosLikeAddress::parse(const std::string &address,
                                 const api::Currency &currency,
                                 const Option<std::string> &derivationPath) {
            auto result = Try<std::shared_ptr<ledger::core::AbstractAddress>>::from([&]() {
                return fromBech32(address, currency, derivationPath);
            });
            return std::dynamic_pointer_cast<AbstractAddress>(result.toOption().getValueOr(nullptr));
        }

        std::shared_ptr<CosmosLikeAddress> CosmosLikeAddress::fromBech32(const std::string &address,
                                                                         const api::Currency &currency,
                                                                         const Option<std::string> &derivationPath) {
            auto& params = networks::getCosmosLikeNetworkParameters(currency.name);
            auto type = address.find(cosmos::getBech32Params(api::CosmosBech32Type::ADDRESS_VAL).hrp) == std::string::npos ? api::CosmosBech32Type::ADDRESS : api::CosmosBech32Type::ADDRESS_VAL;
            auto bech32 = std::make_shared<CosmosBech32>(type);
            auto decoded = bech32->decode(address);
            // Second supposed to be hash160 of pubKey
            if (decoded.second.size() != 20) {
                throw Exception(api::ErrorCode::INVALID_BECH32_FORMAT, "Invalid decoded public key hash");
            }
            return std::make_shared<ledger::core::CosmosLikeAddress>(currency, decoded.second, params.AddressPrefix, type, derivationPath);
        }
    }
}
