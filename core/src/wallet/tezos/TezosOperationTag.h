/*
 *
 * TezosOperationTag
 *
 * Created by El Khalil Bellakrid on 16/05/2019.
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


#ifndef LEDGER_CORE_TEZOSOPERATIONTAG_H
#define LEDGER_CORE_TEZOSOPERATIONTAG_H

namespace ledger {
    namespace core {
        enum TezosOperationTag {
            OPERATION_TAG_NONE = -1,
            OPERATION_TAG_GENERIC = 3,
            OPERATION_TAG_PROPOSAL = 5,
            OPERATION_TAG_BALLOT = 6,
            OPERATION_TAG_REVEAL = 7,
            OPERATION_TAG_TRANSACTION = 8,
            OPERATION_TAG_ORIGINATION = 9,
            OPERATION_TAG_DELEGATION = 10,
        };
        namespace XTZOperationTag {
            static TezosOperationTag from_string(const std::string &tag) {
                if (tag == "OPERATION_TAG_GENERIC") return TezosOperationTag::OPERATION_TAG_GENERIC;
                else if (tag == "OPERATION_TAG_PROPOSAL") return TezosOperationTag::OPERATION_TAG_PROPOSAL;
                else if (tag == "OPERATION_TAG_BALLOT") return TezosOperationTag::OPERATION_TAG_BALLOT;
                else if (tag == "OPERATION_TAG_REVEAL") return TezosOperationTag::OPERATION_TAG_REVEAL;
                else if (tag == "OPERATION_TAG_TRANSACTION") return TezosOperationTag::OPERATION_TAG_TRANSACTION;
                else if (tag == "OPERATION_TAG_ORIGINATION") return TezosOperationTag::OPERATION_TAG_ORIGINATION;
                else if (tag == "OPERATION_TAG_DELEGATION") return TezosOperationTag::OPERATION_TAG_DELEGATION;
                else return TezosOperationTag::OPERATION_TAG_NONE;
            }

            static std::string to_string(TezosOperationTag tag) {
                switch (tag) {
                    case TezosOperationTag::OPERATION_TAG_GENERIC:
                        return "OPERATION_TAG_GENERIC";
                    case TezosOperationTag::OPERATION_TAG_PROPOSAL:
                        return "OPERATION_TAG_PROPOSAL";
                    case TezosOperationTag::OPERATION_TAG_BALLOT:
                        return "OPERATION_TAG_BALLOT";
                    case TezosOperationTag::OPERATION_TAG_REVEAL:
                        return "OPERATION_TAG_REVEAL";
                    case TezosOperationTag::OPERATION_TAG_TRANSACTION:
                        return "OPERATION_TAG_TRANSACTION";
                    case TezosOperationTag::OPERATION_TAG_ORIGINATION:
                        return "OPERATION_TAG_ORIGINATION";
                    case TezosOperationTag::OPERATION_TAG_DELEGATION:
                        return "OPERATION_TAG_DELEGATION";
                    default:
                        return "OPERATION_TAG_NONE";
                }
            }
        }
    }
}
#endif //LEDGER_CORE_TEZOSOPERATIONTAG_H
