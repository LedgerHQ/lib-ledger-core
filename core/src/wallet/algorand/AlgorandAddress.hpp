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

#ifndef LEDGER_CORE_ALGORANDADDRESS_H
#define LEDGER_CORE_ALGORANDADDRESS_H

#include "AlgorandLikeCurrencies.hpp"

#include <wallet/common/AbstractAddress.h>

#include <string>
#include <vector>

namespace ledger {
namespace core {
namespace algorand {

    class Address : public AbstractAddress {

    public:

        // Currency-generic constructors
        Address(const api::Currency& currency, const std::vector<uint8_t> & pubKey);
        Address(const api::Currency& currency, const std::string & address);

        // Convenience constructors using default ALGO currency
        Address();
        explicit Address(const std::string & address);

        std::string toString() override;
        const std::string& toString() const;
        std::vector<uint8_t> getPublicKey() const;

        // Utility methods for easy conversion, could be useful for tests
        static std::string fromPublicKey(const std::vector<uint8_t> & pubKey);
        static std::vector<uint8_t> toPublicKey(const std::string & address);

        static std::shared_ptr<ledger::core::AbstractAddress>
        parse(const std::string& address, const api::Currency& currency);

    private:

        std::string _address;

    };


    bool operator==(const Address& first, const Address& second);
    bool operator!=(const Address& first, const Address& second);

} // namespace algorand
} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_ALGORANDADDRESS_H

