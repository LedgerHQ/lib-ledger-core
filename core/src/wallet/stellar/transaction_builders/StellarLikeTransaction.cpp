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
#include <crypto/SHA256.hpp>
#include <wallet/stellar/StellarLikeAddress.hpp>
#include <utils/Exception.hpp>

namespace ledger {
    namespace core {
        std::vector<uint8_t> StellarLikeTransaction::toRawTransaction() {
            stellar::xdr::Encoder encoder;
            encoder << _envelope;
            return encoder.toByteArray();
        }

        std::vector<uint8_t> StellarLikeTransaction::toSignatureBase() {
            auto networkId = SHA256::stringToBytesHash(_params.NetworkPassphrase);
            stellar::xdr::Encoder envTypeEncoder;
            envTypeEncoder << static_cast<int32_t>(stellar::xdr::EnvelopeType::ENVELOPE_TYPE_TX);
            auto encodedEnvType = envTypeEncoder.toByteArray();
            std::vector<uint8_t> signatureBase;
            stellar::xdr::Encoder txEncoder;
            txEncoder << _envelope.tx;
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
            std::copy(signature.begin(), )
            std::copy(pubKey.begin() + (pubKey.size() - 4), pubKey.end(), sig.hint.begin());
            _envelope.signatures.emplace_back(sig);
        }
    }
}


