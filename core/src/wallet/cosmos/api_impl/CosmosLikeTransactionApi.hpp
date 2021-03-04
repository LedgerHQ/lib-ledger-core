/*
 *
 * CosmosLikeTransactionApi
 *
 * Created by El Khalil Bellakrid on  14/06/2019.
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

#ifndef LEDGER_CORE_COSMOSLIKETRANSACTIONAPI_H
#define LEDGER_CORE_COSMOSLIKETRANSACTIONAPI_H

#include <vector>

#include <api/Amount.hpp>
#include <api/CosmosLikeAmount.hpp>
#include <api/CosmosLikeBlock.hpp>
#include <api/CosmosLikeMessage.hpp>
#include <api/CosmosLikeOperation.hpp>
#include <api/CosmosLikeTransaction.hpp>
#include <api/Currency.hpp>
#include <math/BigInt.h>
#include <wallet/common/api_impl/OperationApi.h>
#include <wallet/cosmos/cosmos.hpp>

namespace ledger {
namespace core {
class CosmosLikeTransactionApi : public api::CosmosLikeTransaction {
   public:
    explicit CosmosLikeTransactionApi()
    {
    }
    explicit CosmosLikeTransactionApi(const cosmos::Transaction &txData);
    explicit CosmosLikeTransactionApi(const std::shared_ptr<OperationApi> &baseOp);

    void setCurrency(const api::Currency &currency);
    void setFee(const std::shared_ptr<BigInt> &fee);
    void setGas(const std::shared_ptr<BigInt> &gas);
    void setHash(const std::string &hash);
    void setSequence(const std::string &sequence);
    void setMemo(const std::string &memo);
    void setAccountNumber(const std::string &accountNumber);
    void setMessages(const std::vector<std::shared_ptr<api::CosmosLikeMessage>> &messages);
    void setSigningPubKey(const std::vector<uint8_t> &pubKey);

    void setSignature(
        const std::vector<uint8_t> &rSignature, const std::vector<uint8_t> &sSignature) override;
    void setDERSignature(const std::vector<uint8_t> &signature) override;

    std::chrono::system_clock::time_point getDate() const override;
    std::shared_ptr<api::Amount> getFee() const override;
    std::shared_ptr<api::Amount> getGas() const override;
    std::shared_ptr<api::BigInt> getGasUsed() const override;
    std::shared_ptr<api::BigInt> getGasWanted() const override;
    std::string getHash() const override;
    std::string getMemo() const override;
    std::vector<std::shared_ptr<api::CosmosLikeMessage>> getMessages() const override;
    std::vector<uint8_t> getSigningPubKey() const override;

    std::string serializeForSignature() override;
    std::string serializeForBroadcast(const std::string &mode) override;

    void setRawData(const cosmos::Transaction &txData);
    const cosmos::Transaction &getRawData() const;

    const std::string &getAccountNumber() const;
    const std::string &getAccountSequence() const;
    const api::Currency &getCurrency() const;
    const cosmos::Transaction &getTxData() const;

   private:
    /// Return true if the transaction is for a Stargate chain.
    bool isStargate() const;
    cosmos::Transaction _txData;

    std::string _accountNumber;
    std::string _accountSequence;
    api::Currency _currency;
    std::vector<uint8_t> _rSignature;
    std::vector<uint8_t> _sSignature;
    std::vector<uint8_t> _signingPubKey;
};
}  // namespace core
}  // namespace ledger

#endif  // LEDGER_CORE_COSMOSLIKETRANSACTIONAPI_H
