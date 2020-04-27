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
                        BigInt(boost::get<uint64_t>(_memo.content))
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
    }
}