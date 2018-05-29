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
                              "44'/121'/0'");

KeychainTestData LTC_DATA_SEGWIT(ledger::core::networks::LITECOIN,
                                      ledger::core::currencies::LITECOIN,
                                      "Ltub2ZBG1gnknDGm4ezDZmZRmbvfKqQr7sxge7oQwLJLCMVJRC7xxWhWRkpEBgvFw5dP8c3hXhswSoZviH3vKh2hg4GBuCR5GM5DaG9YZVT6TNT",
                                      "49'/121'/0'");

