/*
 *
 * address_tests.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/03/2019.
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

#include "StellarFixture.hpp"
#include <wallet/stellar/StellarLikeAddress.hpp>

static std::vector<std::string> pub_keys = {
        "a1083d11720853a2c476a07e29b64e0f9eb2ff894f1e485628faa7b63de77a4f",
        "3a83935fabfdc44749ad4d042dbc4df9b59442f325a27960519fba516adb8a50"
};

static std::vector<std::string> addresses = {
        "GCQQQPIROIEFHIWEO2QH4KNWJYHZ5MX7RFHR4SCWFD5KPNR5455E6BR3",
        "GA5IHE27VP64IR2JVVGQILN4JX43LFCC6MS2E6LAKGP3UULK3OFFBJXR"
};

TEST_F(StellarFixture, AddressFromPubKey) {
    for (auto i = 0; i < pub_keys.size(); i++) {
        StellarLikeAddress address(hex::toByteArray(pub_keys[i]), getCurrency(), Option<std::string>::NONE);
        std::cout << "Address: " << address.toString() << std::endl;
        EXPECT_EQ(address.toString(), addresses[i]);
    }
}