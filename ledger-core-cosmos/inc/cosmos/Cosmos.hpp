/*
 *
 * Cosmos.hpp
 *
 * Created by Gerry Agbobada on 23/06/2020
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
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
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

#include <core/api/Account.hpp>
#include <core/api/ErrorCodeCallback.hpp>
#include <core/api/Services.hpp>
#include <core/api/WalletStore.hpp>
#include <cosmos/Account.hpp>
#include <cosmos/api/Cosmos.hpp>
#include <cosmos/api_impl/Operation.hpp>

namespace ledger {
namespace core {
namespace cosmos {
struct Cosmos : ::ledger::core::api::Cosmos {
  Cosmos() = default;
  ~Cosmos() = default;

  void registerInto(
      std::shared_ptr<api::Services> const &services,
      std::shared_ptr<api::WalletStore> const &walletStore,
      std::shared_ptr<api::ErrorCodeCallback> const &callback) override;

  std::shared_ptr<api::CosmosLikeAccount>
  fromCoreAccount(const std::shared_ptr<api::Account> &coreAccount) override;

  std::shared_ptr<api::CosmosLikeOperation> fromCoreOperation(
      const std::shared_ptr<api::Operation> &coreOperation) override;
};
} // namespace cosmos
} // namespace core
} // namespace ledger
