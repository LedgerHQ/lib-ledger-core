/*
 *
 * p2wsh_keychain_test
 *
 * Created by El Khalil Bellakrid on 25/02/2019.
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
#include <src/wallet/bitcoin/keychains/P2WSHBitcoinLikeKeychain.hpp>
#include <src/bitcoin/BitcoinLikeAddress.hpp>
#include <src/wallet/bitcoin/scripts/operators.h>
#include <src/crypto/SHA256.hpp>
#include <src/crypto/HASH160.hpp>
#include <src/crypto/HashAlgorithm.h>
#include <src/collections/DynamicObject.hpp>
#include <src/api/KeychainEngines.hpp>
#include <src/wallet/bitcoin/networks.hpp>
#include "keychain_test_helper.h"

using namespace std;

class BitcoinP2WSHKeychains : public KeychainFixture<P2WSHBitcoinLikeKeychain> {
};

TEST_F(BitcoinP2WSHKeychains, UnitTest) {
    auto currency = currencies::BITCOIN;
    //Script
    const auto& params = currency.bitcoinLikeNetworkParameters.value();
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

TEST_F(BitcoinP2WSHKeychains, tBTCKeychainDerivation) {
    testKeychain(BTC_TESTNET_DATA, [] (P2WSHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBech32(), "tb1qq8r8x00yct9hhj7yqu7fgaglesrh3r4hvq0656ufgpawkmlmvq3scl5mvm");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBech32(), "tb1qa9ku3s4hcszccjz705xgmzuhph76uzsjx0g2mpcf466kav3q7avqa0nqst");
    });
}

TEST_F(BitcoinP2WSHKeychains, BTCKeychainDerivation) {
    testKeychain(BTC_DATA, [] (P2WSHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBech32(), "bc1q70f40q8dpnt0jpeqmthwhxq7g6cdu76mzxpj697693ta7nnaaxuqkq5hqu");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBech32(), "bc1qyr9s9xh08v0tgepcd77fy3y4d07xj34h4wn9t8y9tpgq7j7pk25sy2lzwt");
    });
}

TEST_F(BitcoinP2WSHKeychains, BCHKeychainDerivation) {
    testKeychain(BCH_DATA, [] (P2WSHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE)->toBech32(), "bitcoincash:prnq44gs0t28gqwerhdgah8cmr3wj2k40mjsktwnmj9r7rwnwzdhzvkz89te4");
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::CHANGE)->toBech32(), "bitcoincash:prsl6f5ufwz6lvt4sf679lnj02fq4qnjdnvghj0v78h05z9nfwdqq2pp02tnc");
    });
}