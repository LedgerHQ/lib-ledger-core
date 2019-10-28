//
// Created by PIERRE POLLASTRI on 28/08/2017.
//

#include <gtest/gtest.h>
#include <crypto/Secp256k1Api.h>
#include <crypto/SHA256.hpp>

using namespace ledger::core;

TEST(SECP256K1, SimpleSignAndVerify) {
    auto secp256k1 = api::Secp256k1::newInstance();
    auto privKey = SHA256::stringToBytesHash("my private key");
    auto data = SHA256::stringToBytesHash("My data");
    auto signature = secp256k1->sign(privKey, data);
    auto pubKey = secp256k1->computePubKey(privKey, true);
    EXPECT_TRUE(secp256k1->verify(data, signature, pubKey));
}