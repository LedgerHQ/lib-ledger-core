/*
 *
 * p2wpkh_keychain_test
 *
 * Created by El Khalil Bellakrid on 24/02/2019.
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
#include <gtest/gtest.h>
#include <src/wallet/bitcoin/keychains/P2WPKHBitcoinLikeKeychain.hpp>
#include "keychain_test_helper.h"
#include "../BaseFixture.h"

using namespace std;

class BitcoinP2WPKHKeychains : public KeychainFixture<P2WPKHBitcoinLikeKeychain> {
};

TEST_F(BitcoinP2WPKHKeychains, tBTCKeychainDerivation) {
    testKeychain(BTC_TESTNET_DATA, [] (P2WPKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBech32(), "tb1qunawpra24prfc46klknlhl0ydy32feajmwpg84");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBech32(), "tb1qm2grk5v7jy42vtnjccx8dwthngslq650n5hm23");
    });
}

TEST_F(BitcoinP2WPKHKeychains, BTCKeychainDerivation) {
    testKeychain(BTC_DATA, [] (P2WPKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBech32(), "bc1q9sz3mlk5t9cm5vz88hjtfetj0z7e7qq7cq472f");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBech32(), "bc1qrkt83qrp40p6hjumpxj803mqg8p708jt8ynx0y");
    });
}

TEST_F(BitcoinP2WPKHKeychains, BCHKeychainDerivation) {
    testKeychain(BCH_DATA, [] (P2WPKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBech32(), "bitcoincash:qpenyye7dhp9wgugtsh9t3ukdvrnpwyvqyafwfxe0w");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBech32(), "bitcoincash:qzf6rezvt9agmmwnca4ykj74kppr4dx2hvm6f8kzqr");
    });
}