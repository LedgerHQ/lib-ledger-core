/*
 *
 * AbstractAddress.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 14/05/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#include "AbstractAddress.h"
#include <utils/Exception.hpp>
#include <bitcoin/BitcoinLikeAddress.hpp>
#include <ethereum/EthereumLikeAddress.h>
#include <ripple/RippleLikeAddress.h>
#include <tezos/TezosLikeAddress.h>
#include <wallet/stellar/StellarLikeAddress.hpp>

namespace ledger {
    namespace core {

        AbstractAddress::AbstractAddress(const api::Currency &currency, const Option<std::string>& path)
                : _currency(currency), _path(path) {

        }

        api::Currency AbstractAddress::getCurrency() {
            return _currency;
        }

        optional<std::string> AbstractAddress::getDerivationPath() {
            return _path.toOptional();
        }

        std::shared_ptr<api::BitcoinLikeAddress> AbstractAddress::asBitcoinLikeAddress() {
            return std::dynamic_pointer_cast<api::BitcoinLikeAddress>(shared_from_this());
        }

        bool AbstractAddress::isInstanceOfBitcoinLikeAddress() {
            return asBitcoinLikeAddress() != nullptr;
        }

        std::shared_ptr<api::StellarLikeAddress> AbstractAddress::asStellarLikeAddress() {
            return std::dynamic_pointer_cast<api::StellarLikeAddress>(shared_from_this());
        }

        bool AbstractAddress::isInstanceOfStellarLikeAddress() {
            return _currency.walletType == api::WalletType::STELLAR;
        }


        std::shared_ptr<api::Address> api::Address::parse(const std::string &address, const Currency &currency) {
            switch (currency.walletType) {
                case WalletType::BITCOIN:
                    return ledger::core::BitcoinLikeAddress::parse(address, currency);
                case WalletType::ETHEREUM:
                    return ledger::core::EthereumLikeAddress::parse(address, currency);
                case WalletType::STELLAR:
                    return ledger::core::StellarLikeAddress::parse(address, currency);
                case WalletType::RIPPLE:
                    return ledger::core::RippleLikeAddress::parse(address, currency);
                case WalletType::TEZOS:
                    return ledger::core::TezosLikeAddress::parse(address, currency);
                case WalletType::MONERO:
                    throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "MONERO address parser is not implemented yet");
            }
        }

        bool api::Address::isValid(const std::string &address, const Currency &currency) {
            return parse(address, currency) != nullptr;
        }

    }
}
