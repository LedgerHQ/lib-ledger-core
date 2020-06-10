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

    // Json objects keys
    static const std::string xHash = "hash";
    static const std::string xTimestamp = "timestamp";
    static const std::string xRound = "round";

    static const std::string xCreator = "creator";
    static const std::string xAmount = "amount";
    static const std::string xFrozen = "frozen";
    static const std::string xTotal = "total";
    static const std::string xDecimal = "decimals";
    static const std::string xUnitName = "unitname";
    static const std::string xAssetName = "assetname";
    static const std::string xManagerKey = "managerkey";
    static const std::string xFreezeAddr = "freezeaddr";
    static const std::string xClawbackAddr = "clawbackaddr";
    static const std::string xReserveAddr = "reserveaddr";
    static const std::string xMetadataHash = "metadatahash";
    static const std::string xDefaultFrozen = "defaultfrozen";
    static const std::string xUrl = "url";

    static const std::string xAddress = "address";
    static const std::string xPendingRewards = "pendingrewards";
    static const std::string xAmountWithoutPendingRewards = "amountwithoutpendingrewards";
    static const std::string xRewards = "rewards";
    static const std::string xStatus = "status";
    static const std::string xAssets = "assets";
    static const std::string xThisAssetTotal = "thisassettotal";
    static const std::string xParticipation = "participation";

    static const std::string xTo = "to";
    static const std::string xToRewards = "torewards";
    static const std::string xClose = "close";
    static const std::string xCloseAmount = "closeamount";
    static const std::string xCloseRewards = "closerewards";

    static const std::string xSelkey = "selkey";
    static const std::string xVotefst = "votefst";
    static const std::string xVotekd = "votekd";
    static const std::string xVotekey = "votekey";
    static const std::string xVotelst = "votelst";

    static const std::string xId = "id";
    static const std::string xParams = "params";

    static const std::string xAmt = "amt";
    static const std::string xRcv = "rcv";
    static const std::string xSnd = "snd";
    static const std::string xCloseTo = "closeto";

    static const std::string xAcct = "acct";
    static const std::string xFreeze = "freeze";

    static const std::string xType = "type";
    static const std::string xTx = "tx";
    static const std::string xFrom = "from";
    static const std::string xFee = "fee";
    static const std::string xFirstRound = "first-round";
    static const std::string xLastRound = "last-round";
    static const std::string xNoteB64 = "noteb64";
    static const std::string xFromRewards = "fromrewards";
    static const std::string xGenesisId = "genesisID";
    static const std::string xGenesisHashB64 = "genesishashb64";
    static const std::string xGroup = "group";
    static const std::string xLease = "lease";

    static const std::string xPay = "pay";
    static const std::string xKeyregs = "keyreg";
    static const std::string xAcfg = "acfg";
    static const std::string xAxfer = "axfer";
    static const std::string xAfreeze = "afrz";

    static const std::string xPayment = "payment";
    static const std::string xKeyreg = "keyreg";
    static const std::string xCurcfg = "curcfg";
    static const std::string xCurxfer = "curxfer";
    static const std::string xCurfrz = "curfrz";

    static const std::string xTransactions = "transactions";
    static const std::string xTxId = "txId";
    static const std::string xMinFee = "fee";
    static const std::string xConsensusVersion = "consensusVersion";
    static const std::string xLastRoundParam = "lastRound";

} // namespace constants
} // namespace algorand
} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_ALGORANDEXPLORERCONSTANTS_H

