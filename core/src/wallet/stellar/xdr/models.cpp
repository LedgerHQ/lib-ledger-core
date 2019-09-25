/*
 *
 * models.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 23/07/2019.
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

#include "models.hpp"

namespace ledger { namespace core { namespace stellar { namespace xdr {

// Here is the skycrapper of make_encoder functions

#define BEGIN_ENCODER(type) template <> ObjectEncoder make_encoder(const type & object) { \
    return [=] (Encoder& encoder) {

#define END_ENCODER };}

BEGIN_ENCODER(TimeBounds)
    encoder << object.minTime << object.maxTime;
END_ENCODER

BEGIN_ENCODER(PublicKey)
    encoder << static_cast<int32_t>(object.type);
    switch (object.type) {
        case PublicKeyType::PUBLIC_KEY_TYPE_ED25519:
            encoder << object.content;
            break;
    }
END_ENCODER

BEGIN_ENCODER(Memo)
    encoder << static_cast<int32_t>(object.type);
    switch (object.type) {
        case MemoType::MEMO_NONE:
            break;
        case MemoType::MEMO_TEXT:
            encoder << boost::get<std::string>(object.content);
            break;
        case MemoType::MEMO_ID:
            encoder << boost::get<uint64_t>(object.content);
            break;
        case MemoType::MEMO_HASH:
        case MemoType::MEMO_RETURN:
            encoder << boost::get<Hash>(object.content);
            break;
    }
END_ENCODER

BEGIN_ENCODER(Asset)
    encoder << static_cast<int32_t>(object.type);
    switch (object.type) {
        case AssetType::ASSET_TYPE_NATIVE:
            break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
            encoder << boost::get<AssetCode4>(object.assetCode);
            break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
            encoder << boost::get<AssetCode12>(object.assetCode);
            break;
    }
    if (object.issuer.nonEmpty()) {
        encoder << object.issuer;
    }
END_ENCODER

BEGIN_ENCODER(Price)
    encoder << object.n << object.d;
END_ENCODER

BEGIN_ENCODER(SignerKey)
    encoder << static_cast<int32_t>(object.type) << object.content;
END_ENCODER

BEGIN_ENCODER(Signer)
    encoder << object.key << object.weight;
END_ENCODER

BEGIN_ENCODER(CreateAccountOp)
    encoder << object.destination << object.startingBalance;
END_ENCODER

BEGIN_ENCODER(PaymentOp)
    encoder << object.destination << object.asset << object.amount;
END_ENCODER

BEGIN_ENCODER(PathPaymentOp)
    encoder
            << object.sendAsset
            << object.sendMax
            << object.destination
            << object.destAsset
            << object.destAmount
            << object.path;
END_ENCODER

BEGIN_ENCODER(ManageSellOfferOp)
    encoder
            << object.selling
            << object.buying
            << object.price
            << object.offerID;
END_ENCODER

BEGIN_ENCODER(ManageBuyOfferOp)
    encoder
            << object.selling
            << object.buying
            << object.buyAmount
            << object.price
            << object.offerID;
END_ENCODER

BEGIN_ENCODER(CreatePassiveSellOfferOp)
    encoder
            << object.selling
            << object.buying
            << object.amount
            << object.price;
END_ENCODER

BEGIN_ENCODER(SetOptionsOp)
    encoder
            << object.inflationDest
            << object.clearFlags
            << object.setFlags
            << object.masterWeight
            << object.lowThreshold
            << object.medThreshold
            << object.highThreshold
            << object.homeDomain
            << object.signer;
END_ENCODER

BEGIN_ENCODER(ChangeTrustOp)
    encoder << object.line << object.limit;
END_ENCODER

BEGIN_ENCODER(AllowTrustOp)
    encoder << object.trustor << static_cast<int32_t>(object.type);
    switch (object.type) {
        case AssetType::ASSET_TYPE_NATIVE:
            break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
            encoder << boost::get<AssetCode4>(object.assetCode);
            break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
            encoder << boost::get<AssetCode12>(object.assetCode);
            break;
    }
    encoder << object.authorize;
END_ENCODER

BEGIN_ENCODER(ManageDataOp)
    encoder << object.dataName << object.dataValue;
END_ENCODER

BEGIN_ENCODER(BumpSequenceOp)
    encoder << object.bumpTo;
END_ENCODER

BEGIN_ENCODER(Operation)
    encoder << object.sourceAccount << static_cast<int32_t>(object.type);
    switch (object.type) {
        case OperationType::CREATE_ACCOUNT:
            encoder << boost::get<CreateAccountOp>(object.content);
            break;
        case OperationType::PAYMENT:
            encoder << boost::get<PaymentOp>(object.content);
            break;
        case OperationType::PATH_PAYMENT:
            encoder << boost::get<PathPaymentOp>(object.content);
            break;
        case OperationType::MANAGE_OFFER:
            encoder << boost::get<ManageSellOfferOp>(object.content);
            break;
        case OperationType::CREATE_PASSIVE_OFFER:
            encoder << boost::get<CreatePassiveSellOfferOp>(object.content);
            break;
        case OperationType::SET_OPTIONS:
            encoder << boost::get<SetOptionsOp>(object.content);
            break;
        case OperationType::CHANGE_TRUST:
            encoder << boost::get<ChangeTrustOp>(object.content);
            break;
        case OperationType::ALLOW_TRUST:
            encoder << boost::get<AllowTrustOp>(object.content);
            break;
        case OperationType::ACCOUNT_MERGE:
            encoder << boost::get<AccountID>(object.content);
            break;
        case OperationType::INFLATION:
            break;
        case OperationType::MANAGE_DATA:
            encoder << boost::get<ManageDataOp>(object.content);
            break;
        case OperationType::BUMP_SEQUENCE:
            encoder << boost::get<BumpSequenceOp>(object.content);
            break;
        case OperationType::MANAGE_BUY_OFFER:
            encoder << boost::get<ManageBuyOfferOp>(object.content);
            break;
    }
END_ENCODER

BEGIN_ENCODER(Transaction)
    encoder
            << object.sourceAccount
            << object.fee
            << object.seqNum
            << object.timeBounds
            << object.memo
            << object.operations
            << (int32_t) 0;// reserved for future use
END_ENCODER

BEGIN_ENCODER(TransactionEnvelope)
    encoder << object.tx << object.signatures;
END_ENCODER

BEGIN_ENCODER(DecoratedSignature)
    encoder << object.hint << object.signature;
END_ENCODER

} } } }