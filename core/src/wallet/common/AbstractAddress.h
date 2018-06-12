/*
 *
 * AbstractAddress.h
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

#ifndef LEDGER_CORE_ABSTRACTADDRESS_H
#define LEDGER_CORE_ABSTRACTADDRESS_H

#include <api/Address.hpp>
#include <api/Currency.hpp>
#include <utils/Option.hpp>

namespace ledger {
    namespace core {

        class AbstractAddress : public api::Address, public std::enable_shared_from_this<AbstractAddress> {
        public:
            explicit AbstractAddress(const api::Currency& currency, const Option<std::string>& path);
            api::Currency getCurrency() override;

            optional<std::string> getDerivationPath() override;

            std::shared_ptr<api::BitcoinLikeAddress> asBitcoinLikeAddress() override;

            bool isInstanceOfBitcoinLikeAddress() override;

        private:
            api::Currency _currency;
            Option<std::string> _path;

        };
    }
}

#endif //LEDGER_CORE_ABSTRACTADDRESS_H
