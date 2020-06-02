/*
 *
 * CosmosLikeExtendedPublicKey
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

#include <bytes/BytesReader.h>
#include <bytes/BytesWriter.h>
#include <collections/vector.hpp>
#include <cosmos/CosmosLikeAddress.hpp>
#include <cosmos/CosmosLikeExtendedPublicKey.hpp>
#include <cosmos/bech32/CosmosBech32.hpp>
#include <crypto/HASH160.hpp>
#include <crypto/SHA256.hpp>
#include <math/Base58.hpp>

namespace ledger {
namespace core {
CosmosLikeExtendedPublicKey::CosmosLikeExtendedPublicKey(
    const api::Currency &params,
    const DeterministicPublicKey &key,
    api::CosmosCurve curve,
    api::CosmosBech32Type type,
    const DerivationPath &path) :
    _currency(params),
    _key(key),
    _curve(curve),
    _type(type),
    _path(path)
{
}

std::shared_ptr<api::CosmosLikeAddress> CosmosLikeExtendedPublicKey::derive(const std::string &path)
{
    DerivationPath p(path);
    auto key = _derive(0, p.toVector(), _key);
    auto addressType = _type == api::CosmosBech32Type::PUBLIC_KEY
                           ? api::CosmosBech32Type::ADDRESS
                           : api::CosmosBech32Type::ADDRESS_VAL;
    return std::make_shared<CosmosLikeAddress>(
        _currency,
        key.getPublicKeyHash160(),
        std::vector<uint8_t>(),
        addressType,
        optional<std::string>((_path + p).toString()));
}

std::shared_ptr<CosmosLikeExtendedPublicKey> CosmosLikeExtendedPublicKey::derive(
    const DerivationPath &path)
{
    auto key = _derive(0, path.toVector(), _key);
    return std::make_shared<CosmosLikeExtendedPublicKey>(
        _currency, key, _curve, _type, _path + path);
}

std::vector<uint8_t> CosmosLikeExtendedPublicKey::derivePublicKey(const std::string &path)
{
    return CosmosExtendedPublicKey::derivePublicKey(path);
}

std::vector<uint8_t> CosmosLikeExtendedPublicKey::deriveHash160(const std::string &path)
{
    return CosmosExtendedPublicKey::deriveHash160(path);
}

std::string CosmosLikeExtendedPublicKey::toBase58()
{
    return CosmosExtendedPublicKey::toBase58();
}

std::string CosmosLikeExtendedPublicKey::toBech32()
{
    auto const pubKey = getKey().getPublicKey();
    auto const pkBech32 = CosmosBech32(_type);
    return pkBech32.encode(
        pubKey, vector::concat(params().PubKeyPrefix, std::vector<uint8_t>(pubKey.size())));
}

std::string CosmosLikeExtendedPublicKey::getRootPath()
{
    return _path.toString();
}

std::shared_ptr<CosmosLikeExtendedPublicKey> CosmosLikeExtendedPublicKey::fromRaw(
    const api::Currency &currency,
    const optional<std::vector<uint8_t>> &parentPublicKey,
    const std::vector<uint8_t> &publicKey,
    const std::vector<uint8_t> &chainCode,
    const std::string &path,
    api::CosmosCurve curve,
    api::CosmosBech32Type type)
{
    auto const &params = currency.cosmosLikeNetworkParameters.value();
    DeterministicPublicKey k =
        CosmosExtendedPublicKey::fromRaw(currency, params, parentPublicKey, publicKey, {}, path);
    DerivationPath p(path);
    return std::make_shared<CosmosLikeExtendedPublicKey>(currency, k, curve, type, p);
}

std::shared_ptr<CosmosLikeExtendedPublicKey> CosmosLikeExtendedPublicKey::fromBase58(
    const api::Currency &currency,
    const std::string &xpub,
    const Option<std::string> &path,
    api::CosmosBech32Type type)
{
    auto const &params = currency.cosmosLikeNetworkParameters.value();
    DeterministicPublicKey k = CosmosExtendedPublicKey::fromBase58(currency, params, xpub, path);
    return std::make_shared<ledger::core::CosmosLikeExtendedPublicKey>(
        currency, k, api::CosmosCurve::SECP256K1, type, DerivationPath(path.getValueOr("m")));
}

std::shared_ptr<CosmosLikeExtendedPublicKey> CosmosLikeExtendedPublicKey::fromBech32(
    const api::Currency &currency, const std::string &bech32PubKey, const Option<std::string> &path)
{
    auto const &params = currency.cosmosLikeNetworkParameters.value();
    if (bech32PubKey.find(cosmos::getBech32Params(api::CosmosBech32Type::PUBLIC_KEY).hrp) ==
        std::string::npos) {
        throw Exception(
            api::ErrorCode::INVALID_ARGUMENT,
            "Invalid Bech32 public Key: should be prefixed with \"cosmospub\"");
    }
    // From bech32 pubKey to pubKeyHash160
    auto const type =
        bech32PubKey.find(cosmos::getBech32Params(api::CosmosBech32Type::PUBLIC_KEY_VAL).hrp) ==
                std::string::npos
            ? api::CosmosBech32Type::PUBLIC_KEY
            : api::CosmosBech32Type::PUBLIC_KEY_VAL;
    auto const pkBech32 = CosmosBech32(type);
    auto const decodedPk = pkBech32.decode(bech32PubKey);

    // Check version
    if (std::vector<uint8_t>(
            decodedPk.second.begin(), decodedPk.second.begin() + params.PubKeyPrefix.size()) !=
        params.PubKeyPrefix) {
        throw Exception(
            api::ErrorCode::INVALID_ARGUMENT,
            "Invalid Bech32 public Key: wrong public key version");
    }

    // Byte array to encode : <PrefixBytes> <Length> <ByteArray> hence the + 5 e.g. {0xEB, 0x5A,
    // 0xE9, 0x87, 0x21}
    std::vector<uint8_t> secp256k1PubKey(decodedPk.second.begin() + 5, decodedPk.second.end());

    DeterministicPublicKey k(secp256k1PubKey, {}, 0, 0, 0, params.Identifier);
    return std::make_shared<ledger::core::CosmosLikeExtendedPublicKey>(
        currency, k, api::CosmosCurve::SECP256K1, type, DerivationPath(path.getValueOr("m")));
}

}  // namespace core
}  // namespace ledger
