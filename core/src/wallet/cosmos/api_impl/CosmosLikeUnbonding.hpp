/*
 *
 * ledger-core
 *
 * Created by Gerry Agbobada on 2020/03/21
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

#include <algorithm>

#include <api/CosmosLikeUnbonding.hpp>
#include <api/CosmosLikeUnbondingEntry.hpp>
#include <api_impl/BigIntImpl.hpp>
#include <wallet/cosmos/cosmos.hpp>

namespace ledger {
namespace core {
class CosmosLikeUnbondingEntry : public api::CosmosLikeUnbondingEntry {
   public:
    CosmosLikeUnbondingEntry(cosmos::UnbondingEntry entry) : _unbondingEntryData(std::move(entry))
    {
    }

    std::shared_ptr<api::BigInt> getCreationHeight() override
    {
        return std::make_shared<api::BigIntImpl>(_unbondingEntryData.creationHeight);
    }
    std::shared_ptr<api::BigInt> getInitialBalance() override
    {
        return std::make_shared<api::BigIntImpl>(_unbondingEntryData.initialBalance);
    }
    std::shared_ptr<api::BigInt> getBalance() override
    {
        return std::make_shared<api::BigIntImpl>(_unbondingEntryData.balance);
    }
    std::chrono::system_clock::time_point getCompletionTime() override
    {
        return _unbondingEntryData.completionTime;
    }

   private:
    cosmos::UnbondingEntry _unbondingEntryData;
};

class CosmosLikeUnbonding : public api::CosmosLikeUnbonding {
   public:
    CosmosLikeUnbonding(cosmos::Unbonding unbonding) : _unbondingData(std::move(unbonding))
    {
    }

    std::string getDelegatorAddress() override
    {
        return _unbondingData.delegatorAddress;
    }
    std::string getValidatorAddress() override
    {
        return _unbondingData.validatorAddress;
    }
    std::vector<std::shared_ptr<api::CosmosLikeUnbondingEntry>> getEntries() override
    {
        auto result = std::vector<std::shared_ptr<api::CosmosLikeUnbondingEntry>>();
        std::transform(
            _unbondingData.entries.cbegin(),
            _unbondingData.entries.cend(),
            std::back_inserter(result),
            [](const cosmos::UnbondingEntry &entry)
                -> std::shared_ptr<api::CosmosLikeUnbondingEntry> {
                return std::make_shared<CosmosLikeUnbondingEntry>(entry);
            });
        return result;
    }

   private:
    cosmos::Unbonding _unbondingData;
};
}  // namespace core
}  // namespace ledger
