/*
 *
 * BitcoinLikeAddress
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/12/2016.
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
#include "BitcoinLikeAddress.hpp"
#include "../utils/djinni_helpers.hpp"
#include "../bytes/BytesWriter.h"
#include "../math/Base58.hpp"
#include "../collections/vector.hpp"
#include "../utils/Exception.hpp"
#include <collections/DynamicObject.hpp>
#include <bitcoin/bech32/Bech32.h>
#include <bitcoin/bech32/Bech32Factory.h>
using namespace ledger::core;

ledger::core::BitcoinLikeAddress::BitcoinLikeAddress(const ledger::core::api::Currency &currency,
                                                     const std::vector<uint8_t>& hash160,
                                                     const std::vector<uint8_t>& version,
                                                     const Option<std::string>& derivationPath) :
        _params(currency.bitcoinLikeNetworkParameters.value()),
        _version(version),
        _derivationPath(derivationPath),
        _hash160(hash160),
        AbstractAddress(currency, derivationPath)
{

}

std::vector<uint8_t> ledger::core::BitcoinLikeAddress::getVersion() {
    return _version;
}

std::vector<uint8_t> ledger::core::BitcoinLikeAddress::getHash160() {
    return _hash160;
}

ledger::core::api::BitcoinLikeNetworkParameters ledger::core::BitcoinLikeAddress::getNetworkParameters() {
    return _params;
}

std::string ledger::core::BitcoinLikeAddress::toBase58() {
    auto config = std::make_shared<DynamicObject>();
    config->putString("networkIdentifier", _params.Identifier);
    return Base58::encodeWithChecksum(vector::concat(_version, _hash160), config);
}

std::string toBech32Helper(const std::vector<uint8_t> &version,
                           const std::vector<uint8_t> &hash160,
                           const api::BitcoinLikeNetworkParameters &params) {
    auto bech32 = Bech32Factory::newBech32Instance(params.Identifier);
    return bech32->encode(hash160, version);
}

std::string ledger::core::BitcoinLikeAddress::toBech32() {
    return toBech32Helper(_version, _hash160, _params);
}

bool ledger::core::BitcoinLikeAddress::isP2SH() {
    return _version == _params.P2SHVersion;
}

bool ledger::core::BitcoinLikeAddress::isP2PKH() {
    return _version == _params.P2PKHVersion;
}

std::experimental::optional<std::string> ledger::core::BitcoinLikeAddress::getDerivationPath() {
    return _derivationPath.toOptional();
}

std::string ledger::core::BitcoinLikeAddress::toBase58() const {
    auto config = std::make_shared<DynamicObject>();
    config->putString("networkIdentifier", _params.Identifier);
    return Base58::encodeWithChecksum(vector::concat(_version, _hash160), config);
}

std::string ledger::core::BitcoinLikeAddress::toBech32() const {
    return toBech32Helper(_version, _hash160, _params);
}

std::shared_ptr<ledger::core::AbstractAddress>
ledger::core::BitcoinLikeAddress::parse(const std::string &address, const ledger::core::api::Currency &currency,
                                        const Option<std::string>& derivationPath) {
    auto result = Try<std::shared_ptr<ledger::core::AbstractAddress>>::from([&] () {
        return fromBase58(address, currency, derivationPath);
    });
    return std::dynamic_pointer_cast<AbstractAddress>(result.toOption().getValueOr(nullptr));
}

std::string ledger::core::BitcoinLikeAddress::toString() {
    return toBase58();
}

std::shared_ptr<BitcoinLikeAddress> ledger::core::BitcoinLikeAddress::fromBase58(const std::string &address,
                                                                              const api::Currency &currency,
                                                                              const Option<std::string>& derivationPath) {
    auto& params = currency.bitcoinLikeNetworkParameters.value();
    auto config = std::make_shared<DynamicObject>();
    config->putString("networkIdentifier", params.Identifier);
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
    if (version != params.P2PKHVersion && version != params.P2SHVersion) {
        throw Exception(api::ErrorCode::INVALID_VERSION, "Address version doesn't belong to the given network parameters");
    }
    return std::make_shared<ledger::core::BitcoinLikeAddress>(currency, hash160, version, derivationPath);
}

std::shared_ptr<BitcoinLikeAddress> ledger::core::BitcoinLikeAddress::fromBech32(const std::string& address,
                                                                                 const api::Currency& currency,
                                                                                 const Option<std::string>& derivationPath) {
    auto& params = currency.bitcoinLikeNetworkParameters.value();
    auto bech32 = Bech32Factory::newBech32Instance(params.Identifier);
    auto decoded = bech32->decode(address);
    return std::make_shared<ledger::core::BitcoinLikeAddress>(currency,
                                                              decoded.second,
                                                              decoded.first,
                                                              derivationPath);
}