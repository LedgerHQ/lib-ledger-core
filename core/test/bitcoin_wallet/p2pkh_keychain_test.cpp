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
#include <ledger/core/wallet/bitcoin/keychains/P2PKHBitcoinLikeKeychain.hpp>
#include <NativeThreadDispatcher.hpp>
#include <NativePathResolver.hpp>
#include <ledger/core/wallet/bitcoin/networks.hpp>
#include <ledger/core/api_impl/ConfigurationImpl.hpp>

using namespace ledger::core;

const std::string XPUB = "xpub6DCi5iJ57ZPd5qPzvTm5hUt6X23TJdh9H4NjNsNbt7t7UuTMJfawQWsdWRFhfLwkiMkB1rQ4ZJWLB9YBnzR7kbs9N8b2PsKZgKUHQm1X4or";

static void testKeychain(std::string xpub, std::function<void (P2PKHBitcoinLikeKeychain&)> f) {
    auto resolver = std::make_shared<NativePathResolver>();
    auto dispatcher = std::make_shared<NativeThreadDispatcher>();
    auto preferencesLock = dispatcher->newLock();
    auto backend = std::make_shared<ledger::core::PreferencesBackend>(
            "/preferences/tests.db",
            dispatcher->getSerialExecutionContext("worker"),
            resolver
    );
    auto configuration = std::make_shared<ConfigurationImpl>();

    dispatcher->getMainExecutionContext()->execute(make_runnable([=] () {
        P2PKHBitcoinLikeKeychain keychain(
                configuration,
                networks::BITCOIN,
                0,
                api::BitcoinLikeExtendedPublicKey::fromBase58(networks::BITCOIN, xpub, optional<std::string>("44'/0'/0'")),
                backend->getPreferences("keychain")
        );
        f(keychain);
        dispatcher->stop();
    }));

    dispatcher->waitUntilStopped();
    resolver->clean();
}

TEST(BitcoinKeychains, KeychainDerivation) {
    testKeychain(XPUB, [] (P2PKHBitcoinLikeKeychain& keychain) {
        EXPECT_EQ(keychain.getFreshAddress(BitcoinLikeKeychain::KeyPurpose::RECEIVE), "151krzHgfkNoH3XHBzEVi6tSn4db7pVjmR");
    });
}