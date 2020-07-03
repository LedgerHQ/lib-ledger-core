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

#include "AlgorandTransactionImpl.hpp"

#include "../../AlgorandAddress.hpp"
#include "../../model/AlgorandModelMapper.hpp"

#include <utils/Exception.hpp>

namespace ledger {
namespace core {
namespace algorand {

    AlgorandTransactionImpl::AlgorandTransactionImpl(model::SignedTransaction stxn)
        : stxn(std::move(stxn))
    {}

    AlgorandTransactionImpl::AlgorandTransactionImpl(model::Transaction txn)
        : stxn(std::move(txn))
    {}

    std::string AlgorandTransactionImpl::getId() const
    {
        return stxn.txn.header.id.getValueOr("");
    }

    std::string AlgorandTransactionImpl::getType() const
    {
        return stxn.getType();
    }

    std::string AlgorandTransactionImpl::getSender() const
    {
        return stxn.txn.header.sender.toString();
    }

    std::string AlgorandTransactionImpl::getFee() const
    {
        return std::to_string(stxn.txn.header.fee);
    }

    std::string AlgorandTransactionImpl::getNote() const
    {
        const auto& note = stxn.txn.header.note;
        if (note.hasValue()) {
            return { std::begin(*note), std::end(*note) };
        }
        return "";
    }

    std::string AlgorandTransactionImpl::getRound() const
    {
        return std::to_string(stxn.txn.header.round.getValueOr(0));
    }

    std::string AlgorandTransactionImpl::getSenderRewards() const
    {
        return std::to_string(stxn.txn.header.senderRewards.getValueOr(0));
    }

    std::string AlgorandTransactionImpl::getReceiverRewards() const
    {
        return std::to_string(stxn.txn.header.receiverRewards.getValueOr(0));
    }

    std::string AlgorandTransactionImpl::getCloseRewards() const
    {
        return std::to_string(stxn.txn.header.closeRewards.getValueOr(0));
    }

    void AlgorandTransactionImpl::setSender(const std::string& sender)
    {
        stxn.txn.header.sender = Address(currencies::ALGORAND, sender);
    }

    void AlgorandTransactionImpl::setFee(const std::string& fee)
    {
        stxn.txn.header.fee = std::stoull(fee);
    }

    void AlgorandTransactionImpl::setNote(const std::string& note)
    {
        stxn.txn.header.note = { std::begin(note), std::end(note) };
    }

    void AlgorandTransactionImpl::setPaymentInfo(const api::AlgorandPaymentInfo& info)
    {
        stxn.txn.header.type = model::constants::pay;
        stxn.txn.details = model::fromAPI(info);
    }

    api::AlgorandPaymentInfo AlgorandTransactionImpl::getPaymentInfo() const
    {
        if (stxn.getType() != model::constants::pay) {
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Not a payment transaction");
        }

        return model::toAPI(boost::get<model::PaymentTxnFields>(stxn.txn.details));
    }

    void AlgorandTransactionImpl::setParticipationInfo(const api::AlgorandParticipationInfo& info)
    {
        stxn.txn.header.type = model::constants::keyreg;
        stxn.txn.details = model::fromAPI(info);
    }

    api::AlgorandParticipationInfo AlgorandTransactionImpl::getParticipationInfo() const
    {
        if (stxn.getType() != model::constants::keyreg) {
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Not a key registration transaction");
        }

        return model::toAPI(boost::get<model::KeyRegTxnFields>(stxn.txn.details));
    }

    void AlgorandTransactionImpl::setAssetConfigurationInfo(const api::AlgorandAssetConfigurationInfo& info)
    {
        stxn.txn.header.type = model::constants::acfg;
        stxn.txn.details = model::fromAPI(info);
    }

    api::AlgorandAssetConfigurationInfo AlgorandTransactionImpl::getAssetConfigurationInfo() const
    {
        if (stxn.getType() != model::constants::acfg) {
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Not an asset configuration transaction");
        }

        return model::toAPI(boost::get<model::AssetConfigTxnFields>(stxn.txn.details));
    }

    void AlgorandTransactionImpl::setAssetTransferInfo(const api::AlgorandAssetTransferInfo& info)
    {
        stxn.txn.header.type = model::constants::axfer;
        stxn.txn.details = model::fromAPI(info);
    }

    api::AlgorandAssetTransferInfo AlgorandTransactionImpl::getAssetTransferInfo() const
    {
        if (stxn.getType() != model::constants::axfer) {
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Not an asset transfer transaction");
        }

        return model::toAPI(boost::get<model::AssetTransferTxnFields>(stxn.txn.details));
    }

    void AlgorandTransactionImpl::setAssetFreezeInfo(const api::AlgorandAssetFreezeInfo& info)
    {
        stxn.txn.header.type = model::constants::afreeze;
        stxn.txn.details = model::fromAPI(info);
    }

    api::AlgorandAssetFreezeInfo AlgorandTransactionImpl::getAssetFreezeInfo() const
    {
        if (stxn.getType() != model::constants::afreeze) {
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Not an asset freeze transaction");
        }

        return model::toAPI(boost::get<model::AssetFreezeTxnFields>(stxn.txn.details));
    }

    std::vector<uint8_t> AlgorandTransactionImpl::serialize() const
    {
        return stxn.serialize();
    }

    void AlgorandTransactionImpl::setSignature(const std::vector<uint8_t>& signature)
    {
        stxn.setSignature(signature);
    }

    const model::Transaction& AlgorandTransactionImpl::getTransactionData() const
    {
        return stxn.txn;
    }

} // namespace algorand
} // namespace core
} // namespace ledger

