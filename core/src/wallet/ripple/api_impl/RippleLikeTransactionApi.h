/*
 *
 * RippleLikeTransactionApi
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
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

#ifndef LEDGER_CORE_RIPPLELIKETRANSACTIONAPI_H
#define LEDGER_CORE_RIPPLELIKETRANSACTIONAPI_H

#include <api/Amount.hpp>
#include <api/Currency.hpp>
#include <api/RippleLikeMemo.hpp>
#include <api/RippleLikeTransaction.hpp>
#include <math/BigInt.h>
#include <wallet/common/api_impl/OperationApi.h>
#include <wallet/ripple/api_impl/RippleLikeBlockApi.h>

namespace ledger {
    namespace core {
        class RippleLikeTransactionApi : public api::RippleLikeTransaction {
          public:
            explicit RippleLikeTransactionApi(const api::Currency &currency);
            explicit RippleLikeTransactionApi(const std::shared_ptr<OperationApi> &operation);

            std::string getHash() override;
            std::shared_ptr<api::Amount> getFees() override;
            std::shared_ptr<api::RippleLikeAddress> getReceiver() override;
            std::shared_ptr<api::RippleLikeAddress> getSender() override;
            std::shared_ptr<api::Amount> getValue() override;
            std::vector<uint8_t> serialize() override;
            std::chrono::system_clock::time_point getDate() override;
            std::shared_ptr<api::BigInt> getSequence() override;
            std::shared_ptr<api::BigInt> getLedgerSequence() override;
            void setSignature(const std::vector<uint8_t> &rSignature, const std::vector<uint8_t> &sSignature) override;
            void setDERSignature(const std::vector<uint8_t> &signature) override;
            std::vector<uint8_t> getSigningPubKey() override;
            RippleLikeTransactionApi &setFees(const std::shared_ptr<BigInt> &fees);
            RippleLikeTransactionApi &setValue(const std::shared_ptr<BigInt> &value);
            RippleLikeTransactionApi &setSequence(const BigInt &sequence);
            RippleLikeTransactionApi &setLedgerSequence(const BigInt &ledgerSequence);
            RippleLikeTransactionApi &setSender(const std::shared_ptr<api::RippleLikeAddress> &sender);
            RippleLikeTransactionApi &setReceiver(const std::shared_ptr<api::RippleLikeAddress> &receiver);
            RippleLikeTransactionApi &setSigningPubKey(const std::vector<uint8_t> &pubKey);
            RippleLikeTransactionApi &setHash(const std::string &hash);
            RippleLikeTransactionApi &setDestinationTag(int64_t tag);
            std::vector<api::RippleLikeMemo> getMemos() override;
            void addMemo(api::RippleLikeMemo const &memo) override;
            std::experimental::optional<int64_t> getDestinationTag() override;
            int32_t getStatus() override;
            std::string getCorrelationId() override;
            std::string setCorrelationId(const std::string &newId) override;

          private:
            std::chrono::system_clock::time_point _time;
            std::shared_ptr<RippleLikeBlockApi> _block;
            std::string _hash;
            api::Currency _currency;
            std::shared_ptr<api::Amount> _fees;
            std::shared_ptr<api::Amount> _value;
            std::shared_ptr<api::BigInt> _sequence;
            std::shared_ptr<api::BigInt> _ledgerSequence;
            std::shared_ptr<api::RippleLikeAddress> _receiver;
            std::shared_ptr<api::RippleLikeAddress> _sender;
            std::vector<uint8_t> _rSignature;
            std::vector<uint8_t> _sSignature;
            std::vector<uint8_t> _signingPubKey;
            std::vector<api::RippleLikeMemo> _memos;
            Option<int64_t> _destinationTag;
            int32_t _status;
            std::string _correlationId;
        };
    } // namespace core
} // namespace ledger

#endif //LEDGER_CORE_RIPPLELIKETRANSACTIONAPI_H
