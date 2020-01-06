/*
 *
 * StellarLikeTransactionBuilder.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 27/08/2019.
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

#include "StellarLikeTransactionBuilder.hpp"
#include <wallet/stellar/StellarLikeAccount.hpp>
#include <api/StellarLikeTransactionCallback.hpp>
#include <algorithm>
#include <api_impl/BigIntImpl.hpp>

namespace ledger {
    namespace core {

        StellarLikeTransactionBuilder::StellarLikeTransactionBuilder(
                const std::shared_ptr<StellarLikeAccount> &account) : _account(account) {
            // Initialize everything with default values
            _envelope.tx.memo.type = stellar::xdr::MemoType::MEMO_NONE;
        }

        std::shared_ptr<api::StellarLikeTransactionBuilder>
        StellarLikeTransactionBuilder::addNativePayment(const std::string &address,
                                                        const std::shared_ptr<api::Amount> &amount) {
            stellar::xdr::Operation operation;
            stellar::xdr::PaymentOp op;
            op.destination.type = stellar::xdr::PublicKeyType::PUBLIC_KEY_TYPE_ED25519;
            StellarLikeAddress addr(address, _account->getWallet()->getCurrency(), Option<std::string>::NONE);
            auto pubKey = addr.toPublicKey();
            if (pubKey.size() != op.destination.content.max_size())
                throw make_exception(api::ErrorCode::ILLEGAL_STATE, "Public key must be exactly {}", op.destination.content.max_size());
            std::copy(pubKey.begin(), pubKey.begin() + op.destination.content.max_size(), op.destination.content.begin());
            op.asset.type = stellar::xdr::AssetType::ASSET_TYPE_NATIVE;
            op.amount = std::static_pointer_cast<Amount>(amount)->value()->toInt64();

            operation.type = stellar::OperationType::PAYMENT;
            operation.content = op;
            _envelope.tx.operations.emplace_back(operation);

            _balanceChange = _balanceChange + BigInt(op.amount);

            return shared_from_this();
        }

        std::shared_ptr<api::StellarLikeTransactionBuilder>
        StellarLikeTransactionBuilder::addCreateAccount(const std::string &address,
                                                        const std::shared_ptr<api::Amount> &amount) {
            stellar::xdr::Operation operation;
            stellar::xdr::CreateAccountOp op;
            op.destination.type = stellar::xdr::PublicKeyType::PUBLIC_KEY_TYPE_ED25519;
            StellarLikeAddress addr(address, _account->getWallet()->getCurrency(), Option<std::string>::NONE);
            auto pubKey = addr.toPublicKey();
            if (pubKey.size() != op.destination.content.max_size())
                throw make_exception(api::ErrorCode::ILLEGAL_STATE, "Public key must be exactly {}", op.destination.content.max_size());
            std::copy(pubKey.begin(), pubKey.end(), op.destination.content.begin());
            op.startingBalance = std::static_pointer_cast<Amount>(amount)->value()->toInt64();

            operation.type = stellar::OperationType::CREATE_ACCOUNT;
            operation.content = op;
            _envelope.tx.operations.emplace_back(operation);

            _balanceChange = _balanceChange + BigInt(op.startingBalance);

            return shared_from_this();
        }

        std::shared_ptr<api::StellarLikeTransactionBuilder>
        StellarLikeTransactionBuilder::setBaseFee(const std::shared_ptr<api::Amount> &baseFee) {
            _baseFee = static_cast<uint64_t>(baseFee->toLong());
            return shared_from_this();
        }

        void
        StellarLikeTransactionBuilder::build(const std::shared_ptr<api::StellarLikeTransactionCallback> &callback) {
           build().callback(_account->getContext(), callback);
        }

        FuturePtr<ledger::core::api::StellarLikeTransaction> StellarLikeTransactionBuilder::build() {
            // Check if balance is OK
            // Get account pub key
            // Sanitize signatures
            auto account = _account;
            auto envelope = _envelope;
            auto pubKey = _account->params().keychain->getAddress()->toPublicKey();
            if (pubKey.size() != 32)
                throw make_exception(api::ErrorCode::ILLEGAL_STATE, "Public key must be exactly 32 byte long");
            auto balanceChange = _balanceChange;
            auto baseFee = _baseFee.getValueOr(100);
            return account->getBalance().flatMap<Unit>(account->getContext(), [=] (const std::shared_ptr<Amount>& balance) {
                return account->getBaseReserve().map<Unit>(account->getContext(), [=] (const std::shared_ptr<Amount> &reserve) -> Unit {
                    auto newBalance = *balance->value() - balanceChange;
                    if (newBalance <= *reserve->value())
                        throw make_exception(api::ErrorCode::NOT_ENOUGH_FUNDS, "New balance ({}) is below base reserve ({})", newBalance.toInt64(), reserve->toLong());
                    return unit;
                });
            }).mapPtr<api::StellarLikeTransaction>(account->getContext(), [=] (const Unit&) mutable -> std::shared_ptr<api::StellarLikeTransaction> {
                envelope.tx.sourceAccount.type = stellar::xdr::PublicKeyType::PUBLIC_KEY_TYPE_ED25519;
                std::copy(pubKey.begin(), pubKey.end(), envelope.tx.sourceAccount.content.begin());
                for (auto& signature : envelope.signatures) {
                    std::copy(pubKey.begin() + 28,  pubKey.end(), signature.hint.begin());
                }
                envelope.tx.fee = envelope.tx.operations.size() * baseFee;
                return std::make_shared<StellarLikeTransaction>(_account->getWallet()->getCurrency().stellarLikeNetworkParameters.value(), envelope);
            });
        }

        std::shared_ptr<api::StellarLikeTransactionBuilder>
        StellarLikeTransactionBuilder::setTextMemo(const std::string &text) {
            _envelope.tx.memo.type = stellar::xdr::MemoType::MEMO_TEXT;
            _envelope.tx.memo.content = text;
            return shared_from_this();
        }

        std::shared_ptr<api::StellarLikeTransactionBuilder>
        StellarLikeTransactionBuilder::setHashMemo(const std::vector<uint8_t> &hash) {
            _envelope.tx.memo.type = stellar::xdr::MemoType::MEMO_HASH;
            if (hash.size() != 32) {
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "setHashMemo expects the length of the hash to be exactly 32");
            }
            stellar::xdr::uint256 data;
            std::copy(hash.begin(), hash.end(), data.begin());
            _envelope.tx.memo.content = data;
            return shared_from_this();
        }

        std::shared_ptr<api::StellarLikeTransactionBuilder>
        StellarLikeTransactionBuilder::setReturnMemo(const std::vector<uint8_t> &value) {
            _envelope.tx.memo.type = stellar::xdr::MemoType::MEMO_RETURN;
            stellar::xdr::uint256 data;
            if (value.size() != 32) {
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "setReturnMemo expects the length of the value to be exactly 32");
            }
            std::copy(value.begin(), value.end(), data.begin());
            _envelope.tx.memo.content = data;
            return shared_from_this();
        }

        std::shared_ptr<api::StellarLikeTransactionBuilder>
        StellarLikeTransactionBuilder::setNumberMemo(const std::shared_ptr<api::BigInt> &number) {
            _envelope.tx.memo.type = stellar::xdr::MemoType::MEMO_ID;
            _envelope.tx.memo.content = std::static_pointer_cast<api::BigIntImpl>(number)->backend().toUint64();
            return shared_from_this();;
        }

        std::shared_ptr<api::StellarLikeTransactionBuilder>
        StellarLikeTransactionBuilder::setSequence(const std::shared_ptr<api::BigInt> &sequence) {
            _envelope.tx.seqNum = std::static_pointer_cast<api::BigIntImpl>(sequence)->backend().toInt64();
            return shared_from_this();
        }

    }
}