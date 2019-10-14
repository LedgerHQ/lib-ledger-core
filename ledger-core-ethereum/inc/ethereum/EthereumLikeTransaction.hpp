/*
 *
 * EthereumLikeTransaction
 *
 * Created by El Khalil Bellakrid on 12/07/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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

#include <core/api/Amount.hpp>
#include <core/api/Currency.hpp>
#include <core/math/BigInt.hpp>
#include <core/operation/Operation.hpp>

#include <ethereum/EthereumLikeBlock.hpp>
#include <ethereum/api/EthereumLikeTransaction.hpp>
#include <ethereum/explorers/EthereumLikeBlockchainExplorer.hpp>

namespace ledger {
    namespace core {
        class EthereumLikeTransaction : public api::EthereumLikeTransaction {
        public:
            explicit EthereumLikeTransaction(const api::Currency& currency);
            explicit EthereumLikeTransaction(
                const EthereumLikeBlockchainExplorerTransaction& tx,
                const api::Currency& currency);
            
            std::string getHash() override;
            int32_t getNonce() override;
            std::shared_ptr<api::Amount> getGasPrice() override ;
            std::shared_ptr<api::Amount> getGasLimit() override ;
            std::shared_ptr<api::Amount> getGasUsed() override ;
            std::shared_ptr<api::EthereumLikeAddress> getReceiver() override ;
            std::shared_ptr<api::EthereumLikeAddress> getSender() override;
            std::shared_ptr<api::Amount> getValue() override;
            std::experimental::optional<std::vector<uint8_t>> getData() override;
            int32_t getStatus() override;
            std::vector<uint8_t> serialize() override;
            std::chrono::system_clock::time_point getDate() override;
            std::shared_ptr<api::EthereumLikeBlock> getBlock() override;
            void setSignature(const std::vector<uint8_t> & vSignature, const std::vector<uint8_t> & rSignature, const std::vector<uint8_t> & sSignature) override ;
            void setDERSignature(const std::vector<uint8_t> & signature) override;
            void setVSignature(const std::vector<uint8_t> & vSignature) override;
            EthereumLikeTransaction & setGasPrice(const std::shared_ptr<BigInt>& gasPrice);
            EthereumLikeTransaction & setGasLimit(const std::shared_ptr<BigInt>& gasLimit);
            EthereumLikeTransaction & setNonce(const std::shared_ptr<BigInt>& nonce);
            EthereumLikeTransaction & setValue(const std::shared_ptr<BigInt>& value);
            EthereumLikeTransaction & setData(const std::vector<uint8_t> &data);
            EthereumLikeTransaction & setStatus(int32_t status);
            EthereumLikeTransaction & setReceiver(const std::string &receiver);
            EthereumLikeTransaction & setSender(const std::string &sender);
            // EthereumLikeTransaction & set
        private:
            std::chrono::system_clock::time_point _time;
            std::shared_ptr<EthereumLikeBlock> _block;
            std::string _hash;
            api::Currency _currency;
            std::shared_ptr<api::Amount> _gasPrice;
            std::shared_ptr<api::Amount> _gasLimit;
            std::shared_ptr<api::Amount> _gasUsed;
            std::shared_ptr<api::Amount> _value;
            std::shared_ptr<BigInt> _nonce;
            std::vector<uint8_t> _data;
            int32_t _status;
            std::shared_ptr<api::EthereumLikeAddress> _receiver;
            std::shared_ptr<api::EthereumLikeAddress> _sender;
            std::vector<uint8_t> _vSignature;
            std::vector<uint8_t> _rSignature;
            std::vector<uint8_t> _sSignature;
        };
    }
}