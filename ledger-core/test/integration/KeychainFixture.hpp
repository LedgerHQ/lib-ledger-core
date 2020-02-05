/*
 *
 * KeychainFixture
 * ledger-core
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

#pragma once

#include <string>

#include <core/api/Currency.hpp>
#include <core/preferences/PreferencesBackend.hpp>

#include <integration/BaseFixture.hpp>

struct KeychainData {
    api::Currency currency;
    std::string xpub;
    std::string derivationPath;
};

template <class KeychainMaker, class Keychain>
class KeychainFixture : public BaseFixture {
public:
    void testKeychain(
        KeychainData const &data,
        std::function<void(Keychain&)> f
    ) {
        auto backend = std::make_shared<ledger::core::PreferencesBackend>(
            "/preferences/tests.db",
            dispatcher->getMainExecutionContext(),
            resolver
        );
        auto configuration = std::make_shared<ledger::core::DynamicObject>();
    
        dispatcher->getMainExecutionContext()->execute(ledger::qt::make_runnable([=]() {

            auto keychain = static_cast<KeychainMaker*>(this)->makeKeychain(
                backend, configuration, data);
            
            f(keychain);
            dispatcher->stop();          
        }));
        dispatcher->waitUntilStopped();
    }
};