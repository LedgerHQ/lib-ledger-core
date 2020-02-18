/*
 *
 * p2pkh_keychain_test
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/01/2017.
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

#include <gtest/gtest.h>
#include <src/wallet/bitcoin/keychains/P2PKHBitcoinLikeKeychain.hpp>
#include "keychain_test_helper.h"

class BitcoinKeychains : public KeychainFixture<P2PKHBitcoinLikeKeychain> {

};

TEST_F(BitcoinKeychains, KeychainDerivation) {
    testKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv");
    });
}

TEST_F(BitcoinKeychains, BCHKeychainDerivation) {
    testKeychain(BCH_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "1BW6hLyZKY9AnUwrU9CwHQJ2c79ho49q4f");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "1ETcGdzh7XGgP2HRUqkYuYEMifihTt8ZiF");
    });
}

TEST_F(BitcoinKeychains, BTGKeychainDerivation) {
    testKeychain(BTG_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "GeB2eVacdg6T5U4beqXZgL6vPPPspgqBic");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "GWzECqesKKgjH5RXVt9Na5MYbH2BmeKveF");
    });
}

TEST_F(BitcoinKeychains, ZCashKeychainDerivation) {
    testKeychain(ZCASH_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "t1Y1C1GiyhffDV3AMCAcBdYT2H2J9ng2eoY");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "t1eHTEWDziaKtBqaZrSqAPgvm6tBGZezgBR");
    });
}

TEST_F(BitcoinKeychains, ZenCashKeychainDerivation) {
    testKeychain(ZENCASH_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "znSHF3jfG9REjJWjFoRrr1WWpNFj4NWeyCW");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "zno29sgBi7CdERRqqjs39mHTdkRHhuodZDa");
    });
}

TEST_F(BitcoinKeychains, LTCKeychainDerivation) {
    testKeychain(LTC_DATA_LEGACY, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "LSPT2mGEFnHg9wUZ1dqzFbjxfAtgYizGgT");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "LhUaNteNLDsuM3BiTGJusHkNsLWkLKpgJ5");
    });
}

TEST_F(BitcoinKeychains, PeerCoinKeychainDerivation) {
    testKeychain(PEERCOIN_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "P9FBRWVkjHwjituqUzqxDrRSuhv7uoaP6o");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "PFBBNu1g73ppb8pf7fFJjrhmAWkHdnkxda");
    });
}

TEST_F(BitcoinKeychains, DigiByteKeychainDerivation) {
    testKeychain(DIGIBYTE_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "DH9fuU2RNiEbwAcgKsh6ZyzRnBfuafsn2y");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "DARxx416tf1afnguTepDJ2nMoUkaDKp5kr");
    });
}

TEST_F(BitcoinKeychains, HcashKeychainDerivation) {
    testKeychain(HCASH_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "HGp3XGeaDhj54y2RTCjHLEgfK2X5VhXNyM");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "HQf6cxcoCWQzECNuAvQaHAofxjGA7iuAws");
    });
}

TEST_F(BitcoinKeychains, QtumKeychainDerivation) {
    testKeychain(QTUM_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "QSZ4LacFeSfkm9xfj59Aurg6fjqqYF6nVy");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "QNQYhA5BFo9wwoxRUNeS22cqVoSXrgu6Eu");
    });
}

TEST_F(BitcoinKeychains, StealthCoinKeychainDerivation) {
    testKeychain(XST_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "SERDi3boPHrCAJTGVe8FmTZWRQEW96AkRT");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "SEotdmKruzfxokECTdN9KHpXKcxxHvUaho");
    });
}

TEST_F(BitcoinKeychains, VertCoinKeychainDerivation) {
    testKeychain(VTC_DATA_LEGACY, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "Vf9yiVFBq5nR6xaRVCgQ1yVX4yi6Xd8Mvt");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "VaYyKnW5RFAPZKwrtEBEvqVY2DJKwcHNhQ");
    });
}

TEST_F(BitcoinKeychains, ViaCoinKeychainDerivation) {
    testKeychain(VIA_DATA_LEGACY, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "Vd6MV3A4xLjEueenjDU7cf9ZWcWcSc9yLK");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "Vu2fAKQaxAjKH3m3wZUdtDeNKiwaUga1ue");
    });
}

TEST_F(BitcoinKeychains, DashKeychainDerivation) {
    testKeychain(DASH_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "XwyjpfwcmJjLptD1akqM3yAxcVmkj8H938");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "XsBhSXm3uYg9zSXbomX6GRt39XUF1mDNhx");
    });
}

TEST_F(BitcoinKeychains, DogeKeychainDerivation) {
    testKeychain(DOGE_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "D5LgLdPWThkensZXSX81UqexGcN6f4nu7Z");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "DBjiJaYqzE18U1Akz6gzN5Q3VEdMRshkmf");
    });
}

TEST_F(BitcoinKeychains, StratisKeychainDerivation) {
    testKeychain(STRATIS_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "SSiyTLEviUkD9gA69qrABhVTAa17uFhaTm");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "SUWWpvGrDjwtRhxiaPXEVa3Xqa9NX38Feq");
    });
}

TEST_F(BitcoinKeychains, KomodoKeychainDerivation) {
    testKeychain(KOMODO_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "RUk15GT5joraNpEpdyvaS7iCZaX4LppVSS");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "RAxANs9rTjhmEK5Kw6QAqMuy1mt6b2gDtf");
    });
}

TEST_F(BitcoinKeychains, PosWalletKeychainDerivation) {
    testKeychain(POSWALLET_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "PMAJnswyesefBLuWVzzqeSGh1YNYbSjBun");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "PDT6swtwrZw2qtHVmYLLZrpUbsVTDZ33qP");
    });
}

TEST_F(BitcoinKeychains, PivxKeychainDerivation) {
    testKeychain(PIVX_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "DA7H5Zwq1S9cyspqD4qT43bEcQNEf5swZQ");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "DPbkYYCtaH2zfgjQQhHKRy4dq1RKHwBFBD");
    });
}

TEST_F(BitcoinKeychains, ClubcoinKeychainDerivation) {
    testKeychain(CLUBCOIN_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "CGnEJw3yCK5MgmA6ivbLBuk8KsWpgtFRXu");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "CSQuTH3kDbnGAG6LmVaLje7kySj6pueego");
    });
}

TEST_F(BitcoinKeychains, DecredKeychainDerivation) {
    testKeychain(DECRED_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "Dso5BhFYjnymYwAzsCDEUENmmh7Y9TA4FM7");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "DsSFcrefQbbo8i7u9dQZrtzc6EW6nvVMKZR");
    });
}

TEST_F(BitcoinKeychains, SimpleUsedReceiveAddresses) {
    testKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(addresses.size() < 50000);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR");
        EXPECT_TRUE(keychain.markAsUsed("151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR"));
        EXPECT_FALSE(keychain.markAsUsed("151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "18tMkbibtxJPQoTPUv8s3mSXqYzEsrbeRb");
    });
}

TEST_F(BitcoinKeychains, SimpleUsedChangeAddresses) {
    testKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(addresses.size() < 50000);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv");
        EXPECT_TRUE(keychain.markAsUsed("13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv"));
        EXPECT_FALSE(keychain.markAsUsed("13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "1DYvv8T2q2UFv9hQnbLaPZAuQw8mYx3DAD");
    });
}

TEST_F(BitcoinKeychains, NonConsecutivesReceiveUsed) {
    testKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("18tMkbibtxJPQoTPUv8s3mSXqYzEsrbeRb"));
        auto newAddresses = keychain.getAllObservableAddresses(0, 11);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR");
        EXPECT_TRUE(keychain.markAsUsed("151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "1GJr9FHZ1pbR4hjhX24M4L1BDUd2QogYYA");
    });
}

TEST_F(BitcoinKeychains, NonConsecutivesChangeUsed) {
    testKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("1DYvv8T2q2UFv9hQnbLaPZAuQw8mYx3DAD"));
        auto newAddresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv");
        EXPECT_TRUE(keychain.markAsUsed("13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "1F2arsfX5JEDryBVftmzbVFWaGsJaTVwcg");
    });
}

TEST_F(BitcoinKeychains, CheckIfEmpty) {
    testKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_TRUE(keychain.isEmpty());
        auto addresses = keychain.getAllObservableAddresses(0, 40);
        EXPECT_TRUE(keychain.isEmpty());
        keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE);
        EXPECT_TRUE(keychain.isEmpty());
        keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE);
        EXPECT_TRUE(keychain.isEmpty());
        EXPECT_TRUE(keychain.markAsUsed(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58()));
        EXPECT_FALSE(keychain.isEmpty());
    });
}
