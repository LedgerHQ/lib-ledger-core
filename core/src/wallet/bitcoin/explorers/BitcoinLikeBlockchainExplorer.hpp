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
#include <utils/ConfigurationMatchable.h>
#include "../../../utils/optional.hpp"
#include "../../../api/ErrorCode.hpp"
#include "../../../utils/Option.hpp"
#include "../../../async/Future.hpp"
#include "../../../collections/collections.hpp"
#include "../../../math/BigInt.h"
#include <wallet/common/Block.h>

namespace ledger {
    namespace core {

        class BitcoinLikeBlockchainExplorer : public ConfigurationMatchable {
        public:
            typedef ledger::core::Block Block;

            struct Input {
                uint64_t index;
                Option<BigInt> value;
                Option<std::string> previousTxHash;
                Option<uint32_t> previousTxOutputIndex;
                Option<std::string> address;
                Option<std::string> signatureScript;
                Option<std::string> coinbase;
                uint32_t sequence;
                Input() {
                    sequence = 0xFFFFFFFF;
                };
            };

            struct Output {
                uint64_t index;
                std::string transactionHash;
                BigInt value;
                Option<std::string> address;
                std::string script;

                Output() = default;
            };

            struct Transaction {
                uint32_t  version;
                std::string hash;
                std::chrono::system_clock::time_point receivedAt;
                uint64_t lockTime;
                Option<Block> block;
                std::vector<Input> inputs;
                std::vector<Output> outputs;
                Option<BigInt> fees;

                Transaction() {
                    version = 1;
                }
            };

            struct TransactionsBulk {
                std::vector<Transaction> transactions;
                bool hasNext;
            };


        public:
            BitcoinLikeBlockchainExplorer(const std::shared_ptr<api::DynamicObject>& configuration, const std::vector<std::string> &matchableKeys);
            virtual Future<void *> startSession() = 0;
            virtual Future<Unit> killSession(void *session) = 0;

            virtual FuturePtr<TransactionsBulk> getTransactions(
                    const std::vector<std::string>& addresses,
                    Option<std::string> fromBlockHash = Option<std::string>(),
                    Option<void*> session = Option<void *>()
            ) = 0;

            virtual FuturePtr<Block> getCurrentBlock() = 0;
            virtual Future<Bytes> getRawTransaction(const String& transactionHash) = 0;
            virtual FuturePtr<Transaction> getTransactionByHash(const String& transactionHash) = 0;
            virtual Future<String> pushTransaction(const std::vector<uint8_t>& transaction) = 0;
        };
    }
}


#endif //LEDGER_CORE_BITCOINLIKEBLOCKCHAINEXPLORER_HPP

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

 {
    "hash": "16da85a108a63ff318458be597f34f0a7f6b9f703528249056ba2f48722ae44e",
    "received_at": "2017-04-14T11:00:58Z",
    "lock_time": 0,
    "block": {
      "hash": "0000000000000000015599c8066fa0e72847a748d0fe30e71a7a706399ec3c4a",
      "height": 461831,
      "time": "2017-04-14T11:00:58Z"
    },
    "inputs": [
      {
        "input_index": 0,
        "coinbase": "03070c070004ebabf05804496e151608bef5342d8b2800000a425720537570706f727420384d200a666973686572206a696e78696e092f425720506f6f6c2f",
        "sequence": 4294967295
      }
    ],
    "outputs": [
      {
        "output_index": 0,
        "value": 1380320309,
        "address": "1BQLNJtMDKmMZ4PyqVFfRuBNvoGhjigBKF",
        "script_hex": "76a914721afdf638d570285d02d3076d8be6a03ee0794d88ac"
      }
    ],
    "fees": 0,
    "amount": 1380320309,
    "confirmations": 1
  }

   {
    "hash": "8d2a0ccbe3a71f3e505be1557995c57f2a26f1951a72931f23a61f18fa4b3d2d",
    "received_at": "2017-04-14T11:21:03Z",
    "lock_time": 461833,
    "block": {
      "hash": "00000000000000000046aca5c4cf959924cc5073fd1ea6b16a36a18de1056b1d",
      "height": 461834,
      "time": "2017-04-14T11:44:22Z"
    },
    "inputs": [
      {
        "input_index": 0,
        "output_hash": "0c24371ecaa78121b8782a4b44144af54a89987783847cc0505d769d7e6fa0ad",
        "output_index": 16,
        "value": 254265,
        "address": "1BfxPCanNDsfJvwMvuBbh8jmrTo1i46yHs",
        "script_signature": "473044022013afa3635b5786a0d291547e7bd9201664a64005c33a15e9bd61296ffccfcc8502202c3f6eec92b9778bf866216b8607022e01ec9e429a3ca1d8ab43544302826ec2012102ed855d2fca6ed46ac9655dc567f3309d96160f6ae8f07b7eb0412f1916a484db"
      },
      {
        "input_index": 1,
        "output_hash": "521f08e684b88c747ebe7b7cf96f6217d26936a4ca7ea81d87ef66462d34243f",
        "output_index": 28,
        "value": 198000,
        "address": "1HLrhvF1AfvTAvS2LUnx8JuUdXHGtvqB2R",
        "script_signature": "483045022100ffe3f66f35f75092e94d19237612b8b40b974610f8a26196f2a197a36c96ce01022047c66ef44ad2e25aa1beaa16dbeba0c57931226c1df7991f41aab4d51f3647b6012103376312a553fc3cc7c739c2f6b4056db9143a02ef2c6b85d2223ca2fc91f09223"
      },
      {
        "input_index": 2,
        "output_hash": "521f08e684b88c747ebe7b7cf96f6217d26936a4ca7ea81d87ef66462d34243f",
        "output_index": 40,
        "value": 549000,
        "address": "1K5yNCdVpLV4pqPSf6n3Tuz9i4Y7bTBVRw",
        "script_signature": "47304402202c642e5aa73258fc186d6d106f6c07482b02a6d5239d582699fee400e6bcac1e02200421605af275b8fb7dd51faf3e4f513ad1d54ebf4b384eacbf87ba43451dadf50121027244c4c7e9f6c33a109cb062f34681f6b363f196fd33923298de10245052accf"
      },
      {
        "input_index": 3,
        "output_hash": "ba8760c80707306f1da1efe490ee92aefaac9b0b06b6e36294bfdf2e985a743b",
        "output_index": 34,
        "value": 368875,
        "address": "1PY8LGAN8SNHfTJhGpNUtoe29CXQKbaU7e",
        "script_signature": "483045022100f105205a9894268a636015fd75eca084ab79566df13f83717ca6886dfc81bb3402203248e10a0c1cebd3e64eb692f522dac18e7f7fbabfae37f470df6e9a4d6d8b86012102c603ba3cf1e9a371f1b11e35eb016fe45757a1a147cbca51e8b6077cf709bf71"
      },
      {
        "input_index": 4,
        "output_hash": "cd275d27aca317a3e2b74722726e50824aeae90dafd19ffc605948aa26762efd",
        "output_index": 23,
        "value": 296371,
        "address": "1N356RCaVsYx6mqddapd4jdsj55h6fWYEA",
        "script_signature": "47304402201bc421cd3a4586728cb0cc4bf8b3bdfb2d9a184d2cdd210053e5c6c820e7eb83022019507d5c8bdb6cf7e0dec1243a65d45982c550b791c132df9e6bcad2cc9d5cc8012103793a98b581465f4324bba5b348f40de8f8115c45f6b165a0b8e2cc0884a5c05a"
      },
      {
        "input_index": 5,
        "output_hash": "64717373eef15249771032b0153daae92d18ea63e997c1c70a33879698b43329",
        "output_index": 9,
        "value": 270000,
        "address": "1BEG75jXGZgH7QsSNjmm9RGJ2fgWcXVbxm",
        "script_signature": "483045022100b21b21023b15be3d71fc660513adc4ef1aaa299ee58b9a5c1b8401015d045622022031847f047494c83b199a743d5edd5dbeb33b2dae03dcdff12485b212061d0463012102a7e1245393aa50cf6e08077ac5f4460c2db9c54858f6b0958d91b8d62f39c3bb"
      },
      {
        "input_index": 6,
        "output_hash": "a91d56b0cb105df609ebf5ab7a5d12138e5b8b4c7850e01565b160c3803e5b2e",
        "output_index": 27,
        "value": 45000,
        "address": "1CDfKwYEzn1kMpjzZSG4cWcwsoWM99RJ8m",
        "script_signature": "483045022100af78f14417bfa55a61c1929393f63c1577aedf73c1446dcdee9fb702713be73e022061dc9b3e4ea961dc2c7d06f7925092f482da7459da4760043189407256b2904601210340b7026882b9c260bb043ffda24f891e37264f982946ed4643e341278100e125"
      },
      {
        "input_index": 7,
        "output_hash": "ddf8a92c34d4c5c109e42d7cb33aca375f6ef56d4155ebf92044f5178ed3cb62",
        "output_index": 5,
        "value": 8686071,
        "address": "1PkPrQLrNGx56ntcUv5xaPXFGCapxfVh8E",
        "script_signature": "483045022100b0ad8c0ce11f5b6fe5e0a93982744ed4d5fbc449f227f53ccfeb991091ab6c3f02206a8c6d4c46bfb55883db7528f06f203e6f5d58064045f493db583f46d0bde2e9012103839a4f321a18681fc8521e72454d39c3dd8f7dee8eb8f7e850972808ceb84e4a"
      }
    ],
    "outputs": [
      {
        "output_index": 0,
        "value": 1000012,
        "address": "14w1wdDMV5uSnBd92yf3N9LfgS6TKVzyYr",
        "script_hex": "76a9142b1fa01e4eaa2f3dee2bdbe5e6c7dde1ff42434088ac"
      },
      {
        "output_index": 1,
        "value": 7978645,
        "address": "1pCL4HJ3wbNXKiDde8eNmu9uMs1Tkd9hD",
        "script_hex": "76a91408ed01be5de10e4c9712e814c29c517a5d2af21488ac"
      }
    ],
    "fees": 1688925,
    "amount": 8978657,
    "confirmations": 1
  }

 */