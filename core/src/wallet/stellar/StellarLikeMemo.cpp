/*
 *
 * StellarLikeMemo.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/04/2020.
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

#include "StellarLikeMemo.hpp"
#include <api/StellarLikeMemoType.hpp>
#include <fmt/format.h>
#include <utils/hex.h>
#include <utils/Exception.hpp>
#include <api_impl/BigIntImpl.hpp>
#include <math/BaseConverter.hpp>

namespace ledger {
    namespace core {

        api::StellarLikeMemoType StellarLikeMemo::getMemoType() {
            switch (_memo.type) {
                case stellar::xdr::MemoType::MEMO_NONE:
                    return api::StellarLikeMemoType::MEMO_NONE;
                case stellar::xdr::MemoType::MEMO_TEXT:
                    return api::StellarLikeMemoType::MEMO_TEXT;
                case stellar::xdr::MemoType::MEMO_ID:
                    return api::StellarLikeMemoType::MEMO_ID;
                case stellar::xdr::MemoType::MEMO_HASH:
                    return api::StellarLikeMemoType::MEMO_HASH;
                case stellar::xdr::MemoType::MEMO_RETURN:
                    return api::StellarLikeMemoType::MEMO_RETURN;
            }
        }

        std::string StellarLikeMemo::getMemoText() {
            if (_memo.type == stellar::xdr::MemoType::MEMO_TEXT)
                return boost::get<std::string>(_memo.content);
            throw make_exception(api::ErrorCode::INVALID_STELLAR_MEMO_TYPE, "Memo type is not MEMO_ID");
        }

        std::shared_ptr<api::BigInt> StellarLikeMemo::getMemoId() {
            if (_memo.type == stellar::xdr::MemoType::MEMO_ID)
                return std::make_shared<api::BigIntImpl>(
                        BigInt::fromScalar(boost::get<uint64_t>(_memo.content))
                );
            throw make_exception(api::ErrorCode::INVALID_STELLAR_MEMO_TYPE, "Memo type is not MEMO_ID");
        }

        std::vector<uint8_t> StellarLikeMemo::getMemoHash() {
            if (_memo.type == stellar::xdr::MemoType::MEMO_HASH)
                return std::vector<uint8_t>(
                        boost::get<stellar::xdr::Hash>(_memo.content).begin(),
                        boost::get<stellar::xdr::Hash>(_memo.content).end());
            throw make_exception(api::ErrorCode::INVALID_STELLAR_MEMO_TYPE, "Memo type is not MEMO_HASH");
        }

        std::vector<uint8_t> StellarLikeMemo::getMemoReturn() {
            if (_memo.type == stellar::xdr::MemoType::MEMO_RETURN)
                return std::vector<uint8_t>(
                        boost::get<stellar::xdr::Hash>(_memo.content).begin(),
                        boost::get<stellar::xdr::Hash>(_memo.content).end());
            throw make_exception(api::ErrorCode::INVALID_STELLAR_MEMO_TYPE, "Memo type is not MEMO_RETURN");
        }

        std::string StellarLikeMemo::memoValuetoString() {
            switch (_memo.type) {
                case stellar::xdr::MemoType::MEMO_NONE:
                    return std::string();
                case stellar::xdr::MemoType::MEMO_TEXT:
                    return boost::get<std::string>(_memo.content);
                case stellar::xdr::MemoType::MEMO_ID:
                    return fmt::format("{}", boost::get<uint64_t>(_memo.content));
                case stellar::xdr::MemoType::MEMO_HASH:
                case stellar::xdr::MemoType::MEMO_RETURN:
                    return hex::toString(std::vector<uint8_t>(
                            boost::get<stellar::xdr::Hash>(_memo.content).begin(),
                            boost::get<stellar::xdr::Hash>(_memo.content).end())
                    );
            }
        }

        const stellar::xdr::Memo &StellarLikeMemo::getBackend() const {
            return _memo;
        }

        Try<StellarLikeMemo> StellarLikeMemo::fromDatabase(const std::string &type, const std::string &content) {
            stellar::xdr::Memo memo;
            Try<StellarLikeMemo> result;

            if (type == "text") {
                memo.type = stellar::xdr::MemoType::MEMO_TEXT;
                memo.content = content;
                result.success(memo);
            } else if (type == "none") {
                memo.type = stellar::xdr::MemoType::MEMO_NONE;
                result.success(memo);
            } else if (type == "id") {
                memo.type = stellar::xdr::MemoType::MEMO_ID;
                memo.content = BigInt::fromString(content).toUint64();
                result.success(memo);
            } else if (type == "return" || type == "hash") {
                memo.type = type == "return" ?
                        stellar::xdr::MemoType::MEMO_RETURN : stellar::xdr::MemoType::MEMO_HASH;
                stellar::xdr::Hash hash;
                std::vector<uint8_t> hashVector;
                BaseConverter::decode(content, BaseConverter::BASE64_RFC4648, hashVector);
                if (hashVector.size() == hash.max_size()) {
                    std::copy(hashVector.begin(), hashVector.end(), hash.begin());
                    memo.content = hash;
                    result.success(memo);
                } else {
                    result.fail(api::ErrorCode::ILLEGAL_ARGUMENT, "Unable to rebuild memo from database (bad content)");
                }
            } else {
                result.fail(api::ErrorCode::ILLEGAL_ARGUMENT, "Unable to rebuild memo from database (unknown type)");
            }
            return result;
        }
    }
}