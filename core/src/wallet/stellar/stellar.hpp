/*
 *
 * ledger-core
 *
 * Created by Pierre Pollastri.
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

#ifndef LEDGER_CORE_STELLARLIKE_STELLAR_HPP
#define LEDGER_CORE_STELLARLIKE_STELLAR_HPP

#include <utils/Option.hpp>
#include <cstdint>
#include <unordered_map>
#include <math/BigInt.h>
#include <chrono>

namespace ledger {
    namespace core {
        namespace stellar {

            using XDRData = std::vector<uint8_t>;

            struct Ledger {
                std::string   hash;
                uint64_t height;
                std::chrono::system_clock::time_point time;
                BigInt baseFee;
                BigInt baseReserve;
            };

            struct FeeStats {
                std::string lastLedger;
                BigInt lastBaseFee;
                BigInt modeAcceptedFee;
                BigInt minAccepted;
                BigInt maxFee;
            };

            struct Balance {
                BigInt value;
                Option<BigInt> buyingLiabilities;
                Option<BigInt> sellingLiabilities;
                std::string assetType;
                uint64_t lastModifiedLedger;
                Option<std::string> assetCode;
                Option<std::string> assetIssuer;
            };

            struct Flags {
                bool authImmutable;
                bool authRequired;
                bool authRevocable;

            };

            struct Account {
                std::string accountId;
                uint32_t accountIndex;
                std::string sequence;
                uint32_t subentryCount;
                std::vector<Balance> balances;
                std::unordered_map<std::string, std::string> data;
                Flags flags;
             };

            struct Asset {
                std::string type;
                std::string code;
                std::string issuer;
                Flags flags;
            };

            enum class OperationType : uint8_t  {CREATE_ACCOUNT = 0, PAYMENT = 1, PATH_PAYMENT = 2, MANAGER_OFFER = 3, CREATE_PASSIVE_OFFER = 4, SET_OPTIONS = 5,
                                                 CHANGE_TRUST = 6, ALLOW_TRUST = 7, ACCOUNT_MERGE = 8, INFLATION = 9, MANAGE_DATA = 10, BUMP_SEQUENCE = 11};

            struct Operation {
                std::string id;
                std::string pagingToken;
                bool transactionSuccessful;
                OperationType type;
                Asset asset;
                BigInt amount;
                std::string from;
                std::string to;
                std::string transactionHash;
                std::chrono::system_clock::time_point createdAt;

                Option<Asset> sourceAsset;
                Option<BigInt> sourceAmount;
            };

            struct Transaction {
                bool successful;
                std::string hash;
                uint64_t ledger;
                std::chrono::system_clock::time_point createdAt;
                std::string sourceAccount;
                BigInt sourceAccountSequence;
                BigInt feePaid;
                std::string memoType;
                std::string memo;
                std::string pagingToken;
            };

            using OperationVector = std::vector<std::shared_ptr<Operation>>;
            using TransactionVector = std::vector<std::shared_ptr<Transaction>>;
        }
    }
}

#endif //LEDGER_CORE_STELLARLIKE_STELLAR_HPP
