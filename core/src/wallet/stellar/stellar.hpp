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
            enum class SignerType {ED25519_PUBLIC_KEY, SHA256_HASH, PREAUTH_TX};

            struct Ledger {
                std::string   hash;
                std::string   previousHash;
                std::string   pagingToken;
                std::uint32_t transactionCount;
                std::uint32_t successfulTransactionCount;
            };

            struct Balance {
                BigInt value;
                Option<BigInt> limit;
                Option<BigInt> buyingLiabilities;
                Option<BigInt> sellingLiabilities;
                std::string assetType;
                uint64_t lastModifiedLedger;
                Option<std::string> assetCode;
                Option<std::string> assetIssuer;
            };

            struct Signer {
                int32_t weight;
                std::string key;
                SignerType type;
            };

            struct Thresholds {
                uint32_t low;
                uint32_t mid;
                uint32_t high;
            };

            struct Flags {
                bool authImmutable;
                bool authRequired;
                bool authRevocable;
            };

            struct Account {
                std::string accountId;
                std::string sequence;
                uint32_t subentryCount;
                Option<std::string> inflationDestination;
                std::vector<Balance> balances;
                std::vector<Signer> signers;
                Thresholds thresholds;
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
                uint64_t id;
                Option<std::string> pagingToken;
                bool transactionSuccessful;
                OperationType type;
            };

            struct Transaction {
                std::string id;
                Option<std::string> pagingToken;
                bool successful;
                std::string hash;
                uint64_t ledger;
                std::chrono::system_clock::time_point createdAt;
                std::string sourceAccount;
                std::string sourceAccountSequence;
                BigInt feePaid;
                std::vector<Operation> operations;
                XDRData  envelope;
                XDRData result;
                XDRData resultMeta;
                XDRData feeMeta;
                std::string memoType;
                std::string memo;
                std::vector<std::string> signatures;
            };
        }
    }
}

#endif //LEDGER_CORE_STELLARLIKE_STELLAR_HPP
