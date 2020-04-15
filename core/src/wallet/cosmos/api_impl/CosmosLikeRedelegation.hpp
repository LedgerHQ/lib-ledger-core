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

#include <api/CosmosLikeRedelegation.hpp>
#include <api/CosmosLikeRedelegationEntry.hpp>
#include <api_impl/BigIntImpl.hpp>
#include <wallet/cosmos/cosmos.hpp>

namespace ledger {
namespace core {
class CosmosLikeRedelegationEntry : public api::CosmosLikeRedelegationEntry {
   public:
    CosmosLikeRedelegationEntry(const cosmos::RedelegationEntry &entry) :
        _redelegationEntryData(entry)
    {
    }

    std::shared_ptr<api::BigInt> getCreationHeight() override
    {
        return std::make_shared<api::BigIntImpl>(_redelegationEntryData.creationHeight);
    }
    std::shared_ptr<api::BigInt> getInitialBalance() override
    {
        return std::make_shared<api::BigIntImpl>(_redelegationEntryData.initialBalance);
    }
    std::shared_ptr<api::BigInt> getBalance() override
    {
        return std::make_shared<api::BigIntImpl>(_redelegationEntryData.balance);
    }
    std::chrono::system_clock::time_point getCompletionTime() override
    {
        return _redelegationEntryData.completionTime;
    }

   private:
    cosmos::RedelegationEntry _redelegationEntryData;
};

class CosmosLikeRedelegation : public api::CosmosLikeRedelegation {
   public:
    CosmosLikeRedelegation(const cosmos::Redelegation &redelegation) :
        _redelegationData(redelegation)
    {
    }

    std::string getDelegatorAddress() override
    {
        return _redelegationData.delegatorAddress;
    }
    std::string getSrcValidatorAddress() override
    {
        return _redelegationData.srcValidatorAddress;
    }
    std::string getDstValidatorAddress() override
    {
        return _redelegationData.dstValidatorAddress;
    }
    std::vector<std::shared_ptr<api::CosmosLikeRedelegationEntry>> getEntries() override
    {
        auto result = std::vector<std::shared_ptr<api::CosmosLikeRedelegationEntry>>();
        std::transform(
            _redelegationData.entries.cbegin(),
            _redelegationData.entries.cend(),
            result.begin(),
            [](const cosmos::RedelegationEntry &entry)
                -> std::shared_ptr<api::CosmosLikeRedelegationEntry> {
                return std::make_shared<CosmosLikeRedelegationEntry>(entry);
            });
        return result;
    }

   private:
    cosmos::Redelegation _redelegationData;
};
}  // namespace core
}  // namespace ledger
