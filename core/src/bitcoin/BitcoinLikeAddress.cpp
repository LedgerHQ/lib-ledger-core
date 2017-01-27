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
#include "../utils/vector.hpp"
#include "../utils/Exception.hpp"

ledger::core::BitcoinLikeAddress::BitcoinLikeAddress(const ledger::core::api::BitcoinLikeNetworkParameters &params,
                                                     const std::vector<uint8_t>& hash160,
                                                     const std::vector<uint8_t>& version,
                                                     ledger::core::optional<std::string> derivationPath) :
        _params(params),
        _version(version),
        _derivationPath(derivationPath),
        _hash160(hash160)
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
    return Base58::encodeWithChecksum(vector::concat(_version, _hash160));
}

std::string ledger::core::BitcoinLikeAddress::toPaymentUri() {
    return _params.PaymentUriScheme + ":" + toBase58();
}

bool ledger::core::BitcoinLikeAddress::isP2SH() {
    return _version == _params.P2SHVersion;
}

bool ledger::core::BitcoinLikeAddress::isP2PKH() {
    return _version == _params.P2PKHVersion;
}

std::experimental::optional<std::string> ledger::core::BitcoinLikeAddress::getDerivationPath() {
    return _derivationPath;
}

std::shared_ptr<ledger::core::api::BitcoinLikeAddress>
        ledger::core::api::BitcoinLikeAddress::fromBase58(const BitcoinLikeNetworkParameters &params,
                                                          const std::string &address) {
    auto decoded = Base58::checkAndDecode(address);
    if (decoded.isFailure()) {
        throw decoded.getFailure();
    }
    auto value = decoded.getValue();
    std::vector<uint8_t> hash160(value.end() - 20, value.end());
    std::vector<uint8_t> version(value.begin(), value.end() - 20);
    if (version != params.P2PKHVersion && version != params.P2SHVersion) {
        throw Exception(ErrorCode::INVALID_VERSION, "Address version doesn't belong to the given network parameters");
    }
    return std::make_shared<ledger::core::BitcoinLikeAddress>(params, hash160, version);
}

bool
ledger::core::api::BitcoinLikeAddress::isAddressValid(const ledger::core::api::BitcoinLikeNetworkParameters &params,
                                                      const std::string &address) {
    return Try<std::shared_ptr<ledger::core::api::BitcoinLikeAddress>>::from([&] () {
        return fromBase58(params, address);
    }).isSuccess();
}
