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

#include <wallet/cosmos/api_impl/CosmosLikeReward.hpp>
#include <wallet/cosmos/CosmosLikeCurrencies.hpp>
#include <wallet/common/Amount.h>

#include <cmath>



namespace ledger {
    namespace core {

        CosmosLikeReward::CosmosLikeReward(const cosmos::Reward& rewardData, const std::string& delegatorAddress) :
            _rewardData(rewardData),
            _delegatorAddress(delegatorAddress)
        {}

        std::string CosmosLikeReward::getDelegatorAddress() const {
            return _delegatorAddress;
        }

        std::string CosmosLikeReward::getValidatorAddress() const {
            return _rewardData.validatorAddress;
        }

        std::shared_ptr<api::Amount> CosmosLikeReward::getRewardAmount() const {
            // FIXME std::stod uses radix from current locale (can be . or ,)
            // resulting in incorrect rounding for any locale where the radix is not '.'
            auto roundedReward = std::to_string(std::lround(std::stod(_rewardData.pendingReward.amount)));
            return std::make_shared<Amount>(currencies::ATOM, 0, BigInt::fromDecimal(roundedReward));
        }
    }
}
