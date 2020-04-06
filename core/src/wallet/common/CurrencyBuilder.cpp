/*
 *
 * CurrencyBuilder
 * ledger-core
 *
 * Created by Pierre Pollastri on 12/05/2017.
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
#include <api/Wallet.hpp>
#include <api/WalletType.hpp>
#include "CurrencyBuilder.hpp"

namespace ledger {
    namespace core {

        CurrencyBuilder::CurrencyBuilder(const std::string name) {
            _name = name;
        }

        CurrencyBuilder &CurrencyBuilder::units(std::vector<api::CurrencyUnit> units) {
            _units = units;
            return *this;
        }

        CurrencyBuilder &CurrencyBuilder::forkOfBitcoin(api::BitcoinLikeNetworkParameters params) {
            _type = api::WalletType::BITCOIN;
            _bitcoin = params;
            return *this;
        }

        CurrencyBuilder &CurrencyBuilder::forkOfCosmos(api::CosmosLikeNetworkParameters params) {
            _type = api::WalletType::COSMOS;
            _cosmos = params;
            return *this;
        }


        CurrencyBuilder &CurrencyBuilder::forkOfEthereum(api::EthereumLikeNetworkParameters params) {
            _type = api::WalletType::ETHEREUM;
            _ethereum = params;
            return *this;
        }

        CurrencyBuilder &CurrencyBuilder::forkOfRipple(api::RippleLikeNetworkParameters params) {
            _type = api::WalletType::RIPPLE;
            _ripple = params;
            return *this;
        }

        CurrencyBuilder& CurrencyBuilder::forkOfTezos(api::TezosLikeNetworkParameters params) {
            _type = api::WalletType::TEZOS;
            _tezos = params;
            return *this;
        }

        CurrencyBuilder &CurrencyBuilder::forkOfStellar(const api::StellarLikeNetworkParameters &params) {
            _type = api::WalletType::STELLAR;
            _stellar = params;

            return *this;
        }

        CurrencyBuilder::operator api::Currency() const {
            return api::Currency(_type, _name, _coinType, _paymentUriScheme, _units, _bitcoin.toOptional(), _cosmos.toOptional(), _ethereum.toOptional(), _ripple.toOptional(), _tezos.toOptional(), _stellar.toOptional());
        }

        CurrencyBuilder &CurrencyBuilder::bip44(int coinType) {
            _coinType = coinType;
            return *this;
        }

        CurrencyBuilder &CurrencyBuilder::paymentUri(const std::string &scheme) {
            _paymentUriScheme = scheme;
            return *this;
        }

        CurrencyBuilder &CurrencyBuilder::unit(const std::string &name, int magnitude, const std::string &symbol,
                                               const std::string &code) {
            api::CurrencyUnit u(name, symbol, code, (int32_t) magnitude);
            _units.push_back(u);
            return *this;
        }

        CurrencyBuilder &CurrencyBuilder::unit(const std::string &name, int magnitude, const std::string &code) {
            return unit(name, magnitude, code, code);
        }

        CurrencyBuilder Currency(const std::string& name) {
            return CurrencyBuilder(name);
        }

    }
}
