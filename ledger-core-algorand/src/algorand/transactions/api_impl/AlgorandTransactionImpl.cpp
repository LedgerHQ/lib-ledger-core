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

#include <algorand/AlgorandAddress.hpp>

#include <core/utils/Exception.hpp>

namespace ledger {
namespace core {
namespace algorand {

    AlgorandTransactionImpl::AlgorandTransactionImpl(model::SignedTransaction txn)
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
        stxn.txn.header.sender = Address(currencies::algorand(), sender);
    }

    void AlgorandTransactionImpl::setFee(const std::string& fee)
    {
        stxn.txn.header.fee = std::stoi(fee);
    }

    void AlgorandTransactionImpl::setNote(const std::string& note)
    {
        stxn.txn.header.note = { std::begin(note), std::end(note) };
    }

    void AlgorandTransactionImpl::setPaymentInfo(const api::AlgorandPaymentInfo& info)
    {
        model::PaymentTxnFields details;
        details.amount = std::stoi(info.amount);

        if (info.closeAddress) {
            details.closeAddr = Address(currencies::algorand(), *info.closeAddress);
        }
        details.receiverAddr = Address(currencies::algorand(), info.recipientAddress);

        stxn.txn.details = details;
    }

    api::AlgorandPaymentInfo AlgorandTransactionImpl::getPaymentInfo() const
    {
        if (stxn.getType() != model::constants::pay) {
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Not a payment transaction");
        }

        const auto& details = boost::get<model::PaymentTxnFields>(stxn.txn.details);
        api::AlgorandPaymentInfo info;

        info.amount = std::to_string(details.amount);
        if (details.closeAddr.hasValue()) {
            info.closeAddress = details.closeAddr->toString();
        }
        info.recipientAddress = details.receiverAddr.toString();

        return info;
    }

    void AlgorandTransactionImpl::setParticipationInfo(const api::AlgorandParticipationInfo& info)
    {
        model::KeyRegTxnFields details;

        details.selectionPk = info.vrfPublicKey;
        details.voteFirst = std::stoi(info.voteFirstRound);
        details.voteKeyDilution = std::stoi(info.voteKeyDilution);
        details.votePk = info.rootPublicKey;
        details.voteLast = std::stoi(info.voteLastRound);

        stxn.txn.details = details;
    }

    api::AlgorandParticipationInfo AlgorandTransactionImpl::getParticipationInfo() const
    {
        if (stxn.getType() != model::constants::keyreg) {
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Not a key registration transaction");
        }

        const auto& details = boost::get<model::KeyRegTxnFields>(stxn.txn.details);
        api::AlgorandParticipationInfo info;

        info.vrfPublicKey = details.selectionPk;
        info.voteFirstRound = std::to_string(details.voteFirst);
        info.voteKeyDilution = std::to_string(details.voteKeyDilution);
        info.rootPublicKey = details.votePk;
        info.voteLastRound = std::to_string(details.voteLast);

        return info;
    }

    void AlgorandTransactionImpl::setAssetConfigurationInfo(const api::AlgorandAssetConfigurationInfo& info)
    {
        model::AssetConfigTxnFields details;

        if (info.assetId) {
            details.assetId = std::stoi(*info.assetId);
        }
        if (info.assetParams) {
            const auto params = *info.assetParams;
            model::AssetParams apar;
            if (params.metadataHash) {
                apar.metaDataHash = { std::begin(*params.metadataHash), std::end(*params.metadataHash) };
            }
            if (params.assetName) {
                apar.assetName = *params.assetName;
            }
            if (params.url) {
                apar.url = *params.url;
            }
            if (params.clawbackAddress) {
                apar.clawbackAddr = Address(currencies::algorand(), *params.clawbackAddress);
            }
            if (params.decimals) {
                apar.decimals = *params.decimals;
            }
            if (params.defaultFrozen) {
                apar.defaultFrozen = *params.defaultFrozen;
            }
            if (params.freezeAddress) {
                apar.freezeAddr = Address(currencies::algorand(), *params.freezeAddress);
            }
            if (params.managerAddress) {
                apar.managerAddr = Address(currencies::algorand(), *params.managerAddress);
            }
            if (params.reserveAddress) {
                apar.reserveAddr = Address(currencies::algorand(), *params.reserveAddress);
            }
            if (params.total) {
                apar.total = std::stoi(*params.total);
            }
            if (params.unitName) {
                apar.unitName = *params.unitName;
            }

            details.assetParams = apar;
        }

        stxn.txn.details = details;
    }

    api::AlgorandAssetConfigurationInfo AlgorandTransactionImpl::getAssetConfigurationInfo() const
    {
        if (stxn.getType() != model::constants::acfg) {
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Not an asset configuration transaction");
        }

        const auto& details = boost::get<model::AssetConfigTxnFields>(stxn.txn.details);
        api::AlgorandAssetConfigurationInfo info;

        if (details.assetId) {
            info.assetId = std::to_string(*details.assetId);
        }
        if (details.assetParams) {
            const auto& apar = *details.assetParams;
            api::AlgorandAssetParams params;

            if (apar.metaDataHash) {
                params.metadataHash = std::string(std::begin(*apar.metaDataHash), std::end(*apar.metaDataHash));
            }
            if (apar.assetName) {
                params.assetName = *apar.assetName;
            }
            if (apar.url) {
                params.url = *apar.url;
            }
            if (apar.clawbackAddr) {
                params.clawbackAddress = apar.clawbackAddr->toString();
            }
            if (apar.decimals) {
                params.decimals = *apar.decimals;
            }
            if (apar.defaultFrozen) {
                params.defaultFrozen = *apar.defaultFrozen;
            }
            if (apar.freezeAddr) {
                params.freezeAddress = apar.freezeAddr->toString();
            }
            if (apar.managerAddr) {
                params.managerAddress = apar.managerAddr->toString();
            }
            if (apar.reserveAddr) {
                params.reserveAddress = apar.reserveAddr->toString();
            }
            if (apar.total) {
                params.total = std::to_string(*apar.total);
            }
            if (apar.unitName) {
                params.unitName = *params.unitName;
            }

            info.assetParams = params;
        }

        return info;
    }

    void AlgorandTransactionImpl::setAssetTransferInfo(const api::AlgorandAssetTransferInfo& info)
    {
        model::AssetTransferTxnFields details;

        if (info.amount) {
            details.assetAmount = std::stoi(*info.amount);
        }
        if (info.closeAddress) {
            details.assetCloseTo = Address(currencies::algorand(), *info.closeAddress);
        }
        details.assetReceiver = Address(currencies::algorand(), info.recipientAddress);
        if (info.clawedBackAddress) {
            details.assetSender = Address(currencies::algorand(), *info.clawedBackAddress);
        }
        details.assetId = std::stoi(info.assetId);

        stxn.txn.details = details;
    }

    api::AlgorandAssetTransferInfo AlgorandTransactionImpl::getAssetTransferInfo() const
    {
        if (stxn.getType() != model::constants::axfer) {
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Not an asset transfer transaction");
        }

        const auto& details = boost::get<model::AssetTransferTxnFields>(stxn.txn.details);
        api::AlgorandAssetTransferInfo info;

        if (details.assetAmount) {
            info.amount = std::to_string(*details.assetAmount);
        }
        if (details.assetCloseTo) {
            info.closeAddress = details.assetCloseTo->toString();
        }
        info.recipientAddress = details.assetReceiver.toString();
        if (details.assetSender) {
            info.clawedBackAddress = details.assetSender->toString();
        }
        info.assetId = std::to_string(details.assetId);

        return info;
    }

    void AlgorandTransactionImpl::setAssetFreezeInfo(const api::AlgorandAssetFreezeInfo& info)
    {
        model::AssetFreezeTxnFields details;

        details.assetFrozen = info.frozen;
        details.frozenAddress = Address(currencies::algorand(), info.frozenAddress);
        details.assetId = std::stoi(info.assetId);

        stxn.txn.details = details;
    }

    api::AlgorandAssetFreezeInfo AlgorandTransactionImpl::getAssetFreezeInfo() const
    {
        if (stxn.getType() != model::constants::afreeze) {
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "Not an asset freeze transaction");
        }

        const auto& details = boost::get<model::AssetFreezeTxnFields>(stxn.txn.details);
        api::AlgorandAssetFreezeInfo info;

        info.frozen = details.assetFrozen;
        info.frozenAddress = details.frozenAddress.toString();
        info.assetId = details.assetId;

        return info;
    }

    std::vector<uint8_t> AlgorandTransactionImpl::serialize() const
    {
        return stxn.serialize();
    }

    void AlgorandTransactionImpl::setSignature(const std::vector<uint8_t>& signature)
    {
        stxn.setSignature(signature);
    }

    std::shared_ptr<api::AlgorandTransaction>
    AlgorandTransactionImpl::makeTransaction(model::Transaction txn)
    {
        return std::make_shared<AlgorandTransactionImpl>(model::SignedTransaction(std::move(txn)));
    }

} // namespace algorand
} // namespace core
} // namespace ledger

