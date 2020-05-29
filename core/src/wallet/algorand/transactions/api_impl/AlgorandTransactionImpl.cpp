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
#include <algorand/model/AlgorandModelMapper.hpp>

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
        model::PaymentTxnFields details;
        details.amount = std::stoull(info.amount);

        if (info.closeAddress) {
            details.closeAddr = Address(currencies::ALGORAND, *info.closeAddress);
        }
        details.receiverAddr = Address(currencies::ALGORAND, info.recipientAddress);

        stxn.txn.details = details;
    }

    api::AlgorandPaymentInfo AlgorandTransactionImpl::getPaymentInfo() const
    {
        if (stxn.getType() != model::constants::pay) {
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Not a payment transaction");
        }

        const auto& details = boost::get<model::PaymentTxnFields>(stxn.txn.details);
        return model::toAPI(details);
    }

    void AlgorandTransactionImpl::setParticipationInfo(const api::AlgorandParticipationInfo& info)
    {
        stxn.txn.details = model::fromAPI(info);
    }

    api::AlgorandParticipationInfo AlgorandTransactionImpl::getParticipationInfo() const
    {
        if (stxn.getType() != model::constants::keyreg) {
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Not a key registration transaction");
        }

        const auto& details = boost::get<model::KeyRegTxnFields>(stxn.txn.details);
        return model::toAPI(details);
    }

    void AlgorandTransactionImpl::setAssetConfigurationInfo(const api::AlgorandAssetConfigurationInfo& info)
    {
        stxn.txn.details = model::fromAPI(info);
    }

    api::AlgorandAssetConfigurationInfo AlgorandTransactionImpl::getAssetConfigurationInfo() const
    {
        if (stxn.getType() != model::constants::acfg) {
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Not an asset configuration transaction");
        }

        const auto& details = boost::get<model::AssetConfigTxnFields>(stxn.txn.details);
        return model::toAPI(details);
    }

    void AlgorandTransactionImpl::setAssetTransferInfo(const api::AlgorandAssetTransferInfo& info)
    {
        stxn.txn.details = model::fromAPI(info);
    }

    api::AlgorandAssetTransferInfo AlgorandTransactionImpl::getAssetTransferInfo() const
    {
        if (stxn.getType() != model::constants::axfer) {
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Not an asset transfer transaction");
        }

        const auto& details = boost::get<model::AssetTransferTxnFields>(stxn.txn.details);
        return model::toAPI(details);
    }

    void AlgorandTransactionImpl::setAssetFreezeInfo(const api::AlgorandAssetFreezeInfo& info)
    {
        stxn.txn.details = model::fromAPI(info);
    }

    api::AlgorandAssetFreezeInfo AlgorandTransactionImpl::getAssetFreezeInfo() const
    {
        if (stxn.getType() != model::constants::afreeze) {
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Not an asset freeze transaction");
        }

        const auto& details = boost::get<model::AssetFreezeTxnFields>(stxn.txn.details);
        return model::toAPI(details);
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

