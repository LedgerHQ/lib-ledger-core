/*
 *
 * StellarModelUtils.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 05/07/2020.
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

#include "StellarModelUtils.hpp"
#include <utils/Exception.hpp>

namespace ledger { namespace core { namespace stellar { namespace xdr {

    const std::list<xdr::Operation>& getOperations(const xdr::TransactionEnvelope& envelope) {
        switch (envelope.type) {
            case EnvelopeType::ENVELOPE_TYPE_TX_V0:
                return boost::get<TransactionV0Envelope>(envelope.content).tx.operations;
            case EnvelopeType::ENVELOPE_TYPE_TX:
                return boost::get<TransactionV1Envelope>(envelope.content).tx.operations;
            case EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP:
                return boost::get<FeeBumpTransactionEnvelope>(envelope.content).tx.operations;
            default:
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Envelope type {} is not supported",
                        static_cast<int>(envelope.type));
        }
    }

    TransactionEnvelope wrap(const xdr::TransactionV0Envelope& envelope) {
        TransactionEnvelope env;
        env.type = EnvelopeType::ENVELOPE_TYPE_TX_V0;
        env.content = envelope;
        return env;
    }

    TransactionEnvelope wrap(const xdr::TransactionV1Envelope& envelope) {
        TransactionEnvelope env;
        env.type = EnvelopeType::ENVELOPE_TYPE_TX;
        env.content = envelope;
        return env;
    }

} } } }