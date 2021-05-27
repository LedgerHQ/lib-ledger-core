/*
 * AlgorandTransactionImpl
 *
 * Created by Rémi Barjon on 15/05/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#include "../../model/transactions/AlgorandTransaction.hpp"
#include "../../model/transactions/AlgorandSignedTransaction.hpp"

#include <api/AlgorandTransaction.hpp>
#include <api/AlgorandPaymentInfo.hpp>
#include <api/AlgorandParticipationInfo.hpp>
#include <api/AlgorandAssetConfigurationInfo.hpp>
#include <api/AlgorandAssetTransferInfo.hpp>
#include <api/AlgorandAssetFreezeInfo.hpp>

#include <memory>
#include <string>
#include <vector>

namespace ledger {
namespace core {
namespace algorand {

    class AlgorandTransactionImpl : public api::AlgorandTransaction
    {
    public:
        explicit AlgorandTransactionImpl(model::SignedTransaction stxn);
        explicit AlgorandTransactionImpl(model::Transaction txn);
        std::string getId() const override;
        std::string getType() const override;
        std::string getSender() const override;
        std::string getFee() const override;
        std::string getNote() const override;
        std::string getRound() const override;
        std::string getSenderRewards() const override;
        std::string getReceiverRewards() const override;
        std::string getCloseRewards() const override;
        std::string getCorrelationId() const override;
        void setSender(const std::string& sender) override;
        void setFee(const std::string& fee) override;
        void setNote(const std::string& note) override;
        void setCorrelationId(const std::string& correlationId) override;
        void setPaymentInfo(const api::AlgorandPaymentInfo& info) override;
        api::AlgorandPaymentInfo getPaymentInfo() const override;
        void setParticipationInfo(const api::AlgorandParticipationInfo& info) override;
        api::AlgorandParticipationInfo getParticipationInfo() const override;
        void setAssetConfigurationInfo(const api::AlgorandAssetConfigurationInfo& info) override;
        api::AlgorandAssetConfigurationInfo getAssetConfigurationInfo() const override;
        void setAssetTransferInfo(const api::AlgorandAssetTransferInfo& info) override;
        api::AlgorandAssetTransferInfo getAssetTransferInfo() const override;
        void setAssetFreezeInfo(const api::AlgorandAssetFreezeInfo& info) override;
        api::AlgorandAssetFreezeInfo getAssetFreezeInfo() const override;
        std::vector<uint8_t> serialize() const override;
        void setSignature(const std::vector<uint8_t>& signature) override;

        const model::Transaction& getTransactionData() const;

    private:
        model::SignedTransaction stxn;
        std::string correlationId;
    };

} // namespace algorand
} // namespace core
} // namespace ledger

