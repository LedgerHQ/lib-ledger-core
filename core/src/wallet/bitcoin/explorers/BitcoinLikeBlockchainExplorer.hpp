/*
 *
 * BitcoinLikeBlockchainExplorer
 * ledger-core
 *
 * Created by Pierre Pollastri on 17/01/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#ifndef LEDGER_CORE_BITCOINLIKEBLOCKCHAINEXPLORER_HPP
#define LEDGER_CORE_BITCOINLIKEBLOCKCHAINEXPLORER_HPP

#include <string>
#include <chrono>
#include <vector>
#include "../../../utils/optional.hpp"
#include "../../../api/ErrorCode.hpp"
#include "../../../utils/Option.hpp"
#include "../../../async/Future.hpp"
#include "../../../collections/collections.hpp"

namespace ledger {
    namespace core {

        /*
         *  {
             "hash":"9fdbe15a16fe282291426df15894ab1473e252bc31f244e4d923a17e11743eda",
             "received_at":"2016-03-23T11:54:21Z",
             "lock_time":0,
             "block":{
                "hash":"0000000000000000026aa418ef33e0b079a42d348f35bc0a2fa4bc150a9c459d",
                "height":403912,
                "time":"2016-03-23T11:54:21Z"
             },
             "inputs":[
                {
                   "output_hash":"c74e3063a385d486b2add4b542a7e900c9bd4501a9f37f5e662bdd4b83fd6e02",
                   "output_index":1,
                   "input_index":0,
                   "value":1634001,
                   "address":"1Nd2kJid5fFmPks9KSRpoHQX4VpkPhuATm",
                   "script_signature":"483045022100f72c86ce3bc364c7c45904fa19b44116fc5935f7b25bf9ce290b55dd6e5e2fa602203109dbaf83bfd987c17c5753d3be1b6b6ce9397b66fe04fd72fb299362d122ba01210389e6255cc4c5245d58bb0d074541f392a7e577c004c7529525abe7c3352f77cc"
                }
             ],
             "outputs":[
                {
                   "output_index":0,
                   "value":1000,
                   "address":"1QKJghDW4kLqCsH2pq3XKKsSSeYNPcL5PD",
                   "script_hex":"76a914ffc1247b4de5e6bfd0c632c9f5d74aa2bc9bda5e88ac"
                },
                {
                   "output_index":1,
                   "value":1621651,
                   "address":"19j8biFtMSy5HFRX6mXiurjz3jszg7nLN5",
                   "script_hex":"76a9145fb8d1ce006aeca54d0bbb6355233dcd5885a67f88ac"
                }
             ],
             "fees":11350,
             "amount":1622651
         }
         */


        class BitcoinLikeBlockchainExplorer {
        public:
            struct Block {
                std::string hash;
                uint64_t height;
                std::chrono::system_clock::time_point time;
            };

            struct Input {
                uint32_t index;
                optional<uint64_t> value;
                optional<std::string> previousTxHash;
                optional<uint32_t> previousTxOutputIndex;
                optional<std::string> address;
                optional<std::string> signatureScript;
                optional<std::string> coinbase;
            };

            struct Output {
                uint32_t index;
                uint64_t value;
                optional<std::string> address;
                std::string script;
            };

            struct Transaction {
                std::string hash;
                std::chrono::system_clock::time_point receivedAt;
                uint64_t lockTime;
                optional<Block> block;
                std::vector<Input> inputs;
                std::vector<Output> outputs;
            };

            struct TransactionsBulk {
                std::vector<Transaction> transactions;
                bool hasNext;
            };


        public:
            virtual Future<void *> startSession() = 0;
            virtual Future<Unit> killSession(void *session) = 0;

            virtual Future<TransactionsBulk> getTransactions(
                    const std::vector<std::string>& addresses,
                    Option<std::string> fromBlockHash = Option<std::string>(),
                    Option<void*> session = Option<void *>()
            ) = 0;

            virtual Future<Block> getCurrentBlock() = 0;
            virtual Future<Bytes> getRawTransaction(const String& transactionHash) = 0;
            virtual Future<Transaction> getTransactionByHash(const String& transactionHash) = 0;
            virtual Future<String> pushTransaction(const std::vector<uint8_t>& transaction) = 0;
        };
    }
}


#endif //LEDGER_CORE_BITCOINLIKEBLOCKCHAINEXPLORER_HPP
