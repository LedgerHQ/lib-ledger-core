/*
 *
 * keychain_test_helper
 * ledger-core
 *
 * Created by El Khalil Bellakrid on 16/05/2018.
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

#include "keychain_test_helper.h"

//abandon
//bip44
//const std::string XPUB = "tpubDC5FSnBiZDMmhiuCmWAYsLwgLYrrT9rAqvTySfuCCrgsWz8wxMXUS9Tb9iVMvcRbvFcAHGkMD5Kx8koh4GquNGNTfohfk7pgjhaPCdXpoba";
//bip49
//const std::string XPUB = "upub5EFU65HtV5TeiSHmZZm7FUffBGy8UKeqp7vw43jYbvZPpoVsgU93oac7Wk3u6moKegAEWtGNF8DehrnHtv21XXEMYRUocHqguyjknFHYfgY";

KeychainTestData BTC_DATA(ledger::core::networks::BITCOIN,
                  ledger::core::currencies::BITCOIN,
                  "xpub6DCi5iJ57ZPd5qPzvTm5hUt6X23TJdh9H4NjNsNbt7t7UuTMJfawQWsdWRFhfLwkiMkB1rQ4ZJWLB9YBnzR7kbs9N8b2PsKZgKUHQm1X4or",
                  "44'/0'/0'");

KeychainTestData BTC_TESTNET_DATA(ledger::core::networks::BITCOIN_TESTNET,
                          ledger::core::currencies::BITCOIN_TESTNET,
                          "tpubDCcvqEHx7prGddpWTfEviiew5YLMrrKy4oJbt14teJZenSi6AYMAs2SNXwYXFzkrNYwECSmobwxESxMCrpfqw4gsUt88bcr8iMrJmbb8P2q",
                          "49'/1'/6'");

KeychainTestData BCH_DATA(ledger::core::networks::BITCOIN_CASH,
                  ledger::core::currencies::BITCOIN_CASH,
                  "xpub6DLkNEsEhGueDbvD2Te6rVKNFkXDkL7Hc98RNf5AN3b3RzHgFZZHGB7YDCozVnRpoTHfBjJZm7Fcmvvh28DbnS8vme4uexDyaX31jyhKqtT",
                  "49'/145'/0'");

KeychainTestData BTG_DATA(ledger::core::networks::BITCOIN_GOLD,
                  ledger::core::currencies::BITCOIN_GOLD,
                  "xpub6CyZVLEtsmz9sQz3SSNGzV7QX7W2tKz3FQauoLX6Ew1n8GMaNoXQqMEbe4vF1AcNTvSEe8gAu2v1iWmsVZTRHJFpoLxwFc9t3P2bALvVEkf",
                  "44'/156'/0'");

KeychainTestData ZCASH_DATA(ledger::core::networks::ZCASH,
                  ledger::core::currencies::ZCASH,
                  "xpub6CoAz6o5a3XWqnLYTMD1NnkbiEnwSaSXr8mtqnfGdGtLw6383aDr3EuMUMqpmkoRwtbGtkk9ChYPxm9Bv4YftfyA8PP4quotPYtNyEWJEmZ",
                  "44'/133'/0'");

KeychainTestData ZENCASH_DATA(ledger::core::networks::ZENCASH,
                            ledger::core::currencies::ZENCASH,
                            "xpub661MyMwAqRbcFtXgS5sYJABqqG9YLmC4Q1Rdap9gSE8NqtwybGhePY2gZ29ESFjqJoCu1Rupje8YtGqsefD265TMg7usUDFdp6W1EGMcet8",
                            "44'/121'/0'");

KeychainTestData LTC_DATA_LEGACY(ledger::core::networks::LITECOIN,
                              ledger::core::currencies::LITECOIN,
                              "Ltub2YPzVPhWX8TxWJ7XqGm9pogX1sHAH9FX85wUCLVEJyjCftKMELDNRoBqUMTozzweMi9rBhzD748mAYF3y9x36BzBXGZdQYmLQXfkhAknaJo",
                              "44'/2'/0'");

KeychainTestData LTC_DATA_SEGWIT(ledger::core::networks::LITECOIN,
                                      ledger::core::currencies::LITECOIN,
                                      "Ltub2ZBG1gnknDGm4ezDZmZRmbvfKqQr7sxge7oQwLJLCMVJRC7xxWhWRkpEBgvFw5dP8c3hXhswSoZviH3vKh2hg4GBuCR5GM5DaG9YZVT6TNT",
                                      "49'/2'/0'");

KeychainTestData PEERCOIN_DATA(ledger::core::networks::PEERCOIN,
                                 ledger::core::currencies::PEERCOIN,
                                 "r29uBq5Q9CB7aRvUAcj5rj4DsqrkTfSr8KczaATNrkm4Wo8odYxDAxE2T8y9mwZpDVNuEdUYVddi8c3E86EvJK36SFUP421yF5qAfY8A5RDn865p",
                                 "44'/6'/0'");

KeychainTestData DIGIBYTE_DATA(ledger::core::networks::DIGIBYTE,
                               ledger::core::currencies::DIGIBYTE,
                               "xpub6CgBxxzcxNuygpe1v55VpTBbU6X5xubFGS8uYJWovN1wyh58ubdDszeAEpQKpRsbnqtM6UBdiCai6PkPRCThKhbhGTAKAFdr4Hrak5nMDWb",
                               "44'/20'/0'");

KeychainTestData HCASH_DATA(ledger::core::networks::HCASH,
                               ledger::core::currencies::HCASH,
                               "xq5hcHJjonMJpmXNU9VMGHgqDpdcenKDKuYYWhMykmwGXyMTVWbEuxCdZbdggHiErfjwQMjP3J5cvhJxNnAU79P2uJ37a7JmjkkC5M8arUWL9nY",
                               "44'/171'/0'");

KeychainTestData QTUM_DATA(ledger::core::networks::QTUM,
                            ledger::core::currencies::QTUM,
                            "xpub6BpuTfyPAPFnWuLsQf2EH719KQzi8R5epGo3zChsy9fQm6VGAUzSfJ33UzhoTPcUC9uKEJPo3Kmf5JSWg6q5wtxkaXp8rGVj1pcP3fdiY4c",
                            "44'/88'/0'");

KeychainTestData XST_DATA(ledger::core::networks::STEALTHCOIN,
                           ledger::core::currencies::STEALTHCOIN,
                           "XSTpb6FdMEENrxPtAKfAyT4zqeVdkuhbCC87D3PzQMttGgHBRbH1xTcEd8z5PUame175N2bZoPAXzecGV9VU9w14rBr5WxqGqta8UCDbk65e69Sp",
                           "44'/125'/0'");

KeychainTestData VTC_DATA_LEGACY(ledger::core::networks::VERTCOIN,
                          ledger::core::currencies::VERTCOIN,
                          "xpub6DFmiMmBWhSoiogqtM7KGvKvkeDyCYxhEQ29QkCJxYTpi9PgzHc4eCPRvHm8QtiqyjnCLeBeUdFwPkQJRCxSuxfBVF1mG6jEb6SWfDvdaxY",
                          "44'/128'/0'");

KeychainTestData VTC_DATA_SEGWIT(ledger::core::networks::VERTCOIN,
                                 ledger::core::currencies::VERTCOIN,
                                 "xpub6DNtAtrKYEr6KMrhBZf73kXHh5GzQrfzS11itibq8FHt2RqWCddfk6zJaZHyRNEH595BZxboSfqRtADXsVLG4wa3oNKVRazUvTTHjRTnjTv",
                                 "49'/128'/0'");

KeychainTestData VIA_DATA_LEGACY(ledger::core::networks::VIACOIN,
                                 ledger::core::currencies::VIACOIN,
                                 "xpub6C7vYC4chAVd1LP5bE5fT3UaXJGUiRfPNNpYba6t8YA3c9eHy4K9hcFub1R1k8AT3JBcTsAUtviMyQWZ3F9CJ2KQ8gd7FFmCKkuwEk89auA",
                                 "44'/14'/0'");

KeychainTestData VIA_DATA_SEGWIT(ledger::core::networks::VIACOIN,
                                 ledger::core::currencies::VIACOIN,
                                 "xpub6Bs4YEHywoFiEPDtvpmtusUsXdWj6deLjrFbMm6ZmDWjWFqCXnjAeUfnb9SUEzGvXJZU1W3kmHDC9X58fF4Qr7YcgkMrCWR9pMKPkdb2MiL",
                                 "49'/14'/0'");

KeychainTestData DASH_DATA(ledger::core::networks::DASH,
                                 ledger::core::currencies::DASH,
                                 "drkvjRzFDMbmAw3Pi99Fn6R3UbaFM6vCL5rpjLX2cU41xiBAudaXU4ZDtQMBZvfWbXN6XfaQQXELRrRgbrV4ZYPsipogGUYTKkZW3u1GfrRqNXo",
                                 "44'/5'/0'");

KeychainTestData DOGE_DATA(ledger::core::networks::DOGECOIN,
                           ledger::core::currencies::DOGECOIN,
                           "dgub8rJNSCsgxfNy1T6KVP9c53u3T3u5hkUdkgFMVaxW4ZSaWx3rAz2bVnRuLw5RCtU7iQVZvwL2wK5PmNYWGJicneNhahcebqE7eijcSKh2L4g",
                           "44'/3'/0'");

KeychainTestData STRATIS_DATA(ledger::core::networks::STRATIS,
                           ledger::core::currencies::STRATIS,
                           "xq5hcJQXVhUxEsbpbx4k3xdbdKBgD8pVGxyM76L8T3SBgmUYb2WZzcHo1xeQb2zLFBt9TtRhDk8e33abV4vBn6J72fXjXyXhzDiGXKmUN1JrQeo",
                           "44'/105'/0'");

KeychainTestData KOMODO_DATA(ledger::core::networks::KOMODO,
                              ledger::core::currencies::KOMODO,
                              "v4PKUB9tiZXdveqdsJxhg9Ybh3Dp4Mn8a3qGeGoAAm4k7JyuTSXAf6Ljd3wUYRmEqJSz9ExPpT8uv4pcujA47trr7KLBBc8A432UrVfjB9A7mtnw",
                              "44'/141'/0'");

KeychainTestData POSWALLET_DATA(ledger::core::networks::POSWALLET,
                             ledger::core::currencies::POSWALLET,
                             "xpub6BtQQpa4LtgoxvyztRD8bhz773YPa7WLYFkxkufC3LH3cqoFzp7nrK6BakrZDXpgGe5VPwpUdLg6oamtve5TJC6nsz43YeX5neFgRLPxjxX",
                             "44'/47'/0'");

KeychainTestData PIVX_DATA(ledger::core::networks::PIVX,
                                ledger::core::currencies::PIVX,
                                "ToEA6n4e91QmX5GdCrTPcxmACuxgh4sUYx1QQYv1vQqSBRARTNqs7zfkwAGGHyidVXaJSThDtqquFq8txx3GnTqqAfBMNwoXTUntZtxz8Z7UWkq",
                                "44'/77'/0'");

KeychainTestData CLUBCOIN_DATA(ledger::core::networks::CLUBCOIN,
                           ledger::core::currencies::CLUBCOIN,
                           "xpub6DP5DgaoV75F7oGi3UjSo5EDTGKH3aBVLX18AJFdBEZG6fHwcqQ324n4oRf1MaPpK8UjXok4hcxvtxCRVWEeaKkyvuY8y9E4Xn3uGDs6Z3X",
                           "44'/79'/0'");