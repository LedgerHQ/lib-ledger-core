/*
 *
 * StellarLikeAccount.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/02/2019.
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

#include "StellarLikeAccount.hpp"
#include <wallet/stellar/StellarLikeWallet.hpp>

namespace ledger {
    namespace core {

        StellarLikeAccount::StellarLikeAccount(const std::shared_ptr<StellarLikeWallet> &wallet,
                                               const StellarLikeAccountParams &params)
                                               : AbstractAccount(wallet, params.index), _params(params) {

        }

        bool StellarLikeAccount::isSynchronizing() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        std::shared_ptr<api::EventBus> StellarLikeAccount::synchronize() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        void StellarLikeAccount::startBlockchainObservation() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        void StellarLikeAccount::stopBlockchainObservation() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        bool StellarLikeAccount::isObservingBlockchain() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        std::string StellarLikeAccount::getRestoreKey() {
            return _params.keychain->getRestoreKey();
        }

        FuturePtr<ledger::core::Amount> StellarLikeAccount::getBalance() {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        Future<AbstractAccount::AddressList> StellarLikeAccount::getFreshPublicAddresses() {
            auto self = getSelf();
            return async<AbstractAccount::AddressList>([=] () {
                return AbstractAccount::AddressList({self->_params.keychain->getAddress()});
            });
        }

        Future<std::vector<std::shared_ptr<api::Amount>>>
        StellarLikeAccount::getBalanceHistory(const std::string &start, const std::string &end,
                                              api::TimePeriod precision) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        Future<api::ErrorCode> StellarLikeAccount::eraseDataSince(const std::chrono::system_clock::time_point &date) {
            throw make_exception(api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Not implemented");
        }

        std::shared_ptr<StellarLikeAccount> StellarLikeAccount::getSelf() {
            return std::dynamic_pointer_cast<StellarLikeAccount>(shared_from_this());
        }
    }
}
