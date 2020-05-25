/*
 * AlgorandPayment
 *
 * Created by Rémi Barjon on 04/05/2020.
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

#pragma once

#include "../../AlgorandAddress.hpp"

#include <utils/Option.hpp>

#include <cstdint>

namespace ledger {
namespace core {
namespace algorand {
namespace model {

    class PaymentTxnFields
    {
    public:
        PaymentTxnFields() = default;
        PaymentTxnFields(uint64_t amount,
                         Option<Address> closeAddr,
                         Address receiverAddr)
            : amount(amount)
            , closeAddr(std::move(closeAddr))
            , receiverAddr(std::move(receiverAddr))
        {}

        uint64_t amount;
        Option<Address> closeAddr;
        Address receiverAddr;

        // Additional fields retrieved from the blockchain
        Option<uint64_t> closeAmount;
        Option<uint64_t> closeRewards;
        Option<uint64_t> receiverRewards;

    };

} // namespace model
} // namespace ledger
} // namespace core
} // namespace algorand

