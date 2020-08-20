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
#include <api/TezosOperationTag.hpp>
namespace ledger {
    namespace core {
        // Reference: https://github.com/obsidiansystems/ledger-app-tezos/blob/9a0c8cc546677147b93935e0b0c96925244baf64/src/types.h
        class TezosLikeTransactionApi : public api::TezosLikeTransaction {
        public:
            explicit TezosLikeTransactionApi(const api::Currency &currency,
                                             const std::string &protocolUpdate);

            explicit TezosLikeTransactionApi(const std::shared_ptr<OperationApi> &operation,
                                             const std::string &protocolUpdate);

            api::TezosOperationTag getType() override;

            std::string getHash() override;

            std::shared_ptr<api::Amount> getFees() override;

            std::shared_ptr<api::TezosLikeAddress> getReceiver() override;

            std::shared_ptr<api::TezosLikeAddress> getSender() override;

            std::shared_ptr<api::Amount> getValue() override;

            std::vector<uint8_t> serialize() override;
            std::vector<uint8_t> serializeWithType(api::TezosOperationTag type);

            /// Serialize the transaction as json for Tezos Node run_operation JSON RPC endpoint
            std::vector<uint8_t> serializeForDryRun(const std::vector<uint8_t>& chainID);

            /// Serialize the transaction as json for Tezos Node run_operation JSON RPC endpoint
            std::string serializeJsonForDryRun(const std::string& chainID);

            std::chrono::system_clock::time_point getDate() override;

            std::shared_ptr<api::BigInt> getCounter() override;

            std::shared_ptr<api::Amount> getGasLimit() override ;

            std::shared_ptr<api::BigInt> getStorageLimit() override;

            std::experimental::optional<std::string> getBlockHash() override;

            void setSignature(const std::vector<uint8_t> &signature) override;

            std::vector<uint8_t> getSigningPubKey() override;

            int32_t getStatus() override;

            TezosLikeTransactionApi &setFees(const std::shared_ptr<BigInt> &fees);

            TezosLikeTransactionApi &setValue(const std::shared_ptr<BigInt> &value);

            TezosLikeTransactionApi &setSender(const std::shared_ptr<api::TezosLikeAddress> &sender, api::TezosCurve curve = api::TezosCurve::ED25519);

            TezosLikeTransactionApi &setReceiver(const std::shared_ptr<api::TezosLikeAddress> &receiver, api::TezosCurve curve = api::TezosCurve::ED25519);

            TezosLikeTransactionApi &setSigningPubKey(const std::vector<uint8_t> &pubKey);

            TezosLikeTransactionApi &setHash(const std::string &hash);

            TezosLikeTransactionApi &setBlockHash(const std::string &blockHash);

            TezosLikeTransactionApi & setGasLimit(const std::shared_ptr<BigInt>& gasLimit);
            
            TezosLikeTransactionApi & setCounter(const std::shared_ptr<BigInt>& counter);

            TezosLikeTransactionApi & setStorage(const std::shared_ptr<BigInt>& storage);

            TezosLikeTransactionApi & setType(api::TezosOperationTag type);

            TezosLikeTransactionApi & setBalance(const BigInt &balance);

            TezosLikeTransactionApi & setManagerAddress(const std::string &managerAddress, api::TezosCurve curve);
            std::string getManagerAddress() const;

            TezosLikeTransactionApi &setRawTx(const std::vector<uint8_t> &rawTx);

            TezosLikeTransactionApi &reveal(bool needReveal);
            bool toReveal() const;
        private:
            std::chrono::system_clock::time_point _time;
            std::shared_ptr<TezosLikeBlockApi> _block;
            std::string _hash;
            api::Currency _currency;
            std::shared_ptr<api::Amount> _fees;
            std::shared_ptr<api::Amount> _gasLimit;
            std::shared_ptr<BigInt> _storage;
            std::shared_ptr<BigInt> _counter;
            std::shared_ptr<api::Amount> _value;
            std::shared_ptr<api::TezosLikeAddress> _receiver;
            api::TezosCurve _receiverCurve;
            std::shared_ptr<api::TezosLikeAddress> _sender;
            api::TezosCurve _senderCurve;
            std::vector<uint8_t> _signature;
            std::vector<uint8_t> _signingPubKey;
            api::TezosOperationTag _type;
            std::string _revealedPubKey;
            BigInt _balance;
            std::string _protocolUpdate;
            std::string _managerAddress;
            api::TezosCurve _managerCurve;
            std::vector<uint8_t> _rawTx;
            bool _needReveal;
            int32_t _status;
        };
    }
}
#endif //LEDGER_CORE_TEZOSLIKETRANSACTIONAPI_H
