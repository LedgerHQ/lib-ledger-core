/*
 *
 * SECP256k1Point
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/12/2016.
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
#include "SECP256k1Point.hpp"
#include "../utils/Exception.hpp"

namespace ledger {
    namespace core {


        SECP256k1Point::SECP256k1Point(const std::vector<uint8_t> &p) : SECP256k1Point() {
            BIGNUM* bn = BN_bin2bn(p.data(), p.size(), NULL);
            EC_POINT_bn2point(_group, bn, _point, _ctx);
            BN_clear_free(bn);
        }

        SECP256k1Point SECP256k1Point::operator+(const SECP256k1Point &p) const {
            SECP256k1Point result;
            EC_POINT_add(_group, result._point, _point, p._point, _ctx);
            return result;
        }

        SECP256k1Point::SECP256k1Point() {
            _group = EC_GROUP_new_by_curve_name(NID_secp256k1);
            _point = EC_POINT_new(_group);
            _ctx = BN_CTX_new();
        }

        SECP256k1Point::~SECP256k1Point() {
            EC_POINT_free(_point);
            EC_GROUP_free(_group);
            BN_CTX_free(_ctx);
        }

        SECP256k1Point::SECP256k1Point(const SECP256k1Point &p) : SECP256k1Point() {
           *this = p;
        }

        SECP256k1Point &SECP256k1Point::operator=(const SECP256k1Point &p) {
            EC_POINT_copy(_point, p._point);
            return *this;
        }

        bool SECP256k1Point::isAtInfinity() const {
            return EC_POINT_is_at_infinity(_group, _point) == 1;
        }

        SECP256k1Point SECP256k1Point::generatorMultiply(const std::vector<uint8_t> &n) const {
            SECP256k1Point result;
            BIGNUM* bn = BN_bin2bn(n.data(), n.size(), NULL);

            int flag = EC_POINT_mul(_group, result._point, bn, _point, BN_value_one(), _ctx);
            BN_clear_free(bn);

            if (flag == 0) throw Exception(api::ErrorCode::RUNTIME_ERROR, "SECP256k1Point SECP256k1Point::generatorMultiply(const std::vector<uint8_t> &n) failed");
            return result;
        }

        std::vector<uint8_t> SECP256k1Point::toByteArray(bool compressed) const {
            BIGNUM* result = BN_new();
            auto conversion = compressed ? POINT_CONVERSION_COMPRESSED : POINT_CONVERSION_UNCOMPRESSED;
            EC_POINT_point2bn(_group, _point, conversion, result, _ctx);
            auto size = BN_num_bytes(result);
            std::vector<uint8_t> bytes;
            bytes.resize(size);
            BN_bn2bin(result, bytes.data());
            return bytes;

        }

    }
}