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

#include <core/crypto/SECP256k1Point.hpp>
#include <core/debug/Benchmarker.hpp>
#include <core/utils/Exception.hpp>
#include <core/utils/VectorUtils.hpp>
#include <include/secp256k1.h>

namespace ledger {
    namespace core {
        SECP256k1Point::SECP256k1Point(const std::vector<uint8_t> &p) : SECP256k1Point() {
            _pubKey = new secp256k1_pubkey();
            if (secp256k1_ec_pubkey_parse(_context, _pubKey, p.data(), p.size()) == -1)
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Unable to parse secp256k1 point");
        }

        SECP256k1Point SECP256k1Point::operator+(const SECP256k1Point &p) const {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "SECP256k1Point SECP256k1Point::operator+(const SECP256k1Point &p) const");
        }

        SECP256k1Point::SECP256k1Point() {
            _context = secp256k1_context_create(SECP256K1_CONTEXT_SIGN | SECP256K1_CONTEXT_VERIFY);
            _pubKey = nullptr;
        }

        SECP256k1Point::~SECP256k1Point() {
            secp256k1_context_destroy(_context);
            if (_pubKey) {
                delete _pubKey;
            }
        }

        SECP256k1Point::SECP256k1Point(const SECP256k1Point &p) : SECP256k1Point() {
           *this = p;
        }

        SECP256k1Point &SECP256k1Point::operator=(const SECP256k1Point &p) {
            _pubKey = new secp256k1_pubkey();
            ::memcpy(_pubKey, p._pubKey, sizeof(*p._pubKey));
            return *this;
        }

        bool SECP256k1Point::isAtInfinity() const {
            ensurePubkeyIsNotNull();
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "bool SECP256k1Point::isAtInfinity() const");
        }

        SECP256k1Point SECP256k1Point::generatorMultiply(const std::vector<uint8_t> &n) const {
            ensurePubkeyIsNotNull();
            // Pad the number to 32 bytes with 0
            auto num = n;
            VectorUtils::padOnLeft<uint8_t>(num, 0, 32);

            secp256k1_pubkey* pubKey = new secp256k1_pubkey();
            memcpy(pubKey, _pubKey, sizeof(*pubKey));
            std::vector<uint8_t> serializedKey(33);
            auto len = serializedKey.size();
            auto flag = secp256k1_ec_pubkey_tweak_add(_context, pubKey, num.data());
            if (flag == 0) throw Exception(api::ErrorCode::RUNTIME_ERROR, "SECP256k1Point SECP256k1Point::generatorMultiply(const std::vector<uint8_t> &n) failed");
            secp256k1_ec_pubkey_serialize(_context, serializedKey.data(), &len, pubKey, SECP256K1_EC_COMPRESSED);
            return SECP256k1Point(serializedKey);
        }

        std::vector<uint8_t> SECP256k1Point::toByteArray(bool compressed) const {
            ensurePubkeyIsNotNull();
            if (compressed) {
                std::vector<uint8_t> result(33);
                size_t len = 33;
                secp256k1_ec_pubkey_serialize(_context, result.data(), &len, _pubKey, SECP256K1_EC_COMPRESSED);
                return result;
            }
            return std::vector<uint8_t>(_pubKey->data, _pubKey->data + sizeof(_pubKey->data));

        }

        void SECP256k1Point::ensurePubkeyIsNotNull() const {
            if (_pubKey == nullptr)
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Public key is null, cannot do any computation on the point.");
        }
    }
}
