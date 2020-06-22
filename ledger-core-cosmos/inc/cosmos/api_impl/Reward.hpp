/*
 *
 * CosmosLikeReward
 *
 * Created by Hakim Aammar on 17/03/2020.
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

#include <cosmos/api/Amount.hpp>
#include <cosmos/api/CosmosLikeReward.hpp>
#include <cosmos/cosmos.hpp>

namespace ledger {
namespace core {
namespace cosmos {
class CosmosLikeReward : public api::CosmosLikeReward {
public:
  explicit CosmosLikeReward() {}

  explicit CosmosLikeReward(const Reward &rewardData,
                            const std::string &delegatorAddress);

  std::string getDelegatorAddress() const override;

  std::string getValidatorAddress() const override;

  std::shared_ptr<api::Amount> getRewardAmount() const override;

private:
  Reward _rewardData;
  std::string _delegatorAddress;
};
} // namespace cosmos
} // namespace core
} // namespace ledger
