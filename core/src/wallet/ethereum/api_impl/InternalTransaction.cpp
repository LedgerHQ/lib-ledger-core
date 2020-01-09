/*
 *
 * InternalTransaction
 *
 * Created by El Khalil Bellakrid on 15/07/2019.
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


#include "InternalTransaction.h"
#include <api_impl/BigIntImpl.hpp>

namespace ledger {
    namespace core {
        InternalTransaction::InternalTransaction(const InternalTx &internalTx) : _internalTx(internalTx)
        {}

        std::shared_ptr<api::BigInt> InternalTransaction::getGasLimit() {
            return std::make_shared<api::BigIntImpl>(_internalTx.gasLimit);
        }

        std::shared_ptr<api::BigInt> InternalTransaction::getUsedGas() {
            return std::make_shared<api::BigIntImpl>(_internalTx.gasUsed.getValueOr(BigInt::ZERO));
        }

        std::string InternalTransaction::getSender() {
            return _internalTx.from;
        }

        std::string InternalTransaction::getReceiver() {
            return _internalTx.to;
        }

        std::shared_ptr<api::BigInt> InternalTransaction::getValue() {
            return std::make_shared<api::BigIntImpl>(_internalTx.value);
        }

        std::vector<uint8_t> InternalTransaction::getData() {
            return _internalTx.inputData;
        }

        api::OperationType InternalTransaction::getOperationType() {
            return _internalTx.type;
        }
    }
}
