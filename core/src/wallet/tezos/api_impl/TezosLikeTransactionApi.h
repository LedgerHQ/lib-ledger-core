/*
 *
 * TezosLikeTransactionApi
 *
 * Created by El Khalil Bellakrid on 27/04/2019.
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


#ifndef LEDGER_CORE_TEZOSLIKETRANSACTIONAPI_H
#define LEDGER_CORE_TEZOSLIKETRANSACTIONAPI_H

#include <wallet/common/api_impl/OperationApi.h>
#include <wallet/tezos/api_impl/TezosLikeBlockApi.h>

#include <api/TezosLikeTransaction.hpp>
#include <api/Amount.hpp>
#include <api/Currency.hpp>
#include <math/BigInt.h>

namespace ledger {
    namespace core {
        // Reference: https://github.com/obsidiansystems/ledger-app-tezos/blob/9a0c8cc546677147b93935e0b0c96925244baf64/src/types.h
        enum OperationTag {
            OPERATION_TAG_NONE = -1,
            OPERATION_TAG_GENERIC = 3,
            OPERATION_TAG_PROPOSAL = 5,
            OPERATION_TAG_BALLOT = 6,
            OPERATION_TAG_REVEAL = 7,
            OPERATION_TAG_TRANSACTION = 8,
            OPERATION_TAG_ORIGINATION = 9,
            OPERATION_TAG_DELEGATION = 10,
        };
        enum CurveTag {
            Ed25519,
            Secp256k1,
            P256,
        };
        class TezosLikeTransactionApi : public api::TezosLikeTransaction {
        public:
            explicit TezosLikeTransactionApi(const api::Currency &currency);

            explicit TezosLikeTransactionApi(const std::shared_ptr<OperationApi> &operation);

            std::string getHash() override;

            std::shared_ptr<api::Amount> getFees() override;

            std::shared_ptr<api::TezosLikeAddress> getReceiver() override;

            std::shared_ptr<api::TezosLikeAddress> getSender() override;

            std::shared_ptr<api::Amount> getValue() override;

            std::vector<uint8_t> serialize() override;

            std::chrono::system_clock::time_point getDate() override;

            std::shared_ptr<api::BigInt> getCounter() override;

            std::shared_ptr<api::Amount> getGasLimit() override ;

            std::shared_ptr<api::Amount> getStorageLimit() override;

            std::experimental::optional<std::string> getBlockHash() override;

            void setSignature(const std::vector<uint8_t> &rSignature, const std::vector<uint8_t> &sSignature) override;

            void setDERSignature(const std::vector<uint8_t> &signature) override;

            std::vector<uint8_t> getSigningPubKey() override;

            TezosLikeTransactionApi &setFees(const std::shared_ptr<BigInt> &fees);

            TezosLikeTransactionApi &setValue(const std::shared_ptr<BigInt> &value);

            TezosLikeTransactionApi &setSender(const std::shared_ptr<api::TezosLikeAddress> &sender);

            TezosLikeTransactionApi &setReceiver(const std::shared_ptr<api::TezosLikeAddress> &receiver);

            TezosLikeTransactionApi &setSigningPubKey(const std::vector<uint8_t> &pubKey);

            TezosLikeTransactionApi &setHash(const std::string &hash);

            TezosLikeTransactionApi &setBlockHash(const std::string &blockHash);

            TezosLikeTransactionApi & setGasLimit(const std::shared_ptr<BigInt>& gasLimit);
            
            TezosLikeTransactionApi & setCounter(const std::shared_ptr<BigInt>& counter);

            TezosLikeTransactionApi & setStorage(const std::shared_ptr<BigInt>& storage);

        private:
            std::chrono::system_clock::time_point _time;
            std::shared_ptr<TezosLikeBlockApi> _block;
            std::string _hash;
            api::Currency _currency;
            std::shared_ptr<api::Amount> _fees;
            std::shared_ptr<api::Amount> _gasLimit;
            std::shared_ptr<api::Amount> _storage;
            std::shared_ptr<BigInt> _counter;
            std::shared_ptr<api::Amount> _value;
            std::shared_ptr<api::TezosLikeAddress> _receiver;
            std::shared_ptr<api::TezosLikeAddress> _sender;
            std::vector<uint8_t> _rSignature;
            std::vector<uint8_t> _sSignature;
            std::vector<uint8_t> _signingPubKey;
        };
    }
}
#endif //LEDGER_CORE_TEZOSLIKETRANSACTIONAPI_H
