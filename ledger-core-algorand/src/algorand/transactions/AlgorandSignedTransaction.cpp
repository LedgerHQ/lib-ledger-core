/*
 * AlgorandSignedTransaction
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

#include <algorand/model/transactions/AlgorandSignedTransaction.hpp>

#include "util/TransactionAdaptor.hpp"

#include <sstream>

namespace ledger {
namespace core {
namespace algorand {
namespace model {

    SignedTransaction::SignedTransaction(Option<std::vector<uint8_t>> sig,
                                         Transaction txn)
        : sig(std::move(sig))
        , txn(std::move(txn))
    {}

    SignedTransaction::SignedTransaction(std::vector<uint8_t> sig,
                                         Transaction txn)
        : SignedTransaction(Option<std::vector<uint8_t>>(std::move(sig)),
                            std::move(txn))
    {}

    SignedTransaction::SignedTransaction(Transaction txn)
        : SignedTransaction(Option<std::vector<uint8_t>>(),
                            std::move(txn))
    {}

    const Option<std::vector<uint8_t>>& SignedTransaction::getSig() const
    {
        return sig;
    }

    const Transaction& SignedTransaction::getTxn() const
    {
        return txn;
    }

    std::vector<uint8_t> SignedTransaction::serialize() const
    {
        std::stringstream ss;
        msgpack::pack(ss, *this);
        const auto str = ss.str();

        return { std::begin(str), std::end(str) };
    }

    void SignedTransaction::setSignature(const std::vector<uint8_t>& signature)
    {
        sig = signature;
    }

} // namespace model
} // namespace ledger
} // namespace core
} // namespace algorand

