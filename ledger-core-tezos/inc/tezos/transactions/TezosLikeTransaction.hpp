/*
 *
 * TezosLikeTransaction
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

#pragma once

#include <tezos/TezosLikeBlock.hpp>
#include <tezos/api/TezosLikeTransaction.hpp>
#include <tezos/api/TezosOperationTag.hpp>
#include <tezos/api/TezosCurve.hpp>
#include <tezos/operations/TezosLikeOperation.hpp>
#include <tezos/explorers/TezosLikeBlockchainExplorer.hpp>

#include <core/api/Amount.hpp>
#include <core/api/Currency.hpp>
#include <core/math/BigInt.hpp>
#include <core/operation/Operation.hpp>
#include <core/operation/OperationQuery.hpp>

namespace ledger {
    namespace core {
        // Reference: https://github.com/obsidiansystems/ledger-app-tezos/blob/9a0c8cc546677147b93935e0b0c96925244baf64/src/types.h
        class TezosLikeTransaction : public api::TezosLikeTransaction {
        public:
            explicit TezosLikeTransaction(const api::Currency &currency,
                                          const std::string &protocolUpdate);

            explicit TezosLikeTransaction(
                const TezosLikeBlockchainExplorerTransaction& tx,
                const api::Currency &currency,
                const std::string &protocolUpdate);

            api::TezosOperationTag getType() override;

            std::string getHash() override;

            std::shared_ptr<api::Amount> getFees() override;

            std::shared_ptr<api::TezosLikeAddress> getReceiver() override;

            std::shared_ptr<api::TezosLikeAddress> getSender() override;

            std::shared_ptr<api::Amount> getValue() override;

            std::vector<uint8_t> serialize() override;
            std::vector<uint8_t> serializeWithType(api::TezosOperationTag type);

            std::chrono::system_clock::time_point getDate() override;

            std::shared_ptr<api::BigInt> getCounter() override;

            std::shared_ptr<api::Amount> getGasLimit() override ;

            std::shared_ptr<api::BigInt> getStorageLimit() override;

            std::experimental::optional<std::string> getBlockHash() override;

            void setSignature(const std::vector<uint8_t> &signature) override;

            std::vector<uint8_t> getSigningPubKey() override;

            int32_t getStatus() override;

            TezosLikeTransaction &setFees(const std::shared_ptr<BigInt> &fees);

            TezosLikeTransaction &setValue(const std::shared_ptr<BigInt> &value);

            TezosLikeTransaction &setSender(const std::shared_ptr<api::TezosLikeAddress> &sender, api::TezosCurve curve = api::TezosCurve::ED25519);

            TezosLikeTransaction &setReceiver(const std::shared_ptr<api::TezosLikeAddress> &receiver, api::TezosCurve curve = api::TezosCurve::ED25519);

            TezosLikeTransaction &setSigningPubKey(const std::vector<uint8_t> &pubKey);

            TezosLikeTransaction &setHash(const std::string &hash);

            TezosLikeTransaction &setBlockHash(const std::string &blockHash);

            TezosLikeTransaction & setGasLimit(const std::shared_ptr<BigInt>& gasLimit);
            
            TezosLikeTransaction & setCounter(const std::shared_ptr<BigInt>& counter);

            TezosLikeTransaction & setStorage(const std::shared_ptr<BigInt>& storage);

            TezosLikeTransaction & setType(api::TezosOperationTag type);

            TezosLikeTransaction & setBalance(const BigInt &balance);

            TezosLikeTransaction & setManagerAddress(const std::string &managerAddress);
            std::string getManagerAddress() const;

            TezosLikeTransaction &setRawTx(const std::vector<uint8_t> &rawTx);

            TezosLikeTransaction &reveal(bool needReveal);
            bool toReveal() const;
        private:
            std::chrono::system_clock::time_point _time;
            std::shared_ptr<TezosLikeBlock> _block;
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
            std::vector<uint8_t> _rawTx;
            bool _needReveal;
            int32_t _status;
        };
    }
}