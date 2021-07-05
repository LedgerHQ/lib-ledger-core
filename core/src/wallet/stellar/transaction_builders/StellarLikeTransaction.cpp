/*
 *
 * StellarLikeTransaction.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 28/08/2019.
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

#include "StellarLikeTransaction.hpp"
#include <api/ErrorCode.hpp>
#include <wallet/stellar/xdr/XDREncoder.hpp>
#include <wallet/stellar/xdr/XDRDecoder.hpp>
#include <crypto/SHA256.hpp>
#include <wallet/stellar/StellarLikeAddress.hpp>
#include <utils/Exception.hpp>
#include <wallet/common/Amount.h>
#include <api_impl/BigIntImpl.hpp>
#include <wallet/stellar/StellarLikeMemo.hpp>
#include <wallet/stellar/xdr/StellarModelUtils.hpp>

namespace ledger {
    namespace core {

        std::shared_ptr<api::StellarLikeTransaction>
        StellarLikeTransaction::parseRawTransaction(const api::Currency & currency,
                                                    const std::vector<uint8_t> & rawTransaction)
        {
            stellar::xdr::Decoder decoder(rawTransaction);
            stellar::xdr::TransactionEnvelope envelope;
            decoder >> envelope;
            return std::make_shared<StellarLikeTransaction>(currency, envelope);
        }

        std::shared_ptr<api::StellarLikeTransaction>
        StellarLikeTransaction::parseSignatureBase(const api::Currency & currency,
                                                   const std::vector<uint8_t> & signatureBase)
        {
            auto networkId = SHA256::stringToBytesHash(currency.stellarLikeNetworkParameters.value().NetworkPassphrase);

            stellar::xdr::Encoder envTypeEncoder;
            envTypeEncoder << static_cast<int32_t>(stellar::xdr::EnvelopeType::ENVELOPE_TYPE_TX);
            auto encodedEnvType = envTypeEncoder.toByteArray();

            auto offset = networkId.size() + encodedEnvType.size();
            auto rawTransaction = std::vector<uint8_t>(signatureBase.begin() + offset, signatureBase.end());

            stellar::xdr::Transaction tx;
            stellar::xdr::Decoder txDecoder(rawTransaction);
            txDecoder >> tx;

            stellar::xdr::TransactionV1Envelope preEnvelope;
            preEnvelope.tx = tx;
            stellar::xdr::TransactionEnvelope envelope = stellar::xdr::wrap(preEnvelope);

            return std::make_shared<StellarLikeTransaction>(currency, envelope);
        }

        std::vector<uint8_t> StellarLikeTransaction::toRawTransaction() {
            stellar::xdr::Encoder encoder;
            encoder << _envelope;
            return encoder.toByteArray();
        }

        std::vector<uint8_t> StellarLikeTransaction::toSignatureBase() {
            auto networkId = SHA256::stringToBytesHash(_currency.stellarLikeNetworkParameters.value().NetworkPassphrase);
            stellar::xdr::Encoder envTypeEncoder;
            envTypeEncoder << static_cast<int32_t>(stellar::xdr::EnvelopeType::ENVELOPE_TYPE_TX);
            auto encodedEnvType = envTypeEncoder.toByteArray();
            std::vector<uint8_t> signatureBase;
            stellar::xdr::Encoder txEncoder;
            if (isEnvelopeV0()) {
               auto content = getEnvelopeV0();
                txEncoder << content.tx;
            } else {
                auto content = getEnvelopeV1();
                txEncoder << content.tx;
            }
            auto encodedTx = txEncoder.toByteArray();

            signatureBase.insert(signatureBase.end(), networkId.begin(), networkId.end());
            signatureBase.insert(signatureBase.end(), encodedEnvType.begin(), encodedEnvType.end());
            signatureBase.insert(signatureBase.end(), encodedTx.begin(), encodedTx.end());

            return signatureBase;
        }

        void StellarLikeTransaction::putSignature(const std::vector<uint8_t> &signature,
                                                  const std::shared_ptr<api::Address>& address) {
            auto stellarAddress = std::dynamic_pointer_cast<StellarLikeAddress>(address);
            if (!stellarAddress)
                throw make_exception(api::ErrorCode::ILLEGAL_ARGUMENT, "putTransaction can only handle stellar addresses");
            // At this point pub key is 32 bytes
            auto pubKey = stellarAddress->toPublicKey();
            stellar::xdr::DecoratedSignature sig;
            sig.signature = signature;
            std::copy(pubKey.begin() + (pubKey.size() - 4), pubKey.end(), sig.hint.begin());

            if (isEnvelopeV0()) {
                getEnvelopeV0().signatures.emplace_back(sig);
            } else {
                getEnvelopeV1().signatures.emplace_back(sig);
            }
        }

        std::shared_ptr<api::Address> StellarLikeTransaction::getSourceAccount() {
            std::string address;
            if (isEnvelopeV0()) {
                const auto& envelope = getEnvelopeV0();
                std::vector<uint8_t> pubKey(envelope.tx.sourceAccountEd25519.begin(), envelope.tx.sourceAccountEd25519.end());
                address = StellarLikeAddress::convertPubkeyToAddress(
                        pubKey, Option<uint64_t>::NONE,
                        _currency.stellarLikeNetworkParameters.value());
            } else {
                const auto& envelope = getEnvelopeV1();
                address = StellarLikeAddress::convertMuxedAccountToAddress(
                        envelope.tx.sourceAccount,
                        _currency.stellarLikeNetworkParameters.value());
            }

            return std::make_shared<StellarLikeAddress>(address, _currency, Option<std::string>());
        }

        std::shared_ptr<api::BigInt> StellarLikeTransaction::getSourceAccountSequence() {
            if (isEnvelopeV0()) {
                return std::make_shared<api::BigIntImpl>(BigInt(getEnvelopeV0().tx.seqNum));
            }
            return std::make_shared<api::BigIntImpl>(BigInt(getEnvelopeV1().tx.seqNum));
        }

        std::shared_ptr<api::Amount> StellarLikeTransaction::getFee() {
            if (isEnvelopeV0()) {
                return std::make_shared<Amount>(_currency, 0, BigInt(getEnvelopeV0().tx.fee));
            }
            return std::make_shared<Amount>(_currency, 0, BigInt(getEnvelopeV1().tx.fee));
        }

        std::shared_ptr<api::StellarLikeMemo> StellarLikeTransaction::getMemo() {
            if (isEnvelopeV0()) {
                return std::make_shared<StellarLikeMemo>(getEnvelopeV0().tx.memo);
            }
            return std::make_shared<StellarLikeMemo>(getEnvelopeV1().tx.memo);
        }

        const stellar::xdr::TransactionV0Envelope &StellarLikeTransaction::getEnvelopeV0() const {
            if (!isEnvelopeV0())
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Envelope type must be V0");
            return boost::get<stellar::xdr::TransactionV0Envelope>(_envelope.content);
        }

        const stellar::xdr::TransactionV1Envelope &StellarLikeTransaction::getEnvelopeV1() const {
            if (!isEnvelopeV1())
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Envelope type must be V1");
            return boost::get<stellar::xdr::TransactionV1Envelope>(_envelope.content);
        }

        stellar::xdr::TransactionV0Envelope &StellarLikeTransaction::getEnvelopeV0() {
            if (!isEnvelopeV0())
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Envelope type must be V0");
            return boost::get<stellar::xdr::TransactionV0Envelope>(_envelope.content);
        }

        stellar::xdr::TransactionV1Envelope &StellarLikeTransaction::getEnvelopeV1() {
            if (!isEnvelopeV1())
                throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Envelope type must be V1");
            return boost::get<stellar::xdr::TransactionV1Envelope>(_envelope.content);
        }

        bool StellarLikeTransaction::isEnvelopeV1() const {
            return _envelope.type == stellar::xdr::EnvelopeType::ENVELOPE_TYPE_TX;
        }

        bool StellarLikeTransaction::isEnvelopeV0() const {
            return _envelope.type == stellar::xdr::EnvelopeType::ENVELOPE_TYPE_TX_V0;
        }

        std::string StellarLikeTransaction::getCorrelationId() {
            return _correlationId;
        }

        std::string StellarLikeTransaction::setCorrelationId(const std::string& newId)  {
            auto oldId = _correlationId;
            _correlationId = newId;
            return oldId;
        }

    }
}


