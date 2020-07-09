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

        static const std::size_t PUBKEY_SIZE = 32;
        static const std::size_t CHECKSUM_SIZE = 2;

        StellarLikeAddress::StellarLikeAddress(const std::string &address, const api::Currency &currency,
                                               const Option<std::string> &path) :   AbstractAddress(currency, path),
                                                                                    _address(address) {

        }

        StellarLikeAddress::StellarLikeAddress(const std::vector<uint8_t> &pubKey, const api::Currency &currency,
                                               const Option<uint64_t>& memoId,
                                               const Option<std::string> &path) : AbstractAddress(currency, path),
                                               _address(convertPubkeyToAddress(pubKey, memoId, *currency.stellarLikeNetworkParameters)) {
        }

        std::string StellarLikeAddress::toString() {
            return _address;
        }

        std::vector<uint8_t> StellarLikeAddress::toPublicKey() const {
            const auto& params = getCurrency().stellarLikeNetworkParameters.value();
            std::vector<uint8_t> bytes;
            BaseConverter::decode(_address, BaseConverter::BASE32_RFC4648_NO_PADDING, bytes);
            BytesReader reader(bytes);
            auto version = reader.read(params.Version.size());
            if (version == params.MuxedVersion)
                reader.seek(sizeof(uint64_t), BytesReader::Seek::CUR);
            return reader.read(32);
        }

        std::shared_ptr<StellarLikeAddress>
        StellarLikeAddress::parse(const std::string &address, const api::Currency &currency) {
            if (!isValid(address, currency)) {
                return nullptr;
            }
            return std::make_shared<StellarLikeAddress>(address, currency, Option<std::string>());
        }

        std::string StellarLikeAddress::convertPubkeyToAddress(const std::vector<uint8_t> &pubKey,
                                                               const Option<uint64_t>& memoId,
                                                               const api::StellarLikeNetworkParameters &params) {
            BytesWriter writer;
            if (memoId) {
                writer.writeByteArray(params.MuxedVersion);
                writer.writeBeValue(memoId.getValue());
                writer.writeByteArray(pubKey);
            } else {
                writer.writeByteArray(params.Version);
                writer.writeByteArray(pubKey);
            }
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

        bool StellarLikeAddress::isValid(const std::string &address, const api::Currency& currency) {
            const auto &networkParams = currency.stellarLikeNetworkParameters.value();
            std::vector<uint8_t> bytes;
            try {
                BaseConverter::decode(address, BaseConverter::BASE32_RFC4648_NO_PADDING, bytes);
            } catch (...) {
                return false;
            }
            if (bytes.size() != (networkParams.Version.size() + PUBKEY_SIZE + CHECKSUM_SIZE) ||
                !std::equal(bytes.begin(), bytes.begin() + networkParams.Version.size(),
                            networkParams.Version.begin())) {
                return false;
            }
            BytesReader reader(bytes);
            auto payload = reader.read(networkParams.Version.size() + PUBKEY_SIZE);
            auto checksum = reader.readNextLeUint16();
            auto expectedChecksum = CRC::calculate(payload, CRC::XMODEM);
            return checksum == expectedChecksum;
        }

        std::string StellarLikeAddress::convertXdrAccountToAddress(const stellar::xdr::AccountID &accountId,
                                                                   const api::StellarLikeNetworkParameters &params) {
            return convertPubkeyToAddress(std::vector<uint8_t>(accountId.content.begin(), accountId.content.end()), {}, params);
        }

        std::string StellarLikeAddress::convertMuxedAccountToAddress(const stellar::xdr::MuxedAccount &account,
                                                                     const api::StellarLikeNetworkParameters &params) {
            switch (account.type) {
                case stellar::xdr::CryptoKeyType::KEY_TYPE_ED25519: {
                    const auto& pk = boost::get<stellar::xdr::uint256>(account.content);
                    return convertPubkeyToAddress(std::vector<uint8_t>(pk.begin(), pk.end()), {}, params);
                }
                case stellar::xdr::CryptoKeyType::KEY_TYPE_MUXED_ED25519: {
                   const auto& pk = boost::get<stellar::xdr::med25519>(account.content);
                   return convertPubkeyToAddress(std::vector<uint8_t>(pk.ed25519.begin(), pk.ed25519.end()), {pk.id}, params);
                }
                case stellar::xdr::CryptoKeyType::KEY_TYPE_PRE_AUTH_TX:
                    throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Crypto key of type KEY_TYPE_PRE_AUTH_TX is not an address.");
                case stellar::xdr::CryptoKeyType::KEY_TYPE_HASH_X:
                    throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Crypto key of type KEY_TYPE_HASH_X is not an address.");
            }
        }

        stellar::xdr::MuxedAccount StellarLikeAddress::toXdrMuxedAccount() const {
            const auto& params = getCurrency().stellarLikeNetworkParameters.value();
            std::vector<uint8_t> bytes;
            BaseConverter::decode(_address, BaseConverter::BASE32_RFC4648_NO_PADDING, bytes);
            BytesReader reader(bytes);
            auto version = reader.read(params.Version.size());
            stellar::xdr::MuxedAccount account;
            if (version == params.MuxedVersion) {
                account.type = stellar::xdr::CryptoKeyType::KEY_TYPE_MUXED_ED25519;
                stellar::xdr::med25519 key;
                key.id = reader.readNextBeUlong();
                auto pubkey = reader.read(32);
                std::copy(pubkey.begin(), pubkey.end(), key.ed25519.begin());
                account.content = key;
            } else {
                auto pubkey = reader.read(32);
                account.type = stellar::xdr::CryptoKeyType::KEY_TYPE_ED25519;
                stellar::xdr::uint256 key;
                std::copy(pubkey.begin(), pubkey.end(), key.begin());
                account.content = key;
            }
            return account;
        }

    }
}