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
#include "XDREncoder.hpp"
#include "XDRDecoder.hpp"

namespace ledger { namespace core { namespace stellar { namespace xdr {

// Here is the skycrapper of make_encoder & make_decoder functions

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
        case PublicKeyType::PUBLIC_KEY_TYPE_MUXED_ED25519:
            // DO NOTHING
            break;
    }
END_ENCODER

BEGIN_ENCODER(med25519)
    encoder << object.id << object.ed25519;
END_ENCODER

BEGIN_ENCODER(MuxedAccount)
    encoder << static_cast<int32_t>(object.type);
    switch (object.type) {
        case CryptoKeyType::KEY_TYPE_ED25519:
            encoder << boost::get<uint256>(object.content);
            break;
        case CryptoKeyType::KEY_TYPE_MUXED_ED25519:
            encoder << boost::get<med25519>(object.content);
            break;
        default:
            // Do nothing
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
            encoder << object.issuer.getValue();
            break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
            encoder << boost::get<AssetCode12>(object.assetCode);
            encoder << object.issuer.getValue();
            break;
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

BEGIN_ENCODER(PathPaymentStrictReceiveOp)
    encoder
            << object.sendAsset
            << object.sendMax
            << object.destination
            << object.destAsset
            << object.destAmount
            << object.path;
END_ENCODER

BEGIN_ENCODER(PathPaymentStrictSendOp)
    encoder
            << object.sendAsset
            << object.sendAmount
            << object.destination
            << object.destAsset
            << object.destMin
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
            encoder << boost::get<PathPaymentStrictReceiveOp>(object.content);
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

BEGIN_ENCODER(TransactionV0)
    encoder
            << object.sourceAccountEd25519
            << object.fee
            << object.seqNum
            << object.timeBounds
            << object.memo
            << object.operations
            << (int32_t) 0;// reserved for future use
END_ENCODER

BEGIN_ENCODER(FeeBumpTransaction)
    encoder
            << object.feeSource
            << object.fee
            << object.operations
            << (int32_t) 0;// reserved for future use
END_ENCODER

BEGIN_ENCODER(TransactionV0Envelope)
  encoder << object.tx << object.signatures;
END_ENCODER

BEGIN_ENCODER(TransactionV1Envelope)
    encoder << object.tx << object.signatures;
END_ENCODER

BEGIN_ENCODER(FeeBumpTransactionEnvelope)
    encoder << object.tx << object.signatures;
END_ENCODER

BEGIN_ENCODER(TransactionEnvelope)
    encoder << static_cast<int32_t>(object.type);
    switch (object.type) {
        case EnvelopeType::ENVELOPE_TYPE_TX_V0:
            encoder << boost::get<TransactionV0Envelope>(object.content);
            break;
        case EnvelopeType::ENVELOPE_TYPE_TX:
            encoder << boost::get<TransactionV1Envelope>(object.content);
            break;
        case EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP:
            encoder << boost::get<FeeBumpTransactionEnvelope>(object.content);
            break;
    }
END_ENCODER

BEGIN_ENCODER(DecoratedSignature)
    encoder << object.hint << object.signature;
END_ENCODER


// make_decoder functions

#define BEGIN_DECODER(T) template <> ObjectDecoder make_decoder(T & object) { \
    return [&] (Decoder& decoder) mutable {

#define END_DECODER };}

BEGIN_DECODER(TimeBounds)
    decoder >> object.minTime >> object.maxTime;
END_DECODER

BEGIN_DECODER(PublicKeyType)
    int32_t type;
    decoder >> type;
    object = static_cast<PublicKeyType>(type);
END_DECODER

BEGIN_DECODER(PublicKey)
    decoder >> object.type;
    switch (object.type) {
        case PublicKeyType::PUBLIC_KEY_TYPE_ED25519:
            decoder >> object.content;
            break;
        case PublicKeyType::PUBLIC_KEY_TYPE_MUXED_ED25519:
            // Do nothing
            break;
    }
END_DECODER

BEGIN_DECODER(CryptoKeyType)
    int32_t type;
    decoder >> type;
    object = static_cast<CryptoKeyType>(type);
END_DECODER

BEGIN_DECODER(med25519)
    decoder >> object.id >> object.ed25519;
END_DECODER

BEGIN_DECODER(MuxedAccount)
    decoder >> object.type;
    switch (object.type) {
        case CryptoKeyType::KEY_TYPE_ED25519: {
            object.content = uint256();
            decoder >> boost::get<uint256>(object.content);
            break;
        }
        case CryptoKeyType::KEY_TYPE_MUXED_ED25519: {
            object.content = med25519();
            decoder >> boost::get<med25519>(object.content);
            break;
        }
        case CryptoKeyType::KEY_TYPE_HASH_X:
        case CryptoKeyType::KEY_TYPE_PRE_AUTH_TX:
            // No op
            break;
    }
END_DECODER

BEGIN_DECODER(MemoType)
    int32_t type;
    decoder >> type;
    object = static_cast<MemoType>(type);
END_DECODER

BEGIN_DECODER(Memo)
    decoder >> object.type;

    switch (object.type) {
        case MemoType::MEMO_NONE:
            break;
        case MemoType::MEMO_TEXT:
            object.content = std::string();
            decoder >> boost::get<std::string>(object.content);
            break;
        case MemoType::MEMO_ID:
            object.content = uint64_t();
            decoder >> boost::get<uint64_t>(object.content);
            break;
        case MemoType::MEMO_HASH:
        case MemoType::MEMO_RETURN:
            object.content = Hash();
            decoder >> boost::get<Hash>(object.content);
            break;
    }
END_DECODER

BEGIN_DECODER(AssetType)
    int32_t type;
    decoder >> type;
    object = static_cast<AssetType>(type);
END_DECODER

BEGIN_DECODER(Asset)
    decoder >> object.type;
    switch (object.type) {
        case AssetType::ASSET_TYPE_NATIVE:
            break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM4: {
            PublicKey issuer;
            object.assetCode = AssetCode4();
            decoder >> boost::get<AssetCode4>(object.assetCode);
            decoder >> issuer;
            object.issuer = issuer;
        }
            break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12: {
            PublicKey issuer;
            object.assetCode = AssetCode12();
            decoder >> boost::get<AssetCode12>(object.assetCode);
            decoder >> issuer;
            object.issuer = issuer;
        }
            break;
    }

END_DECODER

BEGIN_DECODER(Price)
    decoder >> object.n >> object.d;
END_DECODER

BEGIN_DECODER(SignerKeyType)
    int32_t type;
    decoder >> type;
    object = static_cast<SignerKeyType>(type);
END_DECODER

BEGIN_DECODER(SignerKey)
    decoder >> object.type >> object.content;
END_DECODER

BEGIN_DECODER(Signer)
    decoder >> object.key >> object.weight;
END_DECODER

BEGIN_DECODER(CreateAccountOp)
    decoder >> object.destination >> object.startingBalance;
END_DECODER

BEGIN_DECODER(PaymentOp)
    decoder >> object.destination >> object.asset >> object.amount;
END_DECODER

BEGIN_DECODER(PathPaymentStrictReceiveOp)
    decoder
            >> object.sendAsset
            >> object.sendMax
            >> object.destination
            >> object.destAsset
            >> object.destAmount
            >> object.path;
END_DECODER

BEGIN_DECODER(PathPaymentStrictSendOp)
    decoder
            >> object.sendAsset
            >> object.sendAmount
            >> object.destination
            >> object.destAsset
            >> object.destMin
            >> object.path;
END_DECODER

BEGIN_DECODER(ManageSellOfferOp)
    decoder
            >> object.selling
            >> object.buying
            >> object.price
            >> object.offerID;
END_DECODER

BEGIN_DECODER(ManageBuyOfferOp)
    decoder
            >> object.selling
            >> object.buying
            >> object.buyAmount
            >> object.price
            >> object.offerID;
END_DECODER

BEGIN_DECODER(CreatePassiveSellOfferOp)
    decoder
            >> object.selling
            >> object.buying
            >> object.amount
            >> object.price;
END_DECODER

BEGIN_DECODER(SetOptionsOp)
    decoder
            >> object.inflationDest
            >> object.clearFlags
            >> object.setFlags
            >> object.masterWeight
            >> object.lowThreshold
            >> object.medThreshold
            >> object.highThreshold
            >> object.homeDomain
            >> object.signer;
END_DECODER

BEGIN_DECODER(ChangeTrustOp)
    decoder >> object.line >> object.limit;
END_DECODER

BEGIN_DECODER(AllowTrustOp)
    decoder >> object.trustor >> object.type;
    switch (object.type) {
        case AssetType::ASSET_TYPE_NATIVE:
            break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM4:
            object.assetCode = AssetCode4();
            decoder >> boost::get<AssetCode4>(object.assetCode);
            break;
        case AssetType::ASSET_TYPE_CREDIT_ALPHANUM12:
            object.assetCode = AssetCode12();
            decoder >> boost::get<AssetCode12>(object.assetCode);
            break;
    }
    decoder >> object.authorize;
END_DECODER

BEGIN_DECODER(ManageDataOp)
    decoder >> object.dataName >> object.dataValue;
END_DECODER

BEGIN_DECODER(BumpSequenceOp)
    decoder >> object.bumpTo;
END_DECODER

BEGIN_DECODER(OperationType)
    int32_t type;
    decoder >> type;
    object = static_cast<OperationType>(type);
END_DECODER

BEGIN_DECODER(EnvelopeType)
    int32_t type;
    decoder >> type;
    object = static_cast<EnvelopeType>(type);
END_DECODER

BEGIN_DECODER(Operation)
    decoder >> object.sourceAccount >> object.type;
    switch (object.type) {
        case OperationType::CREATE_ACCOUNT:
            object.content = CreateAccountOp();
            decoder >> boost::get<CreateAccountOp>(object.content);
            break;
        case OperationType::PAYMENT:
            object.content = PaymentOp();
            decoder >> boost::get<PaymentOp>(object.content);
            break;
        case OperationType::PATH_PAYMENT:
            object.content = PathPaymentStrictReceiveOp();
            decoder >> boost::get<PathPaymentStrictReceiveOp>(object.content);
            break;
        case OperationType::MANAGE_OFFER:
            object.content = ManageSellOfferOp();
            decoder >> boost::get<ManageSellOfferOp>(object.content);
            break;
        case OperationType::CREATE_PASSIVE_OFFER:
            object.content = CreatePassiveSellOfferOp();
            decoder >> boost::get<CreatePassiveSellOfferOp>(object.content);
            break;
        case OperationType::SET_OPTIONS:
            object.content = SetOptionsOp();
            decoder >> boost::get<SetOptionsOp>(object.content);
            break;
        case OperationType::CHANGE_TRUST:
            object.content = ChangeTrustOp();
            decoder >> boost::get<ChangeTrustOp>(object.content);
            break;
        case OperationType::ALLOW_TRUST:
            object.content = AllowTrustOp();
            decoder >> boost::get<AllowTrustOp>(object.content);
            break;
        case OperationType::ACCOUNT_MERGE:
            object.content = AccountID();
            decoder >> boost::get<AccountID>(object.content);
            break;
        case OperationType::INFLATION:
            break;
        case OperationType::MANAGE_DATA:
            object.content = ManageDataOp();
            decoder >> boost::get<ManageDataOp>(object.content);
            break;
        case OperationType::BUMP_SEQUENCE:
            object.content = BumpSequenceOp();
            decoder >> boost::get<BumpSequenceOp>(object.content);
            break;
        case OperationType::MANAGE_BUY_OFFER:
            object.content = ManageBuyOfferOp();
            decoder >> boost::get<ManageBuyOfferOp>(object.content);
            break;
    }
END_DECODER

BEGIN_DECODER(Transaction)
    int32_t unused;
    decoder
            >> object.sourceAccount
            >> object.fee
            >> object.seqNum
            >> object.timeBounds
            >> object.memo
            >> object.operations
            >> unused; // reserved for future use
END_DECODER

BEGIN_DECODER(TransactionV0)
    int32_t unused;
    decoder
            >> object.sourceAccountEd25519
            >> object.fee
            >> object.seqNum
            >> object.timeBounds
            >> object.memo
            >> object.operations
            >> unused; // reserved for future use
END_DECODER

BEGIN_DECODER(FeeBumpTransaction)
    int32_t unused;
    decoder
            >> object.feeSource
            >> object.fee
            >> unused; // reserved for future use
END_DECODER

BEGIN_DECODER(FeeBumpTransactionEnvelope)
    decoder >> object.tx;
END_DECODER

BEGIN_DECODER(TransactionV1Envelope)
    decoder >> object.tx >> object.signatures;
END_DECODER

BEGIN_DECODER(TransactionV0Envelope)
    decoder >> object.tx >> object.signatures;
END_DECODER

BEGIN_DECODER(TransactionEnvelope)
    decoder >> object.type;
    switch (object.type) {
        case EnvelopeType::ENVELOPE_TYPE_TX_V0:
            object.content = TransactionV0Envelope();
            decoder >> boost::get<TransactionV0Envelope>(object.content);
            break;
        case EnvelopeType::ENVELOPE_TYPE_TX:
            object.content = TransactionV1Envelope();
            decoder >> boost::get<TransactionV1Envelope>(object.content);
            break;
        case EnvelopeType::ENVELOPE_TYPE_TX_FEE_BUMP:
            object.content = FeeBumpTransactionEnvelope();
            decoder >> boost::get<FeeBumpTransactionEnvelope>(object.content);
            break;
    }
END_DECODER

BEGIN_DECODER(DecoratedSignature)
    decoder >> object.hint >> object.signature;
END_DECODER


} } } }