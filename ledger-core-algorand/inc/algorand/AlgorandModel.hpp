/*
 * AlgorandModel
 *
 * Created by Hakim Aammar on 14/04/2020.
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


#ifndef LEDGER_CORE_ALGORANDMODEL_H
#define LEDGER_CORE_ALGORANDMODEL_H

#include <algorand/api/AlgorandTransactionType.hpp>
#include <algorand/api/AlgorandOperationType.hpp>
#include <algorand/api/AlgorandAssetParams.hpp>
#include <algorand/api/AlgorandAssetAmount.hpp>
#include <algorand/api/AlgorandPaymentInfo.hpp>
#include <algorand/api/AlgorandParticipationInfo.hpp>
#include <algorand/api/AlgorandAssetConfigurationInfo.hpp>
#include <algorand/api/AlgorandAssetTransferInfo.hpp>
#include <algorand/api/AlgorandAssetFreezeInfo.hpp>

#include <core/utils/Option.hpp>

#include <boost/variant.hpp>

#include <string>
#include <vector>
#include <map>

namespace ledger {
namespace core {
namespace algorand {
namespace model {

    using TransactionType = api::AlgorandTransactionType;
    using OperationType = api::AlgorandOperationType;

    using AssetParams = api::AlgorandAssetParams;
    using AssetAmount = api::AlgorandAssetAmount;

    using PaymentInfo = api::AlgorandPaymentInfo;
    using ParticipationInfo = api::AlgorandParticipationInfo;
    using AssetConfigurationInfo = api::AlgorandAssetConfigurationInfo;
    using AssetTransferInfo = api::AlgorandAssetTransferInfo;
    using AssetFreezeInfo = api::AlgorandAssetFreezeInfo;

    using TransactionDetails = boost::variant<PaymentInfo,
                                              ParticipationInfo,
                                              AssetConfigurationInfo,
                                              AssetTransferInfo,
                                              AssetFreezeInfo>;

    using MicroAlgoAmount = uint64_t;

    // Contains information about the side effects of a transaction
    struct TransactionResults {
        Option<uint64_t> createdAsset; // The ID of an asset created by this transaction.
    };

    struct Transaction {
        // Hash of the Transaction
        Option<std::string> id;
        // The address of the account that pays the fee and amount
        std::string senderAddress;

        // The first round (block number) for which the transaction is valid
        uint64_t firstRound;
        // The last round (block number) for which the transaction is valid
        uint64_t lastRound;
        // The block number this transaction was confirmed in
        Option<uint64_t> round;

        // Fees paid by the sender, in microAlgo
        MicroAlgoAmount fee;
        // The amount of pending rewards applied to the sender account as part of this transaction
        // FIXME What is this for?
        Option<uint64_t> fromRewards;

        // The hash of the genesis block of the network for which the transaction is valid (32 bytes in Base64)
        std::string genesisHash;
        // The human-readable string that identifies the network for the transaction
        Option<std::string> genesisId;

        // Any data up to 1000 bytes (Base64)
        Option<std::string> note;

        // Group ID for this transaction (32 bytes in Base64)
        Option<std::string> group;
        // Lease ID for this transaction (32 bytes in Base64)
        Option<std::string> lease;

        // If not empty, indicates the transaction was evicted from this node's transaction pool
        Option<std::string> poolError;

        // FIXME Make sure whether TransactionResults is a free object or belongs to a transaction
        // Contains information about the side effects of a transaction
        //Option<TransactionResults> results;

        // Type of the transaction
        TransactionType type;
        // Type-dependent information
        TransactionDetails details;

        // TODO If we want more than just single signature
        //boost:variant<SingleSignature, MultiSignature, LogicSignature> signature;
        std::string signature;
    };

    struct Account {
        // The account index
        int32_t index;
        // The round for which this account information is relevant
        uint64_t round;
        // The account public key in hexadecimal representation
        std::string pubKeyHex;
        // The account address
        std::string address;
        // The total number of MicroAlgos in the account
        MicroAlgoAmount amount;
        // The amount of MicroAlgos in the account, without the pending rewards
        MicroAlgoAmount amountWithoutPendingRewards;
        // The amount of MicroAlgos of pending rewards in this account
        MicroAlgoAmount pendingRewards;
        // The total rewards of MicroAlgos the account has received, including pending rewards
        MicroAlgoAmount rewards;
        // The amounts of assets this account contains, indexed by asset ID
        std::map<std::string, AssetAmount> assetsAmounts;
        // The parameters of assets created by this account, indexed by asset ID
        std::map<std::string, AssetParams> createdAssets;
        // Indicates the status of the account (offline|online)
        std::string status;
        // Information about the participation of this account to the blockchain consensus
        Option<ParticipationInfo> participation;
    };

    struct Block {
        // TimeStamp in seconds since epoch
        uint64_t timestamp;
        // The current round on which this block was appended to the chain
        uint64_t round;
        // The period on which the block was confirmed
        uint64_t period;
        // The address of this block proposer
        std::string proposer;
        // The current block hash
        std::string hash;
        // The previous block hash
        std::string previousBlockHash;

        // List of transactions contained in this block
        Option<std::vector<Transaction>> txns;

        // The current protocol
        std::string currentProtocol;
        // The next proposed protocol
        std::string nextProtocol;
        // The number of blocks which approved the protocol upgrade
        uint64_t nextProtocolApprovals;
        // The round on which the protocol upgrade will take effect
        uint64_t nextProtocolSwitchOn;
        // The deadline round for this protocol upgrade (No votes will be considered after this round)
        uint64_t nextProtocolVoteBefore;
        // Indicates a proposed upgrade
        std::string upgradePropose;
        // Indicates a yes vote for the current proposal
        bool upgradeApprove;

        // The sortition seed
        std::string seed;
        // Root of the Merkle tree of all transactions contained in this block.
        // Allows to authenticate the set of transactions in this block.
        std::string txnRoot;
    };

    // Contains parameters that can help a client construct a new transaction (minimum fee, suggested fee...)
    struct TransactionParams {
        // Genesis ID of the node
        std::string genesisID;
        // Genesis hash (32 bytes in Base64) of the node
        std::string genesisHash;
        // The last round seen by the node
        uint64_t lastRound;
        // The consensus protocol version (as of lastRound) applied by the node
        std::string consensusVersion;
        // The suggested transaction fee, in micro-Algos per byte
        MicroAlgoAmount suggestedFeePerByte;
        // The minimum transaction fee (not per byte) required in the current network protocol
        MicroAlgoAmount minFee;
    };

    // TODO ? https://developer.algorand.org/docs/reference/rest-apis/algod/#nodestatus
    //struct NodeStatus {};

    // TODO ? https://developer.algorand.org/docs/reference/rest-apis/algod/#supply
    // struct Supply {};

} // namespace model
} // namespace algorand
} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_ALGORANDMODEL_H
