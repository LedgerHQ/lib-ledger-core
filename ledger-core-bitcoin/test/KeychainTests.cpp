/*
 *
 * KeychainTests
 * ledger-core-bitcoin
 *
 * Created by Alexis Le Provost on 27/01/2020.
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

#include <string>

#include <gtest/gtest.h>

#include <core/crypto/SHA256.hpp>
#include <core/api/KeychainEngines.hpp>

#include <bitcoin/BitcoinNetworks.hpp>
#include <bitcoin/BitcoinLikeCurrencies.hpp>
#include <bitcoin/BitcoinLikeExtendedPublicKey.hpp>
#include <bitcoin/api/BitcoinLikeNetworkParameters.hpp>
#include <bitcoin/keychains/BitcoinLikeKeychain.hpp>
#include <bitcoin/keychains/P2PKHBitcoinLikeKeychain.hpp>
#include <bitcoin/keychains/P2SHBitcoinLikeKeychain.hpp>
#include <bitcoin/keychains/P2WPKHBitcoinLikeKeychain.hpp>
#include <bitcoin/keychains/P2WSHBitcoinLikeKeychain.hpp>
#include <bitcoin/scripts/BitcoinLikeScriptOperators.hpp>

#include <integration/KeychainFixture.hpp>

template <class Keychain>
class BitcoinKeychainMaker : public KeychainFixture<BitcoinKeychainMaker<Keychain>, Keychain> {
    public:
        Keychain makeKeychain(
            std::shared_ptr<PreferencesBackend> backend,
            std::shared_ptr<DynamicObject> configuration,
            KeychainData const &data
        ) {
            return {
                configuration,
                data.currency,
                0,
                BitcoinLikeExtendedPublicKey::fromBase58(
                    data.currency,
                    data.xpub,
                    optional<std::string>(data.derivationPath),
                    configuration
                ),
                backend->getPreferences("keychain")
            };
        }
};

class P2PKHBitcoinKeychains : public BitcoinKeychainMaker<P2PKHBitcoinLikeKeychain> {
};

class P2SHBitcoinKeychains : public BitcoinKeychainMaker<P2SHBitcoinLikeKeychain> {
};

class P2WPKHBitcoinKeychains : public BitcoinKeychainMaker<P2WPKHBitcoinLikeKeychain> {
};

class P2WSHBitcoinKeychains : public BitcoinKeychainMaker<P2WSHBitcoinLikeKeychain> {
};

KeychainData BTC_DATA = {
    currencies::bitcoin(),
    "xpub6DCi5iJ57ZPd5qPzvTm5hUt6X23TJdh9H4NjNsNbt7t7UuTMJfawQWsdWRFhfLwkiMkB1rQ4ZJWLB9YBnzR7kbs9N8b2PsKZgKUHQm1X4or",
    "44'/0'/0'"
};

KeychainData BTC_TESTNET_DATA = {
    currencies::bitcoin_testnet(),
    "tpubDCcvqEHx7prGddpWTfEviiew5YLMrrKy4oJbt14teJZenSi6AYMAs2SNXwYXFzkrNYwECSmobwxESxMCrpfqw4gsUt88bcr8iMrJmbb8P2q",
    "49'/1'/6'"
};

KeychainData BCH_DATA = {
    currencies::bitcoin_cash(),
    "xpub6DLkNEsEhGueDbvD2Te6rVKNFkXDkL7Hc98RNf5AN3b3RzHgFZZHGB7YDCozVnRpoTHfBjJZm7Fcmvvh28DbnS8vme4uexDyaX31jyhKqtT",
    "49'/145'/0'"
};

KeychainData BTG_DATA = {
    currencies::bitcoin_gold(),
    "xpub6CyZVLEtsmz9sQz3SSNGzV7QX7W2tKz3FQauoLX6Ew1n8GMaNoXQqMEbe4vF1AcNTvSEe8gAu2v1iWmsVZTRHJFpoLxwFc9t3P2bALvVEkf",
    "44'/156'/0'"
};

KeychainData ZCASH_DATA = {
    currencies::zcash(), 
    "xpub6CoAz6o5a3XWqnLYTMD1NnkbiEnwSaSXr8mtqnfGdGtLw6383aDr3EuMUMqpmkoRwtbGtkk9ChYPxm9Bv4YftfyA8PP4quotPYtNyEWJEmZ",
    "44'/133'/0'"
};

KeychainData ZENCASH_DATA = {
    currencies::zencash(),
    "xpub661MyMwAqRbcFtXgS5sYJABqqG9YLmC4Q1Rdap9gSE8NqtwybGhePY2gZ29ESFjqJoCu1Rupje8YtGqsefD265TMg7usUDFdp6W1EGMcet8",
    "44'/121'/0'"
};

KeychainData LTC_DATA_LEGACY = {
    currencies::litecoin(),
    "Ltub2YPzVPhWX8TxWJ7XqGm9pogX1sHAH9FX85wUCLVEJyjCftKMELDNRoBqUMTozzweMi9rBhzD748mAYF3y9x36BzBXGZdQYmLQXfkhAknaJo",
    "44'/2'/0'"
};

KeychainData LTC_DATA_SEGWIT = {
    currencies::litecoin(),
    "Ltub2ZBG1gnknDGm4ezDZmZRmbvfKqQr7sxge7oQwLJLCMVJRC7xxWhWRkpEBgvFw5dP8c3hXhswSoZviH3vKh2hg4GBuCR5GM5DaG9YZVT6TNT",
    "49'/2'/0'"
};

KeychainData PEERCOIN_DATA = {
    currencies::peercoin(),
    "r29uBq5Q9CB7aRvUAcj5rj4DsqrkTfSr8KczaATNrkm4Wo8odYxDAxE2T8y9mwZpDVNuEdUYVddi8c3E86EvJK36SFUP421yF5qAfY8A5RDn865p",
    "44'/6'/0'"
};

KeychainData DIGIBYTE_DATA = {
    currencies::digibyte(),
    "xpub6CgBxxzcxNuygpe1v55VpTBbU6X5xubFGS8uYJWovN1wyh58ubdDszeAEpQKpRsbnqtM6UBdiCai6PkPRCThKhbhGTAKAFdr4Hrak5nMDWb",
    "44'/20'/0'"
};

KeychainData HCASH_DATA = {
    currencies::hcash(),
    "xq5hcHJjonMJpmXNU9VMGHgqDpdcenKDKuYYWhMykmwGXyMTVWbEuxCdZbdggHiErfjwQMjP3J5cvhJxNnAU79P2uJ37a7JmjkkC5M8arUWL9nY",
    "44'/171'/0'"
};

KeychainData QTUM_DATA = {
    currencies::qtum(),
    "xpub6BpuTfyPAPFnWuLsQf2EH719KQzi8R5epGo3zChsy9fQm6VGAUzSfJ33UzhoTPcUC9uKEJPo3Kmf5JSWg6q5wtxkaXp8rGVj1pcP3fdiY4c",
    "44'/88'/0'"
};

KeychainData XST_DATA = {
    currencies::stealthcoin(),
    "XSTpb6FdMEENrxPtAKfAyT4zqeVdkuhbCC87D3PzQMttGgHBRbH1xTcEd8z5PUame175N2bZoPAXzecGV9VU9w14rBr5WxqGqta8UCDbk65e69Sp",
    "44'/125'/0'"
};

KeychainData VTC_DATA_LEGACY = {
    currencies::vertcoin(),
    "xpub6DFmiMmBWhSoiogqtM7KGvKvkeDyCYxhEQ29QkCJxYTpi9PgzHc4eCPRvHm8QtiqyjnCLeBeUdFwPkQJRCxSuxfBVF1mG6jEb6SWfDvdaxY",
    "44'/128'/0'"
};

KeychainData VTC_DATA_SEGWIT = {
    currencies::vertcoin(),
    "xpub6DNtAtrKYEr6KMrhBZf73kXHh5GzQrfzS11itibq8FHt2RqWCddfk6zJaZHyRNEH595BZxboSfqRtADXsVLG4wa3oNKVRazUvTTHjRTnjTv",
    "49'/128'/0'"
};

KeychainData VIA_DATA_LEGACY = {
    currencies::viacoin(),
    "xpub6C7vYC4chAVd1LP5bE5fT3UaXJGUiRfPNNpYba6t8YA3c9eHy4K9hcFub1R1k8AT3JBcTsAUtviMyQWZ3F9CJ2KQ8gd7FFmCKkuwEk89auA",
    "44'/14'/0'"
};

KeychainData VIA_DATA_SEGWIT = {
    currencies::viacoin(),
    "xpub6Bs4YEHywoFiEPDtvpmtusUsXdWj6deLjrFbMm6ZmDWjWFqCXnjAeUfnb9SUEzGvXJZU1W3kmHDC9X58fF4Qr7YcgkMrCWR9pMKPkdb2MiL",
    "49'/14'/0'"
};

KeychainData DASH_DATA = {
    currencies::dash(),
    "drkvjRzFDMbmAw3Pi99Fn6R3UbaFM6vCL5rpjLX2cU41xiBAudaXU4ZDtQMBZvfWbXN6XfaQQXELRrRgbrV4ZYPsipogGUYTKkZW3u1GfrRqNXo",
    "44'/5'/0'"
};

KeychainData DOGE_DATA = {
    currencies::dogecoin(),
    "dgub8rJNSCsgxfNy1T6KVP9c53u3T3u5hkUdkgFMVaxW4ZSaWx3rAz2bVnRuLw5RCtU7iQVZvwL2wK5PmNYWGJicneNhahcebqE7eijcSKh2L4g",
    "44'/3'/0'"
};

KeychainData STRATIS_DATA = {
    currencies::stratis(),
    "xq5hcJQXVhUxEsbpbx4k3xdbdKBgD8pVGxyM76L8T3SBgmUYb2WZzcHo1xeQb2zLFBt9TtRhDk8e33abV4vBn6J72fXjXyXhzDiGXKmUN1JrQeo",
    "44'/105'/0'"
};

KeychainData KOMODO_DATA = {
    currencies::komodo(),
    "v4PKUB9tiZXdveqdsJxhg9Ybh3Dp4Mn8a3qGeGoAAm4k7JyuTSXAf6Ljd3wUYRmEqJSz9ExPpT8uv4pcujA47trr7KLBBc8A432UrVfjB9A7mtnw",
    "44'/141'/0'"
};

KeychainData POSWALLET_DATA = {
    currencies::poswallet(),
    "xpub6BtQQpa4LtgoxvyztRD8bhz773YPa7WLYFkxkufC3LH3cqoFzp7nrK6BakrZDXpgGe5VPwpUdLg6oamtve5TJC6nsz43YeX5neFgRLPxjxX",
    "44'/47'/0'"
};

KeychainData PIVX_DATA = {
    currencies::pivx(),
    "ToEA6n4e91QmX5GdCrTPcxmACuxgh4sUYx1QQYv1vQqSBRARTNqs7zfkwAGGHyidVXaJSThDtqquFq8txx3GnTqqAfBMNwoXTUntZtxz8Z7UWkq",
    "44'/77'/0'"
};

KeychainData CLUBCOIN_DATA = {
    currencies::clubcoin(),
    "xpub6DP5DgaoV75F7oGi3UjSo5EDTGKH3aBVLX18AJFdBEZG6fHwcqQ324n4oRf1MaPpK8UjXok4hcxvtxCRVWEeaKkyvuY8y9E4Xn3uGDs6Z3X",
    "44'/79'/0'"
};

KeychainData DECRED_DATA = {
    currencies::decred(),
    "dpubZFUiMExUREbqJQVJkfXSs4wjUb1jwVkoofnPK8Mt95j3PanCyq9Mc4aFnWtRZkhci9ZYPVLZybVLMMkS6g1nKBTN4899KJwGeVBvyumvcjW",
    "44'/42'/0'"
};

TEST_F(P2PKHBitcoinKeychains, KeychainDerivation) {
    testKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv");
    });
}

TEST_F(P2PKHBitcoinKeychains, BCHKeychainDerivation) {
    testKeychain(BCH_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "1BW6hLyZKY9AnUwrU9CwHQJ2c79ho49q4f");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "1ETcGdzh7XGgP2HRUqkYuYEMifihTt8ZiF");
    });
}

TEST_F(P2PKHBitcoinKeychains, BTGKeychainDerivation) {
    testKeychain(BTG_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "GeB2eVacdg6T5U4beqXZgL6vPPPspgqBic");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "GWzECqesKKgjH5RXVt9Na5MYbH2BmeKveF");
    });
}

TEST_F(P2PKHBitcoinKeychains, ZCashKeychainDerivation) {
    testKeychain(ZCASH_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "t1Y1C1GiyhffDV3AMCAcBdYT2H2J9ng2eoY");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "t1eHTEWDziaKtBqaZrSqAPgvm6tBGZezgBR");
    });
}

TEST_F(P2PKHBitcoinKeychains, ZenCashKeychainDerivation) {
    testKeychain(ZENCASH_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "znSHF3jfG9REjJWjFoRrr1WWpNFj4NWeyCW");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "zno29sgBi7CdERRqqjs39mHTdkRHhuodZDa");
    });
}

TEST_F(P2PKHBitcoinKeychains, LTCKeychainDerivation) {
    testKeychain(LTC_DATA_LEGACY, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "LSPT2mGEFnHg9wUZ1dqzFbjxfAtgYizGgT");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "LhUaNteNLDsuM3BiTGJusHkNsLWkLKpgJ5");
    });
}

TEST_F(P2PKHBitcoinKeychains, PeerCoinKeychainDerivation) {
    testKeychain(PEERCOIN_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "P9FBRWVkjHwjituqUzqxDrRSuhv7uoaP6o");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "PFBBNu1g73ppb8pf7fFJjrhmAWkHdnkxda");
    });
}

TEST_F(P2PKHBitcoinKeychains, DigiByteKeychainDerivation) {
    testKeychain(DIGIBYTE_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "DH9fuU2RNiEbwAcgKsh6ZyzRnBfuafsn2y");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "DARxx416tf1afnguTepDJ2nMoUkaDKp5kr");
    });
}

TEST_F(P2PKHBitcoinKeychains, HcashKeychainDerivation) {
    testKeychain(HCASH_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "HGp3XGeaDhj54y2RTCjHLEgfK2X5VhXNyM");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "HQf6cxcoCWQzECNuAvQaHAofxjGA7iuAws");
    });
}

TEST_F(P2PKHBitcoinKeychains, QtumKeychainDerivation) {
    testKeychain(QTUM_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "QSZ4LacFeSfkm9xfj59Aurg6fjqqYF6nVy");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "QNQYhA5BFo9wwoxRUNeS22cqVoSXrgu6Eu");
    });
}

TEST_F(P2PKHBitcoinKeychains, StealthCoinKeychainDerivation) {
    testKeychain(XST_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "SERDi3boPHrCAJTGVe8FmTZWRQEW96AkRT");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "SEotdmKruzfxokECTdN9KHpXKcxxHvUaho");
    });
}

TEST_F(P2PKHBitcoinKeychains, VertCoinKeychainDerivation) {
    testKeychain(VTC_DATA_LEGACY, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "Vf9yiVFBq5nR6xaRVCgQ1yVX4yi6Xd8Mvt");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "VaYyKnW5RFAPZKwrtEBEvqVY2DJKwcHNhQ");
    });
}

TEST_F(P2PKHBitcoinKeychains, ViaCoinKeychainDerivation) {
    testKeychain(VIA_DATA_LEGACY, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "Vd6MV3A4xLjEueenjDU7cf9ZWcWcSc9yLK");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "Vu2fAKQaxAjKH3m3wZUdtDeNKiwaUga1ue");
    });
}

TEST_F(P2PKHBitcoinKeychains, DashKeychainDerivation) {
    testKeychain(DASH_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "XwyjpfwcmJjLptD1akqM3yAxcVmkj8H938");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "XsBhSXm3uYg9zSXbomX6GRt39XUF1mDNhx");
    });
}

TEST_F(P2PKHBitcoinKeychains, DogeKeychainDerivation) {
    testKeychain(DOGE_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "D5LgLdPWThkensZXSX81UqexGcN6f4nu7Z");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "DBjiJaYqzE18U1Akz6gzN5Q3VEdMRshkmf");
    });
}

TEST_F(P2PKHBitcoinKeychains, StratisKeychainDerivation) {
    testKeychain(STRATIS_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "SSiyTLEviUkD9gA69qrABhVTAa17uFhaTm");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "SUWWpvGrDjwtRhxiaPXEVa3Xqa9NX38Feq");
    });
}

TEST_F(P2PKHBitcoinKeychains, KomodoKeychainDerivation) {
    testKeychain(KOMODO_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "RUk15GT5joraNpEpdyvaS7iCZaX4LppVSS");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "RAxANs9rTjhmEK5Kw6QAqMuy1mt6b2gDtf");
    });
}

TEST_F(P2PKHBitcoinKeychains, PosWalletKeychainDerivation) {
    testKeychain(POSWALLET_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "PMAJnswyesefBLuWVzzqeSGh1YNYbSjBun");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "PDT6swtwrZw2qtHVmYLLZrpUbsVTDZ33qP");
    });
}

TEST_F(P2PKHBitcoinKeychains, PivxKeychainDerivation) {
    testKeychain(PIVX_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "DA7H5Zwq1S9cyspqD4qT43bEcQNEf5swZQ");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "DPbkYYCtaH2zfgjQQhHKRy4dq1RKHwBFBD");
    });
}

TEST_F(P2PKHBitcoinKeychains, ClubcoinKeychainDerivation) {
    testKeychain(CLUBCOIN_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "CGnEJw3yCK5MgmA6ivbLBuk8KsWpgtFRXu");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "CSQuTH3kDbnGAG6LmVaLje7kySj6pueego");
    });
}

TEST_F(P2PKHBitcoinKeychains, DecredKeychainDerivation) {
    testKeychain(DECRED_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "Dso5BhFYjnymYwAzsCDEUENmmh7Y9TA4FM7");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "DsSFcrefQbbo8i7u9dQZrtzc6EW6nvVMKZR");
    });
}

TEST_F(P2PKHBitcoinKeychains, SimpleUsedReceiveAddresses) {
    testKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(addresses.size() < 50000);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR");
        EXPECT_TRUE(keychain.markAsUsed("151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR"));
        EXPECT_FALSE(keychain.markAsUsed("151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "18tMkbibtxJPQoTPUv8s3mSXqYzEsrbeRb");
    });
}

TEST_F(P2PKHBitcoinKeychains, SimpleUsedChangeAddresses) {
    testKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(addresses.size() < 50000);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv");
        EXPECT_TRUE(keychain.markAsUsed("13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv"));
        EXPECT_FALSE(keychain.markAsUsed("13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "1DYvv8T2q2UFv9hQnbLaPZAuQw8mYx3DAD");
    });
}

TEST_F(P2PKHBitcoinKeychains, NonConsecutivesReceiveUsed) {
    testKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("18tMkbibtxJPQoTPUv8s3mSXqYzEsrbeRb"));
        auto newAddresses = keychain.getAllObservableAddresses(0, 11);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR");
        EXPECT_TRUE(keychain.markAsUsed("151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "1GJr9FHZ1pbR4hjhX24M4L1BDUd2QogYYA");
    });
}

TEST_F(P2PKHBitcoinKeychains, NonConsecutivesChangeUsed) {
    testKeychain(BTC_DATA, [] (P2PKHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("1DYvv8T2q2UFv9hQnbLaPZAuQw8mYx3DAD"));
        auto newAddresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv");
        EXPECT_TRUE(keychain.markAsUsed("13hSrTAvfRzyEcjRcGS5gLEcNVNDhPvvUv"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "1F2arsfX5JEDryBVftmzbVFWaGsJaTVwcg");
    });
}

TEST_F(P2PKHBitcoinKeychains, CheckIfEmpty) {
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

TEST_F(P2SHBitcoinKeychains, KeychainDerivation) {
    testKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c");
    });
}

TEST_F(P2SHBitcoinKeychains, BCHKeychainDerivation) {
    testKeychain(BCH_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "386ufVmVhuv9AgakZvFno1XsKhuYyF4xGm");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "34oRCa2pifjRjMqkRhadH51BwMXedS3deg");
    });
}

TEST_F(P2SHBitcoinKeychains, BTGKeychainDerivation) {
    testKeychain(BTG_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "ASos7TVvSieocAHzjQy6K2wANaKLsi45cD");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "AHqYBQpa58aLuW6eydeKJyaq21v2oCg8TU");
    });
}

TEST_F(P2SHBitcoinKeychains, ZCASHKeychainDerivation) {
    testKeychain(ZCASH_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "t3fyoi4hioJYbsUtt2rPamWTpzTZLQUddjz");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "t3VrHbQGP1jTVpqbWGcaXY4Z5BHMDYa19xt");
    });
}

TEST_F(P2SHBitcoinKeychains, LTCKeychainDerivation) {
    testKeychain(LTC_DATA_SEGWIT, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "MJGpPKDPeiArQt1UMy8NhcEccdJuRNkbMj");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "MFJ6m85UMdEyRwEem2bziaxTECyKKWnjGp");
    });
}

TEST_F(P2SHBitcoinKeychains, VertCoinKeychainDerivation) {
    testKeychain(VTC_DATA_SEGWIT, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "32Q13F9yNnp55Li3oJ832EnTY5D2skVV6M");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "31jepsZDaBeRzdf8qcrV83gZY4GHKuamPe");
    });
}

TEST_F(P2SHBitcoinKeychains, ViaCoinKeychainDerivation) {
    testKeychain(VIA_DATA_SEGWIT, [] (P2SHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "EMJaVjLFKoB4a5F256DxiMNh6SCR7Um1dZ");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "Ec6sBX4MZDBitLJ9sk6GZ6sfNzEzfReuzA");
    });
}

TEST_F(P2SHBitcoinKeychains, SimpleUsedReceiveAddresses) {
    testKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(addresses.size() < 50000);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        EXPECT_TRUE(keychain.markAsUsed("2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF"));
        EXPECT_FALSE(keychain.markAsUsed("2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "2MwgNx8CZuPbivNX3fXVUFhvNunf9u5q3wJ");
    });
}

TEST_F(P2SHBitcoinKeychains, SimpleUsedChangeAddresses) {
    testKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(addresses.size() < 50000);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c");
        EXPECT_TRUE(keychain.markAsUsed("2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c"));
        EXPECT_FALSE(keychain.markAsUsed("2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "2N3uTrmyNePhAbiUxi8uq7P2J7SxS2bCaji");
    });
}

TEST_F(P2SHBitcoinKeychains, NonConsecutivesReceiveUsed) {
    testKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("2MwgNx8CZuPbivNX3fXVUFhvNunf9u5q3wJ"));
        auto newAddresses = keychain.getAllObservableAddresses(0, 11);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF");
        EXPECT_TRUE(keychain.markAsUsed("2MvuUMAG1NFQmmM69Writ6zTsYCnQHFG9BF"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBase58(), "2N8BfQZXJPJetK6GuBhfWaKmgQxTzKuAa4j");
    });
}

TEST_F(P2SHBitcoinKeychains, NonConsecutivesChangeUsed) {
    testKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
        auto addresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_TRUE(keychain.markAsUsed("2N3uTrmyNePhAbiUxi8uq7P2J7SxS2bCaji"));
        auto newAddresses = keychain.getAllObservableAddresses(0, 10);
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c");
        EXPECT_TRUE(keychain.markAsUsed("2MsMvWTbPMg4eiSudDa5i7y8XNC8fLCok3c"));
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBase58(), "2NDQh3V5suxHfzXbFXyroDYjLVRZB8WCAPc");
    });
}

TEST_F(P2SHBitcoinKeychains, CheckIfEmpty) {
    testKeychain(BTC_TESTNET_DATA, [] (P2SHBitcoinLikeKeychain& keychain) {
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

TEST_F(P2WPKHBitcoinKeychains, tBTCKeychainDerivation) {
    testKeychain(BTC_TESTNET_DATA, [] (P2WPKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBech32(), "tb1qunawpra24prfc46klknlhl0ydy32feajmwpg84");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBech32(), "tb1qm2grk5v7jy42vtnjccx8dwthngslq650n5hm23");
    });
}

TEST_F(P2WPKHBitcoinKeychains, BTCKeychainDerivation) {
    testKeychain(BTC_DATA, [] (P2WPKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBech32(), "bc1q9sz3mlk5t9cm5vz88hjtfetj0z7e7qq7cq472f");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBech32(), "bc1qrkt83qrp40p6hjumpxj803mqg8p708jt8ynx0y");
    });
}

// TODO: https://ledgerhq.atlassian.net/browse/LLC-527
// TEST_F(P2WPKHBitcoinKeychains, BCHKeychainDerivation) {
//     testKeychain(BCH_DATA, [] (P2WPKHBitcoinLikeKeychain& keychain) {
//         EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBech32(), "bitcoincash:qpenyye7dhp9wgugtsh9t3ukdvrnpwyvqyafwfxe0w");
//         EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBech32(), "bitcoincash:qzf6rezvt9agmmwnca4ykj74kppr4dx2hvm6f8kzqr");
//     });
// }

TEST_F(P2WSHBitcoinKeychains, UnitTest) {
    namespace btccore = ledger::core::btccore;

    auto currency = currencies::bitcoin();
    //Script
    const auto& params = networks::getBitcoinLikeNetworkParameters(currency.name);
    HashAlgorithm hashAlgorithm(params.Identifier);
    auto pubKey = hex::toByteArray("210279BE667EF9DCBBAC55A06295CE870B07029BFCDB2DCE28D959F2815B16F81798");
    std::vector<uint8_t> witnessScript;
    //Hash160 of public key
    witnessScript.insert(witnessScript.end(), pubKey.begin(), pubKey.end());
    witnessScript.push_back(btccore::OP_CHECKSIG);
    auto scriptHash = SHA256::bytesToBytesHash(witnessScript);
    BitcoinLikeAddress btcLikeAddress(currency, scriptHash, api::KeychainEngines::BIP173_P2WSH);
    EXPECT_EQ(btcLikeAddress.toBech32(), "bc1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3qccfmv3");
}

TEST_F(P2WSHBitcoinKeychains, tBTCKeychainDerivation) {
    testKeychain(BTC_TESTNET_DATA, [] (P2WSHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBech32(), "tb1qq8r8x00yct9hhj7yqu7fgaglesrh3r4hvq0656ufgpawkmlmvq3scl5mvm");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBech32(), "tb1qa9ku3s4hcszccjz705xgmzuhph76uzsjx0g2mpcf466kav3q7avqa0nqst");
    });
}

TEST_F(P2WSHBitcoinKeychains, BTCKeychainDerivation) {
    testKeychain(BTC_DATA, [] (P2WSHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBech32(), "bc1q70f40q8dpnt0jpeqmthwhxq7g6cdu76mzxpj697693ta7nnaaxuqkq5hqu");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBech32(), "bc1qyr9s9xh08v0tgepcd77fy3y4d07xj34h4wn9t8y9tpgq7j7pk25sy2lzwt");
    });
}

TEST_F(P2WSHBitcoinKeychains, BCHKeychainDerivation) {
    testKeychain(BCH_DATA, [] (P2WSHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBech32(), "bitcoincash:prnq44gs0t28gqwerhdgah8cmr3wj2k40mjsktwnmj9r7rwnwzdhzvkz89te4");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBech32(), "bitcoincash:prsl6f5ufwz6lvt4sf679lnj02fq4qnjdnvghj0v78h05z9nfwdqq2pp02tnc");
    });
}