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
#include "AbstractWalletFactory.hpp"
#include "../bitcoin/factories/BitcoinLikeWalletFactory.hpp"

namespace ledger {
    namespace core {


        AbstractWalletFactory::AbstractWalletFactory(const api::Currency &currency,
                                                     const std::shared_ptr<WalletPool> &pool) : _pool(pool) {
            _currency = currency;
        }

        const api::Currency &AbstractWalletFactory::getCurrency() const {
            return _currency;
        }

        std::shared_ptr<WalletPool> AbstractWalletFactory::getPool() const {
            auto pool = _pool.lock();
            if (!pool)
                throw Exception(api::ErrorCode::RUNTIME_ERROR, "WalletPool is freed.");
            return pool;
        }

        template <>
        std::shared_ptr<AbstractWalletFactory> make_factory<api::WalletType::BITCOIN>(const api::Currency& currency,
                                                                                      const std::shared_ptr<WalletPool>& pool) {
            return std::make_shared<bitcoin::BitcoinLikeWalletFactory>(currency, pool);
        }

        template <>
        std::shared_ptr<AbstractWalletFactory> make_factory<api::WalletType::ETHEREUM>(const api::Currency& currency,
                                                                                      const std::shared_ptr<WalletPool>& pool) {
            //TODO IMPLEMENT ETHEREUM FACTORY
            return nullptr;
        }

        template <>
        std::shared_ptr<AbstractWalletFactory> make_factory<api::WalletType::RIPPLE>(const api::Currency& currency,
                                                                                      const std::shared_ptr<WalletPool>& pool) {
            //TODO IMPLEMENT RIPPLE FACTORY
            return nullptr;
        }

        template <>
        std::shared_ptr<AbstractWalletFactory> make_factory<api::WalletType::MONERO>(const api::Currency& currency,
                                                                                      const std::shared_ptr<WalletPool>& pool) {
            //TODO IMPLEMENT MONERO FACTORY
            return nullptr;
        }

    }
}