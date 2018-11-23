/*
 *
 * AbstractWalletFactory
 * ledger-core
 *
 * Created by Pierre Pollastri on 16/05/2017.
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
#pragma once

#include <memory>
#include <wallet/pool/database/WalletDatabaseEntry.hpp>
#include "AbstractWallet.hpp"

namespace ledger {
    namespace core {
        class AbstractWalletFactory {
        public:
            AbstractWalletFactory(const api::Currency& currency, const std::shared_ptr<WalletPool>& pool);
            virtual std::shared_ptr<AbstractWallet> build(const WalletDatabaseEntry& entry) = 0;
            const api::Currency& getCurrency() const;
            virtual ~AbstractWalletFactory() {};

        protected:
            std::shared_ptr<WalletPool> getPool() const;

        private:
            api::Currency _currency;
            std::weak_ptr<WalletPool> _pool;
        };

        template <api::WalletType>
        std::shared_ptr<AbstractWalletFactory> make_factory(const api::Currency& currency, const std::shared_ptr<WalletPool>& pool);
    }
}
