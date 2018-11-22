/*
 *
 * CommonBitcoinLikeKeychain
 * ledger-core
 *
 * Created by El Khalil Bellakrid on 30/04/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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

#include "BitcoinLikeKeychain.hpp"
#include <set>
#include "../../../collections/DynamicObject.hpp"
#include <bitcoin/BitcoinLikeAddress.hpp>

namespace ledger {
    namespace core {

        struct KeychainPersistentState {
            uint32_t maxConsecutiveChangeIndex;
            uint32_t maxConsecutiveReceiveIndex;
            std::set<uint32_t> nonConsecutiveChangeIndexes;
            std::set<uint32_t> nonConsecutiveReceiveIndexes;
            bool empty;

            template <class Archive>
            void serialize(Archive& archive, std::uint32_t const version) {
                if (version == 0) {
                    archive(
                            maxConsecutiveChangeIndex,
                            maxConsecutiveReceiveIndex,
                            nonConsecutiveChangeIndexes,
                            nonConsecutiveReceiveIndexes,
                            empty
                    );
                }
            }
        };

        class CommonBitcoinLikeKeychains : public BitcoinLikeKeychain {
        public:
            
            CommonBitcoinLikeKeychains(const std::shared_ptr<api::DynamicObject> &configuration,
                                     const api::Currency &params, int account,
                                     const std::shared_ptr<api::BitcoinLikeExtendedPublicKey> &xpub,
                                     const std::shared_ptr<Preferences> &preferences);

            bool markPathAsUsed(const DerivationPath &path) override;

            BitcoinLikeKeychain::Address getFreshAddress(KeyPurpose purpose) override;
            std::vector<BitcoinLikeKeychain::Address> getAllObservableAddresses(uint32_t from, uint32_t to) override;
            std::vector<BitcoinLikeKeychain::Address> getFreshAddresses(KeyPurpose purpose, size_t n) override;
            Option<KeyPurpose> getAddressPurpose(const std::string &address) const override;
            Option<std::string> getAddressDerivationPath(const std::string &address) const override;
            std::vector<BitcoinLikeKeychain::Address> getAllObservableAddresses(KeyPurpose purpose, uint32_t from, uint32_t to) override;
            bool isEmpty() const override;
            std::shared_ptr<api::BitcoinLikeExtendedPublicKey> getExtendedPublicKey() const;
            std::string getRestoreKey() const override;

            bool contains(const std::string &address) const override;

            int32_t getObservableRangeSize() const override;

            Option<std::vector<uint8_t>> getPublicKey(const std::string &address) const override;

        protected:

            std::shared_ptr<api::BitcoinLikeExtendedPublicKey> _internalNodeXpub;
            std::shared_ptr<api::BitcoinLikeExtendedPublicKey> _publicNodeXpub;
            uint32_t _observableRange;
        private:
            virtual BitcoinLikeKeychain::Address derive(KeyPurpose purpose, off_t index) = 0;
            void saveState();
            KeychainPersistentState _state;
            std::shared_ptr<api::BitcoinLikeExtendedPublicKey> _xpub;
        };
    }
}