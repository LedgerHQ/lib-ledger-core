/*
 * AlgorandExplorerConstants
 *
 * Created by Hakim Aammar on 19/05/2020.
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

#ifndef LEDGER_CORE_ALGORANDEXPLORERCONSTANTS_H
#define LEDGER_CORE_ALGORANDEXPLORERCONSTANTS_H

#include <string>

namespace ledger {
namespace core {
namespace algorand {
namespace constants {

    static constexpr uint32_t EXPLORER_QUERY_LIMIT = 100; // Max nb of objects returned by queries - defined by Algorand

    // Block
    /// The block hash is not returned by algod v2 anymore
    // static const std::string xHash = "hash";
    static const std::string xTimestamp = "timestamp";
    static const std::string xRound = "round";

    // AssetParam
    static const std::string xClawbackAddr = "clawback";
    static const std::string xCreator = "creator";
    static const std::string xDecimal = "decimals";
    static const std::string xDefaultFrozen = "default-frozen";
    static const std::string xFreezeAddr = "freeze";
    static const std::string xManagerKey = "manager";
    static const std::string xMetadataHash = "metadata-hash";
    static const std::string xAssetName = "name";
    static const std::string xReserveAddr = "reserve";
    static const std::string xTotal = "total";
    static const std::string xUnitName = "unit-name";
    static const std::string xUrl = "url";

    // AssetAmount
    static const std::string xAmount = "amount";
    static const std::string xFrozen = "is-frozen";

    // Account
    static const std::string xAddress = "address";
    static const std::string xPendingRewards = "pending-rewards";
    static const std::string xAmountWithoutPendingRewards = "amount-without-pending-rewards";
    static const std::string xRewards = "rewards";
    static const std::string xStatus = "status";
    static const std::string xAssets = "assets";
    static const std::string xCreatedAssets = "created-assets";
    static const std::string xParticipation = "participation";

    // Header txn
    static const std::string xConfirmedRound = "confirmed-round";
    static const std::string xRoundTime = "round-time";
    static const std::string xTxType = "tx-type";
    static const std::string xId = "id";
    static const std::string xSender = "sender";
    static const std::string xFee = "fee";
    static const std::string xFirstValid = "first-valid";
    static const std::string xLastValid = "last-valid";
    static const std::string xNote = "note";
    static const std::string xGenesisId = "genesis-id";
    static const std::string xGenesisHash = "genesis-hash";
    static const std::string xGroup = "group";
    static const std::string xLease = "lease";

    // Payment txn
    static const std::string xReceiver = "receiver";
    static const std::string xToRewards = "receiver-rewards";
    static const std::string xCloseAmount = "close-amount";
    static const std::string xCloseRewards = "close-rewards";
    static const std::string xFromRewards = "sender-rewards";
    static const std::string xCloseTo = "close-to";
    static const std::string xCloseRemainderTo = "close-remainder-to";

    // Keyreg txn
    static const std::string xNonParticipation = "non-participation";
    static const std::string xSelkey = "selection-participation-key";
    static const std::string xVotefst = "vote-first-valid";
    static const std::string xVotekd = "vote-key-dilution";
    static const std::string xVotekey = "vote-participation-key";
    static const std::string xVotelst = "vote-last-valid";

    // AssetConfig txn
    static const std::string xAssetId = "asset-id";
    static const std::string xParams = "params";
    static const std::string xIndex = "index";

    // AssetFreeze txn
    static const std::string xNewFreezeStatus = "new-freeze-status";

    static const std::string xPay = "pay";
    static const std::string xKeyregs = "keyreg";
    static const std::string xAcfg = "acfg";
    static const std::string xAxfer = "axfer";
    static const std::string xAfreeze = "afrz";

    static const std::string xPayment = "payment-transaction";
    static const std::string xKeyreg = "keyreg-transaction";
    static const std::string xCurcfg = "asset-config-transaction";
    static const std::string xCurxfer = "asset-transfer-transaction";
    static const std::string xCurfrz = "asset-freeze-transaction";

    static const std::string xTransactions = "transactions";
    static const std::string xTxId = "txId";
    static const std::string xMinFee = "min-fee";
    static const std::string xConsensusVersion = "consensus-version";
    static const std::string xLastRoundParam = "last-round";

} // namespace constants
} // namespace algorand
} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_ALGORANDEXPLORERCONSTANTS_H

