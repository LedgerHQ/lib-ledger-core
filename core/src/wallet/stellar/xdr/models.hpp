/*
 *
 * models.hpp
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

#ifndef LEDGER_CORE_MODELS_HPP
#define LEDGER_CORE_MODELS_HPP

#include <list>
#include <utils/Option.hpp>
#include <boost/variant.hpp>
#include <array>

namespace ledger {
    namespace core {
        namespace stellar {
            namespace xdr {

                // Models based on https://github.com/stellar/stellar-core/tree/master/src/xdr
                // Please use this file to create API around the structures, the encoder doesn't
                // Check the validity of the data, and an error should be raised as soon as the
                // user is attempting any invalid operation

                enum class CryptoKeyType : uint32_t
                {
                    KEY_TYPE_ED25519 = 0,
                    KEY_TYPE_PRE_AUTH_TX = 1,
                    KEY_TYPE_HASH_X = 2,
                    // MUXED enum values for supported type are derived from the enum values
                    // above by ORing them with 0x100
                    KEY_TYPE_MUXED_ED25519 = 0x100
                };

                enum class PublicKeyType : uint32_t
                {
                    PUBLIC_KEY_TYPE_ED25519 = static_cast<uint32_t>(CryptoKeyType::KEY_TYPE_ED25519),
                    PUBLIC_KEY_TYPE_MUXED_ED25519 = static_cast<uint32_t>(CryptoKeyType::KEY_TYPE_MUXED_ED25519)
                };

                using uint256 = std::array<uint8_t, 32>;
                using SequenceNumber = int64_t;
                using TimePoint = uint64_t;
                using Hash = std::array<uint8_t, 32>;
                using DataValue = std::vector<uint8_t>;

                // String types (CHECK THE VALIDITY OF THE DATA BEFORE SETTING!)
                using string32 = std::string; // Max 32 chars
                using string28 = std::string; // Max 28 chars
                using string64 = std::string; // Max 64 chars

                // Signature
                using SignatureHint = std::array<uint8_t, 4>;
                using Signature = std::vector<uint8_t>; // Max length == 64

                // TimeBounds structure
                struct TimeBounds {
                    TimePoint minTime;
                    TimePoint maxTime;
                };

                // Public key union
                struct PublicKey {
                    PublicKeyType type;
                    uint256 content;

                    PublicKey() : type(PublicKeyType::PUBLIC_KEY_TYPE_ED25519) {};
                };

                using AccountID = PublicKey;

                struct med25519 {
                    uint64_t id;
                    uint256 ed25519;
                };

                using CryptoKeyVariant = boost::variant<uint256, med25519>;

                // Source or destination of a payment operation
                struct MuxedAccount {
                    CryptoKeyType type;
                    CryptoKeyVariant  content;
                };

                // Memo union
                enum class MemoType : uint32_t {
                    MEMO_NONE = 0,
                    MEMO_TEXT = 1,
                    MEMO_ID = 2,
                    MEMO_HASH = 3,
                    MEMO_RETURN = 4
                };

                struct Memo {
                    MemoType type;
                    boost::variant<string28, uint64_t, Hash> content;

                    Memo() : type(MemoType::MEMO_NONE) {};
                };

                // Asset union
                using AssetCode4 = std::array<uint8_t, 4>;
                using AssetCode12 = std::array<uint8_t, 12>;

                enum class AssetType : uint32_t
                {
                    ASSET_TYPE_NATIVE = 0,
                    ASSET_TYPE_CREDIT_ALPHANUM4 = 1,
                    ASSET_TYPE_CREDIT_ALPHANUM12 = 2
                };

                struct Asset {
                    AssetType type;
                    boost::variant<AssetCode4, AssetCode12> assetCode;
                    Option<AccountID> issuer;
                };

                // Price structure (fractional representation)
                struct Price
                {
                    int32_t n; // numerator
                    int32_t d; // denominator
                };

                // Signer key union
                enum class SignerKeyType : uint32_t
                {
                    SIGNER_KEY_TYPE_ED25519 = static_cast<uint32_t>(CryptoKeyType::KEY_TYPE_ED25519),
                    SIGNER_KEY_TYPE_PRE_AUTH_TX = static_cast<uint32_t>(CryptoKeyType::KEY_TYPE_PRE_AUTH_TX),
                    SIGNER_KEY_TYPE_HASH_X = static_cast<uint32_t>(CryptoKeyType::KEY_TYPE_HASH_X)
                };

                struct SignerKey {
                    SignerKeyType type;
                    uint256 content;
                };

                // Signer structure
                struct Signer
                {
                    SignerKey key;
                    uint32_t weight;
                };

                // Operation union
                struct CreateAccountOp
                {
                    AccountID destination; // account to create
                    int64_t startingBalance; // amount they end up with
                };

                struct PaymentOp
                {
                    MuxedAccount destination; // recipient of the payment
                    Asset asset;           // what they end up with
                    int64_t amount;          // amount they end up with
                };


                // PathPaymentStrictReceive
                // send an amount to a destination account through a path.
                // (up to sendMax, sendAsset)
                // (X0, Path[0]) .. (Xn, Path[n])
                // (destAmount, destAsset)
                // Threshold: med
                // Result: PathPaymentStrictReceiveResult
                struct PathPaymentStrictReceiveOp
                {
                    Asset sendAsset;            // asset we pay with
                    int64_t sendMax;            // the maximum amount of sendAsset to
                                                // send (excluding fees).
                                                // The operation will fail if can't be met

                    MuxedAccount destination;   // recipient of the payment
                    Asset destAsset;            // what they end up with
                    int64_t destAmount;         // amount they end up with

                    std::vector<Asset> path; // additional hops it must go through to get there
                };

                /* PathPaymentStrictSend
                send an amount to a destination account through a path.
                (sendMax, sendAsset)
                (X0, Path[0]) .. (Xn, Path[n])
                (at least destAmount, destAsset)
                Threshold: med
                Result: PathPaymentStrictSendResult
                */
                struct PathPaymentStrictSendOp
                {
                    Asset sendAsset;            // asset we pay with
                    int64_t sendAmount;         // amount of sendAsset to send (excluding fees)

                    MuxedAccount destination;   // recipient of the payment
                    Asset destAsset;            // what they end up with
                    int64_t destMin;            // the minimum amount of dest asset to
                                                // be received
                    // The operation will fail if it can't be met
                    std::vector<Asset> path; // additional hops it must go through to get there
                };

                struct ManageSellOfferOp
                {
                    Asset selling;
                    Asset buying;
                    int64_t amount; // amount being sold. if set to 0, delete the offer
                    Price price;  // price of thing being sold in terms of what you are buying

                    // 0=create a new offer, otherwise edit an existing offer
                    int64_t offerID;
                };

                struct ManageBuyOfferOp
                {
                    Asset selling;
                    Asset buying;
                    int64_t buyAmount; // amount being bought. if set to 0, delete the offer
                    Price price;     // price of thing being bought in terms of what you are
                    // selling

                    // 0=create a new offer, otherwise edit an existing offer
                    int64_t offerID;
                };

                struct CreatePassiveSellOfferOp
                {
                    Asset selling; // A
                    Asset buying;  // B
                    int64_t amount;  // amount taker gets. if set to 0, delete the offer
                    Price price;   // cost of A in terms of B
                };

                struct SetOptionsOp
                {
                    Option<AccountID> inflationDest; // sets the inflation destination

                    Option<uint32_t> clearFlags; // which flags to clear
                    Option<uint32_t> setFlags;   // which flags to set

                    // account threshold manipulation
                    Option<uint32_t> masterWeight; // weight of the master account
                    Option<uint32_t> lowThreshold;
                    Option<uint32_t> medThreshold;
                    Option<uint32_t> highThreshold;

                    Option<string32> homeDomain; // sets the home domain

                    // Add, update or remove a signer for the account
                    // signer is deleted if the weight is 0
                    Option<Signer> signer;
                };

                struct ChangeTrustOp
                {
                    Asset line;
                    // if limit is set to 0, deletes the trust line
                    int64_t limit;
                };

                struct AllowTrustOp
                {
                    AccountID trustor;
                    AssetType type;
                    boost::variant<AssetCode4, AssetCode12> assetCode;

                    bool authorize;
                };

                struct ManageDataOp
                {
                    string64 dataName;
                    Option<DataValue> dataValue; // set to null to clear
                };

                struct BumpSequenceOp
                {
                    SequenceNumber bumpTo;
                };

                enum class OperationType : uint32_t  {CREATE_ACCOUNT = 0, PAYMENT = 1, PATH_PAYMENT = 2, MANAGE_OFFER = 3, CREATE_PASSIVE_OFFER = 4, SET_OPTIONS = 5,
                    CHANGE_TRUST = 6, ALLOW_TRUST = 7, ACCOUNT_MERGE = 8, INFLATION = 9, MANAGE_DATA = 10, BUMP_SEQUENCE = 11, MANAGE_BUY_OFFER = 12, num_types};

                using OperationVariant = boost::variant<
                        CreateAccountOp, PaymentOp, PathPaymentStrictReceiveOp, ManageSellOfferOp,
                        CreatePassiveSellOfferOp, SetOptionsOp, ChangeTrustOp, AllowTrustOp,
                        AccountID , ManageDataOp, BumpSequenceOp, ManageBuyOfferOp,
                        PathPaymentStrictSendOp>;

                struct Operation {
                    // sourceAccount is the account used to run the operation
                    // if not set, the runtime defaults to "sourceAccount" specified at
                    // the transaction level
                    Option<MuxedAccount> sourceAccount;
                    OperationType type;
                    OperationVariant content;
                };

                // TransactionV0 is a transaction with the AccountID discriminant stripped off,
                // leaving a raw ed25519 public key to identify the source account. This is used
                // for backwards compatibility starting from the protocol 12/13 boundary. If an
                // "old-style" TransactionEnvelope containing a Transaction is parsed with this
                // XDR definition, it will be parsed as a "new-style" TransactionEnvelope
                // containing a TransactionV0.
                struct TransactionV0 {
                    uint256 sourceAccountEd25519;
                    uint32_t  fee;
                    SequenceNumber seqNum;
                    Option<TimeBounds> timeBounds;
                    Memo memo;
                    std::list<Operation> operations;

                    TransactionV0() : fee(0), seqNum(0) {};
                };

                //a transaction is a container for a set of operations
                // - is executed by an account
                // - fees are collected from the account
                // - operations are executed in order as one ACID transaction
                //   either all operations are applied or none are
                //   if any returns a failing code
                struct Transaction
                {
                    // account used to run the transaction
                    MuxedAccount sourceAccount;
                    // the fee the sourceAccount will pay
                    uint32_t fee;
                    // sequence number to consume in the account
                    SequenceNumber seqNum;
                    // validity range (inclusive) for the last ledger close time
                    Option<TimeBounds> timeBounds;
                    Memo memo;
                    std::list<Operation> operations;

                    Transaction() : fee(0), seqNum(0) {};
                };

                // Decorated signature structure
                struct DecoratedSignature
                {
                    SignatureHint hint;  // last 4 bytes of the public key, used as a hint
                    Signature signature; // actual signature
                };

                // Transaction envelope type enum
                enum class EnvelopeType : uint32_t
                {
                    ENVELOPE_TYPE_TX_V0 = 0,
                    ENVELOPE_TYPE_TX = 2,
                    ENVELOPE_TYPE_TX_FEE_BUMP = 54
                };

                // Transaction envelope structure V0
                struct TransactionV0Envelope {
                    TransactionV0 tx;
                    /* Each decorated signature is a signature over the SHA256 hash of
                     * a TransactionSignaturePayload */
                    std::list<DecoratedSignature> signatures;
                };

                // Transaction envelope structure V1
                struct TransactionV1Envelope {
                    Transaction tx;
                    /* Each decorated signature is a signature over the SHA256 hash of
                     * a TransactionSignaturePayload */
                    std::list<DecoratedSignature> signatures;
                };

                struct FeeBumpTransaction {
                    MuxedAccount feeSource;
                    int64_t fee;
                    std::list<Operation> operations;
                };

                // Transaction envelope structure for FeeBumpTransactions
                struct FeeBumpTransactionEnvelope {
                    FeeBumpTransaction tx;
                    /* Each decorated signature is a signature over the SHA256 hash of
                     * a TransactionSignaturePayload */
                    std::list<DecoratedSignature> signatures;
                };

                using TransactionEnvelopeVariant = boost::variant<
                        TransactionV0Envelope, TransactionV1Envelope, FeeBumpTransactionEnvelope>;

                // A TransactionEnvelope wraps a transaction with signatures.
                struct TransactionEnvelope {
                    EnvelopeType type;
                    TransactionEnvelopeVariant content;
                };

            }
        }
    }
}

#endif //LEDGER_CORE_MODELS_HPP
