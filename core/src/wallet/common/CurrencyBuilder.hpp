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
#ifndef LEDGER_CORE_CURRENCYBUILDER_HPP
#define LEDGER_CORE_CURRENCYBUILDER_HPP

#include <api/Currency.hpp>
#include <utils/Option.hpp>
#include <api/EthereumLikeNetworkParameters.hpp>
#include <api/RippleLikeNetworkParameters.hpp>
#include <api/TezosLikeNetworkParameters.hpp>
#include <api/StellarLikeNetworkParameters.hpp>

namespace ledger {
    namespace core {
        class CurrencyBuilder {
        public:
            CurrencyBuilder(const std::string name);
            CurrencyBuilder& units(std::vector<api::CurrencyUnit> units);
            CurrencyBuilder& bip44(int32_t coinType);
            CurrencyBuilder& paymentUri(const std::string& scheme);
            CurrencyBuilder& forkOfBitcoin(api::BitcoinLikeNetworkParameters params);
            CurrencyBuilder& forkOfEthereum(api::EthereumLikeNetworkParameters params);
            CurrencyBuilder& forkOfRipple(api::RippleLikeNetworkParameters params);
            CurrencyBuilder& forkOfTezos(api::TezosLikeNetworkParameters params);
            CurrencyBuilder& forkOfStellar(const api::StellarLikeNetworkParameters& params);
            CurrencyBuilder& unit(const std::string& name, int magnitude, const std::string& symbol, const std::string& code);
            CurrencyBuilder& unit(const std::string& name, int magnitude, const std::string& code);
            operator api::Currency() const;

        private:
            std::vector<api::CurrencyUnit> _units;
            std::string _name;
            Option<api::BitcoinLikeNetworkParameters> _bitcoin;
            Option<api::EthereumLikeNetworkParameters> _ethereum;
            Option<api::RippleLikeNetworkParameters> _ripple;
            Option<api::TezosLikeNetworkParameters> _tezos;
            Option<api::StellarLikeNetworkParameters> _stellar;
            api::WalletType _type;
            std::string _paymentUriScheme;
            int32_t _coinType;
        };
        CurrencyBuilder Currency(const std::string& name);
    }
}


#endif //LEDGER_CORE_CURRENCYBUILDER_HPP
