/*
 *
 * CosmosLikeKeychain
 *
 * Created by El Khalil Bellakrid on 14/06/2019.
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

#include <api/Currency.hpp>
#include <cosmos/bech32/CosmosBech32.hpp>
#include <crypto/HASH160.hpp>
#include <wallet/cosmos/keychains/CosmosLikeKeychain.hpp>

namespace ledger {
namespace core {
static HashAlgorithm COSMOS_HASH_ALGO("cosmos");

CosmosLikeKeychain::CosmosLikeKeychain(
    const std::vector<uint8_t> &pubKey, const DerivationPath &path, const api::Currency &currency)
{
    _pubKey = pubKey;
    std::vector<uint8_t> payload{0xEB, 0x5A, 0xE9, 0x87, (uint8_t)_pubKey.size()};
    payload.insert(payload.end(), _pubKey.begin(), _pubKey.end());
    auto hash160 = HASH160::hash(_pubKey, COSMOS_HASH_ALGO);
    _address = std::make_shared<CosmosLikeAddress>(
        currency,
        hash160,
        std::vector<uint8_t>(),
        api::CosmosBech32Type::ADDRESS,
        Option<std::string>(""));
}

CosmosLikeKeychain::Address CosmosLikeKeychain::getAddress() const
{
    return _address;
}

bool CosmosLikeKeychain::contains(const std::string &address) const
{
    return _address->toBech32() == address || _address->toString() == address;
}

std::string CosmosLikeKeychain::getRestoreKey() const
{
    std::vector<uint8_t> payload{0xEB, 0x5A, 0xE9, 0x87, (uint8_t)_pubKey.size()};
    payload.insert(payload.end(), _pubKey.begin(), _pubKey.end());
    return CosmosBech32(api::CosmosBech32Type::PUBLIC_KEY).encode(payload, {});
}

const std::vector<uint8_t> &CosmosLikeKeychain::getPublicKey() const
{
    return _pubKey;
}

std::vector<CosmosLikeKeychain::Address> CosmosLikeKeychain::getAllObservableAddresses(
    uint32_t from, uint32_t to)
{
    return {_address};
}

std::shared_ptr<CosmosLikeKeychain> CosmosLikeKeychain::restore(
    const DerivationPath &path, const api::Currency &currency, const std::string &restoreKey)
{
    auto p = CosmosBech32(api::CosmosBech32Type::PUBLIC_KEY).decode(restoreKey);
    std::vector<uint8_t> pubKey(std::get<1>(p).begin() + 5, std::get<1>(p).end());
    return std::make_shared<CosmosLikeKeychain>(pubKey, path, currency);
}
}  // namespace core
}  // namespace ledger
