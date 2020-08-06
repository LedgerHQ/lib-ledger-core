/*
 *
 * StellarLikeOperation.cpp
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

#include "StellarLikeOperation.hpp"
#include "StellarLikeAddress.hpp"
#include <api/StellarLikeOperationType.hpp>
#include <wallet/common/Amount.h>
#include <wallet/common/AbstractAccount.hpp>
#include <wallet/stellar/xdr/models.hpp>
#include <wallet/stellar/StellarLikeMemo.hpp>
#include <wallet/stellar/xdr/StellarModelUtils.hpp>

namespace ledger {
    namespace core {

        StellarLikeOperation::StellarLikeOperation(const std::shared_ptr<OperationApi> &api) : _currency(api->getCurrency()) {
            // Create record from API backend
            const auto& operationWithTransaction = api->getBackend().stellarOperation.getValueOr(stellar::OperationWithParentTransaction());
            const auto& op = operationWithTransaction.operation;
            const auto& tx = operationWithTransaction.transaction;
            api::StellarLikeAsset asset(
                    op.asset.type,
                    op.asset.code.empty() ? Option<std::string>() : Option<std::string>(op.asset.code),
                    op.asset.issuer.empty() ? Option<std::string>() : Option<std::string>(op.asset.issuer)
            );
            auto sourceAsset = op.sourceAsset.map<api::StellarLikeAsset>([] (const auto& a) {
                return api::StellarLikeAsset(
                        a.type,
                        a.code.empty() ? Option<std::string>() : Option<std::string>(a.code),
                        a.issuer.empty() ? Option<std::string>() : Option<std::string>(a.issuer)
                );
            });
            std::shared_ptr<api::Amount> sourceAmount;
            if (op.sourceAmount.nonEmpty()) {
                sourceAmount = std::make_shared<Amount>(
                        api->getAccount()->getWallet()->getCurrency(), 0, op.sourceAmount.getValue());
            }
            _record = api::StellarLikeOperationRecord(
                    op.id, op.transactionSuccessful, static_cast<api::StellarLikeOperationType>(op.type),
                    op.transactionHash, asset, sourceAsset, sourceAmount
            );

            // Create the envelope object
            const auto& backend = api->getBackend().stellarOperation.getValue();
            _envelope.tx.sourceAccount = StellarLikeAddress(op.from, api->getCurrency(), Option<std::string>::NONE).toXdrMuxedAccount();
            _envelope.tx.seqNum = op.transactionSequence.toUint64();
            _envelope.tx.fee = op.transactionFee.toUnsignedInt();
            _envelope.tx.memo.type = stellar::xdr::MemoType::MEMO_NONE;
            // Rebuild MEMO
            auto memo = StellarLikeMemo::fromDatabase(tx.memoType, tx.memo);
            if (memo.isSuccess()) {
                _envelope.tx.memo = memo.getValue().getBackend();
            }
        }

        api::StellarLikeOperationRecord StellarLikeOperation::getRecord() {
           return _record;
        }

        std::shared_ptr<api::StellarLikeTransaction> StellarLikeOperation::getTransaction() {
            auto wrapped = stellar::xdr::wrap(_envelope);
            return std::make_shared<StellarLikeTransaction>(_currency, wrapped);
        }

    }
}