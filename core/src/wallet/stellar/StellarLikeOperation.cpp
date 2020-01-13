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

namespace ledger {
    namespace core {

        api::StellarLikeOperationRecord StellarLikeOperation::getRecord() {
            const auto& op = _api->getBackend().stellarOperation.getValueOr(stellar::Operation());
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
                        _api->getAccount()->getWallet()->getCurrency(), 0, op.sourceAmount.getValue());
            }
            return api::StellarLikeOperationRecord(
                        op.id, op.transactionSuccessful, (api::StellarLikeOperationType) op.type,
                        op.transactionHash, asset, sourceAsset, sourceAmount
                    );
        }

        std::shared_ptr<api::StellarLikeTransaction> StellarLikeOperation::getTransaction() {
            stellar::xdr::TransactionEnvelope envelope;
            const auto& backend = _api->getBackend().stellarOperation.getValue();
            envelope.tx.sourceAccount = StellarLikeAddress(backend.from, _api->getCurrency(), Option<std::string>::NONE).toXdrPublicKey();
            envelope.tx.seqNum = backend.transactionSequence.toUint64();
            envelope.tx.fee = backend.transactionFee.toUnsignedInt();
            return std::make_shared<StellarLikeTransaction>(_api->getCurrency(), std::move(envelope));
        }
    }
}