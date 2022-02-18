/*
 *
 * bech32_test
 * ledger-core
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Ledger
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
#include <ledger/core/math/Base58.hpp>
#include <ledger/core/utils/hex.h>
#include <ledger/core/collections/DynamicObject.hpp>
#include <math/bech32/Bech32Factory.h>
#include <math/bech32/Bech32Parameters.h>

using namespace ledger::core;

struct Bech32_TestCase {
    std::string _network_id;
    std::string _address;
    uint8_t _version;
    std::string _scriptPubKey;
    // scriptPubKey has following format:
    // - witness version (1 byte, possible values: 0..16 inclusive)
    // -- 0 - segwit address (P2WPKH, P2WSH)
    // -- 1 - taproot address (P2TR)
    // - key hash or key (P2TR):
    // -- key hash length (1 byte, posible values: for witness version 0: 20 or 32, see BIP 141)
    // -- key hash (for witness version 0: 20 or 32 bytes)
    inline std::string getPubKeyHash() const {
        if (_scriptPubKey.size() < 6)
            throw std::logic_error(std::string("Bech32_TestCase._scriptPubKey is too short: ") + _scriptPubKey);
        // The length of hex encoded pubKeyHash must include at least:
        // version: exactly 2 chars
        // length: exactly 2 chars
        // value: at least 2 chars
        return _scriptPubKey.substr(4); // skip version and length
    }
};

std::string toLower(std::string s) {
    std::string l;
    std::transform(s.begin(), s.end(), std::back_inserter(l), [](unsigned char c){ return std::tolower(c); });
    return l;
}

std::vector<Bech32_TestCase> bech32_segwit_testcases = {
    // Official test vectors
    {"btc", "BC1QW508D6QEJXTDG4Y5R3ZARVARY0C5XW7KV8F3T4",                                  // BIP 173 & 350
            0x00, "0014751e76e8199196d454941c45d1b3a323f1433bd6"},
    {"btc", "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4",
            0x00, "0014751e76e8199196d454941c45d1b3a323f1433bd6"},
    {"btc_testnet", "tb1qqqqqp399et2xygdj5xreqhjjvcmzhxw4aywxecjdzew6hylgvsesrxh6hy",      // BIP 173 & 350
            0x00, "0020000000c4a5cad46221b2a187905e5266362b99d5e91c6ce24d165dab93e86433"},
    {"btc_testnet", "tb1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3q0sl5k7",      // BIP 173 & 350
            0x00, "00201863143c14c5166804bd19203356da136c985678cd4d27a1b8c6329604903262"},

    {"btc", "bc1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vqzk5jj0",              // BIP 350
            0x01, "512079be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"},
    {"btc_testnet", "tb1pqqqqp399et2xygdj5xreqhjjvcmzhxw4aywxecjdzew6hylgvsesf3hn0c",      // BIP 350
            0x01, "5120000000c4a5cad46221b2a187905e5266362b99d5e91c6ce24d165dab93e86433"},

    // Source: https://github.com/bitcoin/bitcoin/blob/master/test/functional/test_framework/segwit_addr.py
    // P2WPKH
    {"btc_regtest", "bcrt1qthmht0k2qnh3wy7336z05lu2km7emzfpm3wg46",
            0x00, "00145df775beca04ef1713d18e84fa7f8ab6fd9d8921"},
    // P2WSH
    {"btc_regtest", "bcrt1qqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqqq3xueyj",
            0x00, "00200000000000000000000000000000000000000000000000000000000000000000"},
    {"btc_regtest", "bcrt1qft5p2uhsdcdc3l2ua4ap5qqfg4pjaqlp250x7us7a8qqhrxrxfsqseac85",
            0x00, "00204ae81572f06e1b88fd5ced7a1a000945432e83e1551e6f721ee9c00b8cc33260"},
    // P2TR
    {"btc_regtest", "bcrt1p0xlxvlhemja6c4dqv22uapctqupfhlxm9h8z3k2e72q4k9hcz7vqc8gma6",
            0x01, "512079be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798"},

    // Bitcoin cash
    // Source: https://github.com/LedgerHQ/ledger-live-common/blob/master/src/__tests__/families/bitcoin/wallet-btc/utils.test.ts
    {"abc", "bitcoincash:qqmyc72pkyx8c0ppgeuummq6clzverhxnsk3qh6jcf",
            0x00, "0014364c7941b10c7c3c214679cdec1ac7c4cc8ee69c"},
    {"abc", "bitcoincash:qzl0x0982hy9xrh99wdnejx4eecdn02jv58as5p595",
            0x00, "0014bef33ca755c8530ee52b9b3cc8d5ce70d9bd5265"},

    // Litecoin
    {"ltc", "ltc1q3e4eh3lldvx97zg6d74x4ns6v5a4j4hwwqycwv",
            0x00, "00148e6b9bc7ff6b0c5f091a6faa6ace1a653b5956ee"},

    // Digibyte
    {"dgb", "dgb1q7zjgqa23xzf602ljfrc94248a9u27xml08nhct",
            0x00, "0014f0a48075513093a7abf248f05aaaa7e978af1b7f"}
};

std::vector<std::string> bech32_invalid_addresses = {
    // Official test vectors from BIP 173
    "tc1qw508d6qejxtdg4y5r3zarvary0c5xw7kg3g4ty",                                   // Invalid human-readable part
    "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t5",                                   // Invalid checksum
    "BC13W508D6QEJXTDG4Y5R3ZARVARY0C5XW7KN40WF2",                                   // Invalid witness version
    "bc1rw5uspcuh",                                                                 // Invalid program length
    "bc10w508d6qejxtdg4y5r3zarvary0c5xw7kw508d6qejxtdg4y5r3zarvary0c5xw7kw5rljs90", // Invalid program length
    "BC1QR508D6QEJXTDG4Y5R3ZARVARYV98GJ9P",                                         // Invalid program length for witness version 0 (per BIP141)
    "tb1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3q0sL5k7",               // Mixed case
    "bc1zw508d6qejxtdg4y5r3zarvaryvqyzf3du",                                        // zero padding of more than 4 bits
    "tb1qrp33g0q5c5txsp9arysrx4k6zdkfs4nce4xj0gdcccefvpysxf3pjxtptv",               // Non-zero padding in 8-to-5 conversion
    "bc1gmk9yu",                                                                    // Empty data section

    // Source: https://github.com/LedgerHQ/ledger-live-common/blob/master/src/__tests__/families/bitcoin/wallet-btc/utils.test.ts
    "bitcoincash:qqmyc72pkyx8c0ppgeuummq6clzverhxnsk3qh6jc1",
    "bitcoincash:qzl0x0982hy9xrh99wdnejx4eecdn02jv58as5p599",
    "ltc1q3e4eh3lldvx97zg6d74x4ns6v5a4j4hwwqycww",
    "dgb1q7zjgqa23xzf602ljfrc94248a9u27xml08nhcc",

    // Addresses below are correctly encoded but aren't accepted
    // because their meaning isn't clear
    "bc1zw508d6qejxtdg4y5r3zarvaryvaxxpcs",                                         // Unknown witness versions
    // witness version: 0x02, decoded: "5210751e76e8199196d454941c45d1b3a323"

    "bc1pw508d6qejxtdg4y5r3zarvary0c5xw7kw508d6qejxtdg4y5r3zarvary0c5xw7kt5nd6y",   // Data are 40 bytes long (only 32 bytes for P2TR)
    // witness version: 0x01,
    // decoded: "5128751e76e8199196d454941c45d1b3a323f1433bd6751e76e8199196d454941c45d1b3a323f1433bd6"

    "BC1SW50QGDZ25J",                                                               // Unknown witness version and decoded data too short
    // witness version: 0x10, decoded: "6002751e"
};

TEST(Bech32, EncodeAddress) {
    for (const Bech32_TestCase& testcase : bech32_segwit_testcases) {
        Option<std::shared_ptr<Bech32>> opt_bech32 = Bech32Factory::newBech32Instance(testcase._network_id);
        EXPECT_TRUE(opt_bech32.hasValue());
        std::shared_ptr<Bech32> sp_bech32 = opt_bech32.getValue();
        EXPECT_TRUE(sp_bech32.get() != nullptr);

        const std::string address = sp_bech32->encode(hex::toByteArray(testcase.getPubKeyHash()), {testcase._version});
        std::cerr << "Encoded: " << testcase._scriptPubKey << " -> " << address << std::endl;
        EXPECT_EQ(toLower(testcase._address), address); // address is encoded correctly (Bech32 is case-insensitive)
    }

    std::cout << std::endl;
}

TEST(Bech32, DecodeValidAddress) {
    for (const Bech32_TestCase& testcase : bech32_segwit_testcases) {
        Option<std::shared_ptr<Bech32>> opt_bech32 = Bech32Factory::newBech32Instance(testcase._network_id);
        EXPECT_TRUE(opt_bech32.hasValue());
        std::shared_ptr<Bech32> sp_bech32 = opt_bech32.getValue();
        EXPECT_TRUE(sp_bech32.get() != nullptr);

        std::pair<std::vector<uint8_t>, std::vector<uint8_t>> decoded;
        try {
            decoded = sp_bech32->decode(testcase._address);
        } catch (const std::exception& e) {
            FAIL() << e.what() << std::endl
                   << "Input address is: \"" << testcase._address
                   << "\"" << std::endl;
        } catch (...) {
            FAIL() << "An error happened while decoding address \""
                   << testcase._address << "\"" << std::endl;
        }

        std::cerr << testcase._network_id << " "
                  << hex::toString(decoded.first) << " "
                  << hex::toString(decoded.second) << std::endl;

        EXPECT_EQ(decoded.first.size(), 1);                                               // version takes one byte
        EXPECT_EQ(decoded.first[0], testcase._version);                                   // version is decoded correctly
        EXPECT_EQ(hex::toString(decoded.second).size(), testcase.getPubKeyHash().size()); // pubKeyHash has a correct size
        EXPECT_EQ(hex::toString(decoded.second), testcase.getPubKeyHash());               // pubKeyHash is decoded correctly

        std::cerr << "Decoded: " << testcase._address << " -> "
                  << hex::toString(decoded.first) << " " << hex::toString(decoded.second) << std::endl;
    }
}

TEST(Bech32, DecodeInvalidAddresses) {
    const std::vector<std::string> netIds = {"btc", "btc_testnet", "btc_regtest", "abc", "dgb", "ltc"};
    for (const std::string& addr : bech32_invalid_addresses) {
        for (const std::string& net_id : netIds) {
            Option<std::shared_ptr<Bech32>> opt_bech32 = Bech32Factory::newBech32Instance(net_id);
            EXPECT_TRUE(opt_bech32.hasValue());
            std::shared_ptr<Bech32> sp_bech32 = opt_bech32.getValue();
            EXPECT_TRUE(sp_bech32.get() != nullptr);

            try {
                std::cerr << "Trying to decode " << addr << " for network " << net_id << " ";
                std::pair<std::vector<uint8_t>, std::vector<uint8_t>> decoded
                        = sp_bech32->decode(addr);
            } catch (const Exception& e) {
                EXPECT_EQ(e.getErrorCode(), api::ErrorCode::INVALID_BECH32_FORMAT);
                std::cerr << "got " << e.getMessage() << std::endl;
                continue;
            } catch (...) {
                FAIL() << "Incorrect type of exception thrown for this address / network id combination." << std::endl
                       << "\taddress: " << addr << std::endl
                       << "\tnetwork id: " << net_id << std::endl;
            }
            FAIL() << "Bech32::decode() must throw an exception for this address / network id combination." << std::endl
                   << "\taddress: " << addr << std::endl
                   << "\tnetwork id: " << net_id << std::endl;
        }
    }
}

