/*
 *
 * StellarLikeAddress.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/02/2019.
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

#include "StellarLikeAddress.hpp"
#include <bytes/BytesWriter.h>
#include <crypto/CRC.hpp>
#include <math/BaseConverter.hpp>
#include <bytes/BytesReader.h>

namespace ledger {
    namespace core {

        StellarLikeAddress::StellarLikeAddress(const std::string &address, const api::Currency &currency,
                                               const Option<std::string> &path) :   AbstractAddress(currency, path),
                                                                                    _address(address) {
        }

        StellarLikeAddress::StellarLikeAddress(const std::vector<uint8_t> &pubKey, const api::Currency &currency,
                                               const Option<std::string> &path) : AbstractAddress(currency, path) {
            _address = convertPubkeyToAddress(pubKey, *currency.stellarLikeNetworkParameters);
        }

        std::string StellarLikeAddress::toString() {
            return _address;
        }

        std::vector<uint8_t> StellarLikeAddress::toPublicKey() const {
            std::vector<uint8_t> bytes;
            BaseConverter::decode(_address, BaseConverter::BASE32_RFC4648_NO_PADDING, bytes);
            BytesReader reader(bytes);
            reader.seek(getCurrency().stellarLikeNetworkParameters.value().Version.size(), BytesReader::Seek::CUR);
            return reader.read(32);
        }

        std::shared_ptr<StellarLikeAddress>
        StellarLikeAddress::parse(const std::string &address, const api::Currency &currency) {
            return std::make_shared<StellarLikeAddress>(address, currency, Option<std::string>());
        }

        std::string StellarLikeAddress::convertPubkeyToAddress(const std::vector<uint8_t> &pubKey,
                                                               const api::StellarLikeNetworkParameters &params) {
            BytesWriter writer;
            writer.writeByteArray(params.Version);
            writer.writeByteArray(pubKey);
            auto payload = writer.toByteArray();
            auto checksum = CRC::calculate(payload, CRC::XMODEM);
            writer.writeLeValue(checksum);
            return BaseConverter::encode(writer.toByteArray(), BaseConverter::BASE32_RFC4648_NO_PADDING);
        }

        stellar::xdr::PublicKey StellarLikeAddress::toXdrPublicKey() const {
            stellar::xdr::PublicKey pk;
            pk.type = stellar::xdr::PublicKeyType::PUBLIC_KEY_TYPE_ED25519;
            const auto pubkey = toPublicKey();
            if (pubkey.size() != pk.content.max_size()) {
                throw make_exception(api::ErrorCode::ILLEGAL_STATE, "Pub key should be {} bytes long (got {})", pk.content.max_size(), pubkey.size());
            }
            std::copy(pubkey.begin(), pubkey.end(), pk.content.begin());
            return pk;
        }

    }
}