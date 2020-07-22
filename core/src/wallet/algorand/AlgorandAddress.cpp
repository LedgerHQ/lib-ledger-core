/*
 * AlgorandAddress
 *
 * Created by Hakim Aammar on 20/04/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include "AlgorandAddress.hpp"
#include <api/AlgorandAddress.hpp>

#include <collections/vector.hpp>
#include <crypto/sha512_256.h>
#include <math/BaseConverter.hpp>

namespace ledger {
namespace core {

namespace api {

    std::string AlgorandAddress::fromPublicKey(const std::vector<uint8_t> & pubKey) {
        return algorand::Address::fromPublicKey(pubKey);
    }

    std::vector<uint8_t> AlgorandAddress::toPublicKey(const std::string & address) {
        return algorand::Address::toPublicKey(address);
    }

} // namespace api

namespace algorand {

    Address::Address(const api::Currency& currency, const std::vector<uint8_t> & pubKey) :
        ledger::core::AbstractAddress(currency, optional<std::string>("")),
        _address(api::AlgorandAddress::fromPublicKey(pubKey))
    {}

    Address::Address(const api::Currency& currency, const std::string & address) :
        ledger::core::AbstractAddress(currency, optional<std::string>("")),
        _address(address)
    {}

    std::string Address::toString() {
        return _address;
    }

    const std::string& Address::toString() const {
        return _address;
    }

    std::vector<uint8_t> Address::getPublicKey() const {
        return api::AlgorandAddress::toPublicKey(_address);
    }

    std::shared_ptr<ledger::core::AbstractAddress>
    Address::parse(const std::string& address, const api::Currency& currency)
    {
        if (fromPublicKey(toPublicKey(address)) == address) {
            return std::make_shared<Address>(currency, address);
        }
        return nullptr;
    }

    std::string Address::fromPublicKey(const std::vector<uint8_t> & pubKey) {
        // 1. pubkey --> pubKeyHash
        auto hasher = cppcrypto::sha512(256);
        unsigned char pubKeyHash[PUBKEY_LEN_BYTES];
        hasher.init();
        hasher.update(pubKey.data(), pubKey.size());
        hasher.final(pubKeyHash); // Now hash contains the hash
        // 2. 4 last bytes of pubKeyHash
        const std::vector<uint8_t> pubKeyHashChecksum(pubKeyHash + PUBKEY_LEN_BYTES - CHECKSUM_LEN_BYTES, pubKeyHash + PUBKEY_LEN_BYTES);
        // 3. pubkey + 4 last bytes of pubKeyHash
        const std::vector<uint8_t> addressBytes = vector::concat<uint8_t>(pubKey, pubKeyHashChecksum);
        // 4. Encode to Base32
        return BaseConverter::encode(addressBytes, BaseConverter::BASE32_RFC4648_NO_PADDING);
    }

    std::vector<uint8_t> Address::toPublicKey(const std::string & address) {
        std::vector<uint8_t> decoded;
        decoded.reserve(PUBKEY_LEN_BYTES + CHECKSUM_LEN_BYTES);
        // 1. Decode from Base32
        BaseConverter::decode(address, BaseConverter::BASE32_RFC4648_NO_PADDING, decoded);
        // 2. Strip last 4 bytes to keep only the public key
        decoded.resize(PUBKEY_LEN_BYTES);
        return decoded;
    }

} // namespace algorand

} // namespace core
} // namespace ledger


