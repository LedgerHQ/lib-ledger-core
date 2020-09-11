/*
 *
 * TezosKey
 * ledger-core
 *
 * Created by Axel Haustant on 10/09/2020.
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
#ifndef LEDGER_CORE_TEZOSKEY_H
#define LEDGER_CORE_TEZOSKEY_H

#include "../utils/optional.hpp"
#include <vector>
#include <api/TezosCurve.hpp>

namespace ledger {
    namespace core {
        /**
         * This namespace holds Tezos Key management constants and helpers
         */
        namespace TezosKeyType {

            /**
             * Specify a Tezos Key type encoding
             */
            struct Encoding {
                std::string prefix;
                std::vector<uint8_t> version;
                api::TezosCurve curve;
                int length;
                int b58_length;
                std::string label;
            };

            std::ostream &operator<<(std::ostream &os, const Encoding &e);

            const Encoding EDPK{"edpk", {13, 15, 37, 217}, api::TezosCurve::ED25519, 32, 54,
                                "ED25519 public key"};

            const Encoding SPPK{"sppk", {3, 254, 226, 86}, api::TezosCurve::SECP256K1, 33, 55,
                                "SECP256K1 public key"};

            const Encoding P2PK{"p2pk", {3, 178, 139, 127}, api::TezosCurve::P256, 33, 55,
                                "P256 public key"};

            
            const Encoding TZ1{"tz1", {6, 161, 159}, api::TezosCurve::ED25519, 20, 36,
                               "ED25519 public key hash"};

            const Encoding TZ2{"tz2", {6, 161, 161}, api::TezosCurve::SECP256K1, 20, 36,
                               "SECP256k1 public key hash"};
            
            const Encoding TZ3{"tz3", {6, 161, 164}, api::TezosCurve::P256, 20, 36,
                               "P256 public key hash"};
            
            const Encoding KT1{"KT1", {2, 90, 121}, api::TezosCurve::ED25519, 20, 36,
                               "Originated address"};

            /**
             * Get the key encoding definition for a given version
             */
            std::experimental::optional<Encoding> fromVersion(std::vector<uint8_t> version);

            /**
             * Get the key encoding definition for a given base58 key
             */
            std::experimental::optional<Encoding> fromBase58(std::string key);
        }
    }
}
#endif //LEDGER_CORE_TEZOSKEY_H
