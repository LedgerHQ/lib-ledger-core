/*
 *
 * DeterministicPublicKey
 * ledger-core
 *
 * Created by Pierre Pollastri on 14/12/2016.
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
#ifndef LEDGER_CORE_DETERMINISTICPUBLICKEY_HPP
#define LEDGER_CORE_DETERMINISTICPUBLICKEY_HPP

#include <vector>

namespace ledger {
    namespace core {
        class DeterministicPublicKey {
        public:
            DeterministicPublicKey( const std::vector<uint8_t>& publicKey,
                                    const std::vector<uint8_t>& chainCode,
                                    uint32_t childNum,
                                    uint32_t depth,
                                    uint32_t parentFingerprint,
                                    const std::string &networkIdentifier
                                    );
            DeterministicPublicKey(const DeterministicPublicKey& key);
            uint32_t getFingerprint() const;
            DeterministicPublicKey derive(uint32_t childIndex) const;

            const std::vector<uint8_t>& getPublicKey() const;
            std::vector<uint8_t> getUncompressedPublicKey() const;
            std::vector<uint8_t> getPublicKeyHash160() const;
            std::vector<uint8_t> getPublicKeyKeccak256() const;
            std::vector<uint8_t> getPublicKeyBlake2b(bool isED25519 = false) const;
            std::vector<uint8_t> toByteArray(const std::vector<uint8_t>& version = {}) const;
        public:


        private:
            const std::vector<uint8_t> _key;
            const std::vector<uint8_t> _chainCode;
            const uint32_t _childNum;
            const uint32_t _depth;
            const uint32_t _parentFingerprint;
            const std::string _networkIdentifier;
        };
    }
}


#endif //LEDGER_CORE_DETERMINISTICPUBLICKEY_HPP
