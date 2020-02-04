/*
 *
 * WalletFixture
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

#include <core/wallet/AbstractWallet.hpp>
#include <core/wallet/WalletStore.hpp>
#include <core/collections/DynamicObject.hpp>
#include <core/api/ErrorCode.hpp>

#include "BaseFixture.hpp"

template <class WalletFactory>
class WalletFixture : public BaseFixture {
public:
    static constexpr auto DEFAULT_PASSWORD = "";
    static constexpr auto DEFAULT_TENANT = "default_tenant";

    void SetUp() override {
        BaseFixture::SetUp();

        services = newDefaultServices(DEFAULT_TENANT, DEFAULT_PASSWORD);
        walletStore = newWalletStore(services);
    }

    void TearDown() override {
        BaseFixture::TearDown();
        
        services.reset();
        walletStore.reset();
    }

    void registerCurrency(api::Currency const &currency) {
        auto walletFactory = std::make_shared<WalletFactory>(currency, services);

        wait(walletStore->addCurrency(currency)); 
        walletStore->registerFactory(currency, walletFactory);
    }

    std::shared_ptr<Services> services;
    std::shared_ptr<WalletStore> walletStore;
};