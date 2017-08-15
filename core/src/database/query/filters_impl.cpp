/*
 *
 * filters_impl
 * ledger-core
 *
 * Created by Pierre Pollastri on 30/06/2017.
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

#include "ConditionQueryFilter.h"
#include <utils/DateUtils.hpp>
#include <cereal/external/base64.hpp>
#include <api/TrustLevel.hpp>
#include <api/OperationType.hpp>

namespace ledger {
    namespace core {

        std::shared_ptr<api::QueryFilter> api::QueryFilter::accountEq(const std::string &accountUid) {
            return std::make_shared<ConditionQueryFilter<std::string>>("account_uid", "=", accountUid);
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::accountNeq(const std::string &accountUid) {
            return std::make_shared<ConditionQueryFilter<std::string>>("account_uid", "<>", accountUid);
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::dateEq(const std::chrono::system_clock::time_point &time) {
            return std::make_shared<ConditionQueryFilter<std::string>>("date", "=", DateUtils::toJSON(time));
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::dateGt(const std::chrono::system_clock::time_point &time) {
            return std::make_shared<ConditionQueryFilter<std::string>>("date", ">", DateUtils::toJSON(time));
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::dateGte(const std::chrono::system_clock::time_point &time) {
            return std::make_shared<ConditionQueryFilter<std::string>>("date", ">=", DateUtils::toJSON(time));
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::dateNeq(const std::chrono::system_clock::time_point &time) {
            return std::make_shared<ConditionQueryFilter<std::string>>("date", "<>", DateUtils::toJSON(time));
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::dateLt(const std::chrono::system_clock::time_point &time) {
            return std::make_shared<ConditionQueryFilter<std::string>>("date", "<", DateUtils::toJSON(time));
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::dateLte(const std::chrono::system_clock::time_point &time) {
            return std::make_shared<ConditionQueryFilter<std::string>>("date", "<=", DateUtils::toJSON(time));
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::trustEq(TrustLevel trust) {
            auto string = api::to_string(trust);
            auto base64 = cereal::base64::encode((const unsigned char *)string.data(), string.size());
            return std::make_shared<ConditionQueryFilter<std::string>>("trust", "LIKE", fmt::format("%{}%", base64));
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::trustNeq(TrustLevel trust) {
            auto string = api::to_string(trust);
            auto base64 = cereal::base64::encode((const unsigned char *)string.data(), string.size());
            return std::make_shared<ConditionQueryFilter<std::string>>("trust", "NOT LIKE", fmt::format("%{}%", base64));
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::containsSender(const std::string &senderAddress) {
            return std::make_shared<ConditionQueryFilter<std::string>>("senders", "LIKE", fmt::format("%{}%", senderAddress));
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::containsRecipient(const std::string &recipientAddress) {
            return std::make_shared<ConditionQueryFilter<std::string>>("recipients", "LIKE", fmt::format("%{}%", recipientAddress));
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::currencyEq(const std::string &currencyName) {
            return std::make_shared<ConditionQueryFilter<std::string>>("currency_name", "=", currencyName);
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::operationUidEq(const std::string &operationUid) {
            return std::make_shared<ConditionQueryFilter<std::string>>("uid", "=", operationUid);
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::operationUidNeq(const std::string &operationUid) {
            return std::make_shared<ConditionQueryFilter<std::string>>("uid", "<>", operationUid);
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::feesEq(const std::shared_ptr<Amount> &amount) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("fees", "=", amount->toLong());
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::feesNeq(const std::shared_ptr<Amount> &amount) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("fees", "<>", amount->toLong());
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::feesGt(const std::shared_ptr<Amount> &amount) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("fees", ">", amount->toLong());
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::feesLt(const std::shared_ptr<Amount> &amount) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("fees", "<", amount->toLong());
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::feesGte(const std::shared_ptr<Amount> &amount) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("fees", ">=", amount->toLong());
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::feesLte(const std::shared_ptr<Amount> &amount) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("fees", "<=", amount->toLong());
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::amountEq(const std::shared_ptr<Amount> &amount) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("amount", "=", amount->toLong());
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::amountNeq(const std::shared_ptr<Amount> &amount) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("amount", "<>", amount->toLong());
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::amountGt(const std::shared_ptr<Amount> &amount) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("amount", ">", amount->toLong());
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::amountGte(const std::shared_ptr<Amount> &amount) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("amount", ">=", amount->toLong());
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::amountLt(const std::shared_ptr<Amount> &amount) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("amount", "<", amount->toLong());
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::amountLte(const std::shared_ptr<Amount> &amount) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("amount", "<=", amount->toLong());
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::blockHeightEq(int64_t blockHeight) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("block_height", "=", blockHeight);
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::blockHeightNeq(int64_t blockHeight) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("block_height", "<>", blockHeight);
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::blockHeightLt(int64_t blockHeight) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("block_height", "<", blockHeight);
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::blockHeightLte(int64_t blockHeight) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("block_height", "<=", blockHeight);
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::blockHeightGt(int64_t blockHeight) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("block_height", ">", blockHeight);
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::blockHeightGte(int64_t blockHeight) {
            return std::make_shared<ConditionQueryFilter<int64_t>>("block_height", ">=", blockHeight);
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::blockHeightIsNull() {
            return std::make_shared<PlainTextConditionQueryFilter>("block_height IS NULL");
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::operationTypeEq(api::OperationType type) {
            return std::make_shared<ConditionQueryFilter<std::string>>("type", "=", api::to_string(type));
        }

        std::shared_ptr<api::QueryFilter> api::QueryFilter::operationTypeNeq(api::
                                                                             OperationType type) {
            return std::make_shared<ConditionQueryFilter<std::string>>("type", "<>", api::to_string(type));
        }

    }
}