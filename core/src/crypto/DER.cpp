/*
 *
 * DER
 * ledger-core
 *
 * Created by Axel Haustant on 01/10/2020.
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
// #include <algorithm>
#include <bytes/BytesReader.h>
#include <collections/vector.hpp>
#include <utils/Exception.hpp>
#include <collections/vector.hpp>

// #include <utils/VectorUtils.hpp>

#include "DER.hpp"

namespace ledger {
    namespace core {

        static std::vector<uint8_t> _pad(const std::vector<uint8_t> &input, int size) {
            auto output = input;
            while(output.size() < size) {
                output.emplace(output.begin(), 0x00);
            }
            return output;
        }

        std::vector<uint8_t> DER::toBytes() {
            // return vector::concat(pad(r), pad(s));
            // return vector::concat(r, s);
            // auto pad = [] (const std::vector<uint8_t> &input) {
            //     auto output = input;
            //     while(output.size() < 32) {
            //         output.emplace(output.begin(), 0x00);
            //     }
            //     return output;
            // };
            return vector::concat(_pad(r, 32), _pad(s, 32));
        }


        DER DER::fromRaw(const std::vector<uint8_t> &raw) {
            BytesReader reader(raw);
            // DER SEQUENCE prefix
            auto prefix = reader.readNextByte();
            if (prefix != ASN1::SEQUENCE) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "A DER Signature should start with a SEQUENCE byte token");
            }
            // Total payload length
            auto length = reader.readNextVarInt();
            
            // Type of R element, should be an Integer
            if (reader.readNextByte() != ASN1::INTEGER) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "A DER Signature should have an integer as first sequence value");
            }
            // R length
            auto rSize = reader.readNextVarInt();
            if (rSize <= 0) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "A DER Signature should have an integer as first sequence value");
            }
            // Extract R
            auto rIsNegative = reader.peek() == 0x00;
            if (rIsNegative) {
                // DER state that integers should start by a 0-byte
                // and if it's negative, 0x00 should be prepended to comply
                reader.readNextByte();
            }
            auto rSignature = reader.read(rIsNegative ? rSize - 1 : rSize);

            // Type of S element, should be an Integer
            if (reader.readNextByte() != ASN1::INTEGER) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "A DER Signature should have an integer as 2nd sequence value");
            }
            // S length
            auto sSize = reader.readNextVarInt();
            if (sSize <= 0) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "A DER Signature should have an integer as 2nd sequence value");
            }
            // Extract S
            auto sIsNegative = reader.peek() == 0x00;
            if (sIsNegative) {
                reader.readNextByte();
            }
            auto sSignature = reader.read(sIsNegative ? sSize - 1 : sSize);

            // Ensure DER encoded size match read bytes
            // Length = len(rType + rSize + rSig + sType + sSize + sSig)
            auto readSize = 2 /* rType + rSize */ + rSize + 2 /* rType + rSize */ + sSize;
            if (length != readSize) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "Read size mismatch DER announced size");
            }

            return {rSignature, sSignature};
        }

    }
}



// void RippleLikeTransactionApi::setDERSignature(const std::vector<uint8_t> &signature) {
//             BytesReader reader(signature);
//             //DER prefix
//             reader.readNextByte();
//             //Total length
//             reader.readNextVarInt();
//             //Nb of elements for R
//             reader.readNextByte();
//             //R length
//             auto rSize = reader.readNextVarInt();
//             _rSignature = reader.read(rSize);
//             //Nb of elements for S
//             reader.readNextByte();
//             //S length
//             auto sSize = reader.readNextVarInt();
//             _sSignature = reader.read(sSize);
//         }

// void CosmosLikeTransactionApi::setDERSignature(const std::vector<uint8_t> &signature) {
//             BytesReader reader(signature);
//             //DER prefix
//             reader.readNextByte();
//             //Total length
//             reader.readNextVarInt();
//             //Nb of elements for R
//             reader.readNextByte();
//             //R length
//             auto rSize = reader.readNextVarInt();
//             if (rSize > 0 && reader.peek() == 0x00) {
//                 reader.readNextByte();
//                 _rSignature = reader.read(rSize - 1);
//             } else {
//                 _rSignature = reader.read(rSize);
//             }
//             //Nb of elements for S
//             reader.readNextByte();
//             //S length
//             auto sSize = reader.readNextVarInt();
//             if (sSize > 0 && reader.peek() == 0x00) {
//                 reader.readNextByte();
//                 _sSignature = reader.read(sSize - 1);
//             } else {
//                 _sSignature = reader.read(sSize);
//             }
//         }
