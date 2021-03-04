#include "../integration/BaseFixture.h"
#include "Fixtures.hpp"

#include <gtest/gtest.h>
#include <rapidjson/document.h>
#include <utils/DateUtils.hpp>
#include <wallet/cosmos/explorers/StargateRpcsParsers.hpp>

using namespace ledger::testing::cosmos;

// TODO Find a good delegations entry as well to test that
TEST(CosmosGaiaParser, StargateDelegation) {
  // Input data
  // Output of
  // /cosmos/staking/v1beta1/delegations/cosmos1q9wtnlwdjrhwtcjmt2uq77jrgx7z3usrq2yz7z
  // the "delegation_responses" part has been removed because the wrapping list abstraction
  // is handled by the getDelegations code
  const std::string explorerResponse =
      // clang-format off
R"json(
    {
      "delegation": {
        "delegator_address": "cosmos1q9wtnlwdjrhwtcjmt2uq77jrgx7z3usrq2yz7z",
        "validator_address": "cosmosvaloper1h2gacd88hkvlmz5g04md87r54kjf0klntl7plk",
        "shares": "2005431.159813209848672109"
      },
      "balance": {
        "denom": "umuon",
        "amount": "2000000"
      }
    }
)json";
  // clang-format on

  // Execution of tested function
  rapidjson::Document delegationJSON;
  delegationJSON.Parse(explorerResponse.c_str());
  cosmos::Delegation parsedResult;
  stargate_rpcs_parsers::parseDelegation(delegationJSON.GetObject(),
                                         parsedResult,
                                         currencies::MUON);

  // Assertions
  EXPECT_EQ(parsedResult.delegatedAmount.toUint64(), 2000000);
  EXPECT_EQ(parsedResult.delegatorAddress, "cosmos1q9wtnlwdjrhwtcjmt2uq77jrgx7z3usrq2yz7z");
  EXPECT_EQ(parsedResult.validatorAddress, "cosmosvaloper1h2gacd88hkvlmz5g04md87r54kjf0klntl7plk");
}


TEST(CosmosGaiaParser, StargateUnbonding) {
  // Input data
  // Output (somewhat modified to add another entry) of
  // /cosmos/staking/v1beta1/delegators/cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl/unbonding_delegations
  const std::string explorerResponse =
      // clang-format off
    R"json(
{
  "unbonding_responses": [
    {
      "delegator_address": "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl",
      "validator_address": "cosmosvaloper1x5wgh6vwye60wv3dtshs9dmqggwfx2ldk5cvqu",
      "entries": [
        {
          "creation_height": "502606",
          "completion_time": "2021-01-01T10:57:40.660598719Z",
          "initial_balance": "1000000",
          "balance": "1000000"
        },
        {
          "creation_height": "502800",
          "completion_time": "2021-01-01T11:14:58.703333936Z",
          "initial_balance": "43000000",
          "balance": "43000000"
        }
      ]
    },
    {
      "delegator_address": "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl",
      "validator_address": "cosmosvaloper1h2gacd88hkvlmz5g04md87r54kjf0klntl7plk",
      "entries": [
        {
          "creation_height": "483290",
          "completion_time": "2021-01-01T22:47:43.660598719Z",
          "initial_balance": "99999999",
          "balance": "11111111"
        }
      ]
    }
  ],
  "pagination": {
    "next_key": null,
    "total": "1"
  }
}
    )json";
  // clang-format on

  // Execution of tested function
  rapidjson::Document unbondingJSON;
  unbondingJSON.Parse(explorerResponse.c_str());
  cosmos::UnbondingList parsedResult;
  stargate_rpcs_parsers::parseUnbondingList(unbondingJSON.GetObject(),
                                            parsedResult);

  // Expected values
  const auto delegator =
      std::string("cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl");

  const auto validator_a =
      std::string("cosmosvaloper1h2gacd88hkvlmz5g04md87r54kjf0klntl7plk");
  const auto validator_b =
      std::string("cosmosvaloper1x5wgh6vwye60wv3dtshs9dmqggwfx2ldk5cvqu");
  const auto val_b_height_a = std::string("502606");
  const auto val_b_height_b = std::string("502800");

  // Assertions
  EXPECT_EQ(parsedResult.size(), 2);
  for (const auto &unbonding : parsedResult) {
    if (unbonding->validatorAddress == validator_a) {
      EXPECT_EQ(unbonding->delegatorAddress, delegator);
      EXPECT_EQ(unbonding->entries.size(), 1);
      for (const auto &entry : unbonding->entries) {
        EXPECT_EQ(entry.creationHeight.toString(), "483290");
        EXPECT_EQ(DateUtils::toJSON(entry.completionTime),
                  "2021-01-01T22:47:43Z");
        EXPECT_EQ(entry.initialBalance.toString(), "99999999");
        EXPECT_EQ(entry.balance.toString(), "11111111");
      }
    } else if (unbonding->validatorAddress == validator_b) {
      EXPECT_EQ(unbonding->delegatorAddress, delegator);
      EXPECT_EQ(unbonding->entries.size(), 2);
      for (const auto &entry : unbonding->entries) {
        if (entry.creationHeight.toString() == val_b_height_a) {
          EXPECT_EQ(DateUtils::toJSON(entry.completionTime),
                    "2021-01-01T10:57:40Z");
          EXPECT_EQ(entry.initialBalance.toString(), "1000000");
          EXPECT_EQ(entry.balance.toString(), "1000000");
        } else if (entry.creationHeight.toString() == val_b_height_b) {
          EXPECT_EQ(DateUtils::toJSON(entry.completionTime),
                    "2021-01-01T11:14:58Z");
          EXPECT_EQ(entry.initialBalance.toString(), "43000000");
          EXPECT_EQ(entry.balance.toString(), "43000000");
        } else {
          FAIL() << fmt::format(
              R"esc(Creation height "{}" for validator "{}" is not expected)esc",
              entry.creationHeight.toString(), validator_b);
        }
      }
    } else {
      FAIL() << fmt::format(R"esc(validator "{}" is not expected)esc",
                            unbonding->validatorAddress);
    }
  }
}

TEST(CosmosGaiaParser, StargateRedelegation) {
  // Input data
  // Output (somewhat modified) of
  // /cosmos/staking/v1beta1/delegators/cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl/redelegations
  const std::string explorerResponse =
      // clang-format off
    R"json(
{
  "redelegation_responses": [
    {
      "redelegation": {
        "delegator_address": "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl",
        "validator_src_address": "cosmosvaloper1x5wgh6vwye60wv3dtshs9dmqggwfx2ldk5cvqu",
        "validator_dst_address": "cosmosvaloper1h2gacd88hkvlmz5g04md87r54kjf0klntl7plk",
        "entries": null
      },
      "entries": [
        {
          "redelegation_entry": {
            "creation_height": 502590,
            "completion_time": "2021-01-01T10:56:15.231455556Z",
            "initial_balance": "2000000",
            "shares_dst": "2000000.000000000000000000"
          },
          "balance": "2000000"
        }
      ]
    }
  ],
  "pagination": {
    "next_key": null,
    "total": "1"
  }
}
    )json";
  // clang-format on

  // Execution of tested function
  rapidjson::Document redelegationJSON;
  redelegationJSON.Parse(explorerResponse.c_str());
  cosmos::RedelegationList parsedResult;
  stargate_rpcs_parsers::parseRedelegationList(redelegationJSON.GetObject(),
                                               parsedResult);

  // Expected values
  const auto delegator =
      std::string("cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl");

  const auto redelegation_a_src =
      std::string("cosmosvaloper1x5wgh6vwye60wv3dtshs9dmqggwfx2ldk5cvqu");
  const auto redelegation_a_dst =
      std::string("cosmosvaloper1h2gacd88hkvlmz5g04md87r54kjf0klntl7plk");

  // Assertions
  EXPECT_EQ(parsedResult.size(), 1);
  for (const auto &redelegation : parsedResult) {
    if (redelegation->srcValidatorAddress == redelegation_a_src &&
        redelegation->dstValidatorAddress == redelegation_a_dst) {
      EXPECT_EQ(redelegation->delegatorAddress, delegator);
      EXPECT_EQ(redelegation->entries.size(), 1);
      for (const auto &entry : redelegation->entries) {
        EXPECT_EQ(entry.creationHeight.toString(), "502590");
        EXPECT_EQ(DateUtils::toJSON(entry.completionTime),
                  "2021-01-01T10:56:15Z");
        EXPECT_EQ(entry.initialBalance.toString(), "2000000");
        EXPECT_EQ(entry.balance.toString(), "2000000");
      }
    } else {
      FAIL() << fmt::format(
          R"esc(Redelegation from validator "{}" to validator "{}"is not expected)esc",
          redelegation->srcValidatorAddress, redelegation->dstValidatorAddress);
    }
  }
}

TEST(CosmosGaiaParser, StargateBlock) {
  // Input data
  // Output (somewhat modified) of
  // /blocks/latest
  const std::string explorerResponse =
      // clang-format off
    R"json(
{
  "block_id": {
    "hash": "F2209FB15F6D3D84EB4C6393C34884AD652F4F9571CAB0AB6F16275660BB5E76",
    "parts": {
      "total": 1,
      "hash": "56BD8347BA3E35FB8A033E2C2164730FD30D3E7C739765EF4E04693EF309A8E3"
    }
  },
  "block": {
    "header": {
      "version": {
        "block": "11"
      },
      "chain_id": "stargate-5",
      "height": "77896",
      "time": "2020-11-16T14:00:49.35986899Z",
      "last_block_id": {
        "hash": "9F94106A84DF5CE967B43BF98C92FE06B1B2B458E6FC660343C7119FC6247F69",
        "parts": {
          "total": 1,
          "hash": "1F9DB2F30DB3C922943A22D64F0220BA674577DF5094854580429D6887CA6938"
        }
      },
      "last_commit_hash": "1F203E02770F211E47D545B13ADD5347ACD3BEE9B595FC797E72A39283FA7786",
      "data_hash": "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855",
      "validators_hash": "EB4DE4776FE10BE67FF4355C04041D72D0B4AC187F2E8D25C0AB2C04A1C12483",
      "next_validators_hash": "EB4DE4776FE10BE67FF4355C04041D72D0B4AC187F2E8D25C0AB2C04A1C12483",
      "consensus_hash": "048091BC7DDC283F77BFBF91D73C44DA58C3DF8A9CBC867405D8B7F3DAADA22F",
      "app_hash": "FABCDC26487FB8A45D81CAB04BAF876399BEA1307CCDCBDF93C7650582FE43D6",
      "last_results_hash": "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855",
      "evidence_hash": "E3B0C44298FC1C149AFBF4C8996FB92427AE41E4649B934CA495991B7852B855",
      "proposer_address": "AB92042548316E39D734D48CBCBF4D36BD27037B"
    },
    "data": {
      "txs": []
    },
    "evidence": {
      "evidence": []
    },
    "last_commit": {
      "height": "77895",
      "round": 0,
      "block_id": {
        "hash": "9F94106A84DF5CE967B43BF98C92FE06B1B2B458E6FC660343C7119FC6247F69",
        "parts": {
          "total": 1,
          "hash": "1F9DB2F30DB3C922943A22D64F0220BA674577DF5094854580429D6887CA6938"
        }
      },
      "signatures": [
        {
          "block_id_flag": 2,
          "validator_address": "AB92042548316E39D734D48CBCBF4D36BD27037B",
          "timestamp": "2020-11-16T14:00:49.35986899Z",
          "signature": "qevXEJKdWNZ2EFul9pfZBriYzgPBIR2I4+0nOM++wOmL+Z5xRrAMRXkV7DqqoQ0kBvlnPGCJX0kCFhvbozcaDg=="
        },
        {
          "block_id_flag": 3,
          "validator_address": "AE61EC6FA0450C6327288B946E233A88683C478A",
          "timestamp": "2020-11-16T14:00:49.585144683Z",
          "signature": "EkTFgKazWM6ucJDimOuUFGQEPnPuoQlesj1eKjBJ6JjwHdWVUtUC36t3Ge8hl0f3gjcfMwiuR/5Pm2/apu6zBA=="
        }
      ]
    }
  }
}
    )json";
  // clang-format on

  // Execution of tested function
  rapidjson::Document blockJSON;
  blockJSON.Parse(explorerResponse.c_str());
  cosmos::Block parsedResult;
  stargate_rpcs_parsers::parseBlock(blockJSON.GetObject(),
                                    currencies::MUON, parsedResult);

  // Assertions
  EXPECT_EQ(parsedResult.currencyName, currencies::MUON.name);
  EXPECT_EQ(parsedResult.hash,
            "F2209FB15F6D3D84EB4C6393C34884AD652F4F9571CAB0AB6F16275660BB5E76");
  EXPECT_EQ(parsedResult.height, 77896);
  EXPECT_EQ(parsedResult.time,
            DateUtils::fromJSON("2020-11-16T14:00:49.35986899Z"));
}

TEST(CosmosGaiaParser, StargateAccount) {
  // Input data
  // Output (somewhat modified) of
  // /cosmos/auth/v1beta1/accounts/cosmos1x5wgh6vwye60wv3dtshs9dmqggwfx2ldnqvev0
  const std::string explorerResponse =
      // clang-format off
    R"json(
{
  "account": {
    "@type": "/cosmos.auth.v1beta1.BaseAccount",
    "address": "cosmos1x5wgh6vwye60wv3dtshs9dmqggwfx2ldnqvev0",
    "pub_key": {
      "@type": "/cosmos.crypto.secp256k1.PubKey",
      "key": "AnDGSF7VFQs9VKEceS6cWhfajxh/mmv+Mbu9bCM9BDzK"
    },
    "account_number": "11",
    "sequence": "1"
  }
}
    )json";
  // clang-format on

  // Execution of tested function
  rapidjson::Document accountJSON;
  accountJSON.Parse(explorerResponse.c_str());
  cosmos::Account parsedResult;
  stargate_rpcs_parsers::parseAccount(accountJSON.GetObject(), parsedResult);

  // Assertions
  EXPECT_EQ(parsedResult.accountNumber, "11");
  EXPECT_EQ(parsedResult.address,
            "cosmos1x5wgh6vwye60wv3dtshs9dmqggwfx2ldnqvev0");
  EXPECT_EQ(parsedResult.pubkey, "cosmospub1addwnpepqfcvvjz7652sk0255yw8jt5utgta4rcc07dxhl33hw7kcgeaqs7v5p3n38j");
  EXPECT_EQ(parsedResult.sequence, "1");
  EXPECT_EQ(parsedResult.balances.size(), 0);
}

TEST(CosmosGaiaParser, StargateValidator) {
  // Input data
  // Output (somewhat modified) of
  // /cosmos/staking/v1beta1/validators?status=BOND_STATUS_BONDED&pagination.limit=130
  // Basically taking out one validator of the list
  const std::string explorerResponse =
      // clang-format off
    R"json(
{
      "operator_address": "cosmosvaloper1x5wgh6vwye60wv3dtshs9dmqggwfx2ldk5cvqu",
      "consensus_pubkey": {
        "@type": "/cosmos.crypto.ed25519.PubKey",
        "key": "pu8GYpo9Fa8lCs5pxkXwHKRhZWE+4JhG5TxYLyq1Lr4="
      },
      "jailed": false,
      "status": "BOND_STATUS_BONDED",
      "tokens": "10000000",
      "delegator_shares": "10000000.000000000000000000",
      "description": {
        "moniker": "Cosmostation",
        "identity": "AE4C403A6E7AA1AC",
        "website": "https://www.cosmostation.io",
        "security_contact": "",
        "details": "CØSMOSTATION Validator. Delegate your tokens and Start Earning Staking Rewards"
      },
      "unbonding_height": "0",
      "unbonding_time": "1970-01-01T00:00:00Z",
      "commission": {
        "commission_rates": {
          "rate": "0.100000000000000000",
          "max_rate": "0.200000000000000000",
          "max_change_rate": "0.010000000000000000"
        },
        "update_time": "2020-11-16T10:21:02.563091132Z"
      },
      "min_self_delegation": "1"
    }
    )json";
  // clang-format on

  // Execution of tested function
  rapidjson::Document valJSON;
  valJSON.Parse(explorerResponse.c_str());
  cosmos::Validator parsedResult;
  stargate_rpcs_parsers::parseValidatorSetEntry(valJSON.GetObject(), parsedResult);

  // Assertions
  EXPECT_EQ(parsedResult.activeStatus, "BOND_STATUS_BONDED");
  EXPECT_EQ(parsedResult.operatorAddress, "cosmosvaloper1x5wgh6vwye60wv3dtshs9dmqggwfx2ldk5cvqu");
  // This address has been manually validated by checking the pre-stargate endpoint :
  // https://cosmos.coin.ledger.com/slashing/validators/cosmosvalconspub1zcjduepq5mhsvc5685267fg2ee5uv30srjjxzetp8msfs3h983vz724496lqtaz884/signing_info
  //
  // Which answers
  // {
  //   "codespace":"slashing",
  //   "code":106,
  //   "message":"no signing info found for address: cosmosvalcons14es7cmaqg5xxxfeg3w2xuge63p5rc3u2vt8ym4"
  // }
  //
  // And cosmosvalcons14es7cmaqg5xxxfeg3w2xuge63p5rc3u2vt8ym4 is exactly the consensus address
  // for this test validator as could be seen on stargate-5 using tron-poc endpoint
  // (see CosmosAddress.CosmosValConsAddressFromBase64ValConsPub test source for explanation)
  EXPECT_EQ(parsedResult.consensusPubkey,
            "cosmosvalconspub1zcjduepq5mhsvc5685267fg2ee5uv30srjjxzetp8msfs3h983vz724496lqtaz884");
  EXPECT_EQ(parsedResult.jailed, false);
  EXPECT_EQ(parsedResult.validatorDetails.moniker, "Cosmostation");
  EXPECT_EQ(parsedResult.validatorDetails.identity.value_or("FAIL"), "AE4C403A6E7AA1AC");
  EXPECT_EQ(parsedResult.validatorDetails.website.value_or("FAIL"), "https://www.cosmostation.io");
  EXPECT_EQ(parsedResult.validatorDetails.securityContact.value_or("FAIL"), "");
  EXPECT_EQ(parsedResult.validatorDetails.details.value_or("FAIL"), "CØSMOSTATION Validator. Delegate your tokens and Start Earning Staking Rewards");
  EXPECT_EQ(parsedResult.unbondingHeight, 0);
  EXPECT_EQ(static_cast<bool>(parsedResult.unbondingTime), false);
  EXPECT_EQ(parsedResult.minSelfDelegation, "1");
  EXPECT_EQ(parsedResult.commission.updateTime,
           DateUtils::fromJSON("2020-11-16T10:21:02.563091132Z"));
  EXPECT_EQ(parsedResult.commission.rates.maxChangeRate, "0.010000000000000000");
  EXPECT_EQ(parsedResult.commission.rates.maxRate, "0.200000000000000000");
  EXPECT_EQ(parsedResult.commission.rates.rate, "0.100000000000000000");
  EXPECT_EQ(parsedResult.votingPower, "10000000");
}

TEST(CosmosGaiaParser, StargateSigningInfos) {
  // Input data
  // Output (somewhat modified) of
  // /cosmos/slashing/v1beta1/signing_infos/cosmosvalcons14es7cmaqg5xxxfeg3w2xuge63p5rc3u2vt8ym4
  const std::string explorerResponse =
      // clang-format off
    R"json(
{
  "val_signing_info": {
    "address": "cosmosvalcons14es7cmaqg5xxxfeg3w2xuge63p5rc3u2vt8ym4",
    "start_height": "75270",
    "index_offset": "11836",
    "jailed_until": "1970-01-01T00:00:00Z",
    "tombstoned": false,
    "missed_blocks_counter": "0"
  }
}
    )json";
  // clang-format on

  // Execution of tested function
  rapidjson::Document signingInfosJSON;
  signingInfosJSON.Parse(explorerResponse.c_str());
  cosmos::ValidatorSigningInformation parsedResult;
  stargate_rpcs_parsers::parseSignInfo(signingInfosJSON.GetObject(), parsedResult);

  // Assertions
  EXPECT_EQ(parsedResult.indexOffset, 11836);
  EXPECT_EQ(parsedResult.jailedUntil, DateUtils::fromJSON("1970-01-01T00:00:00Z"));
  EXPECT_EQ(parsedResult.missedBlocksCounter, 0);
  EXPECT_EQ(parsedResult.startHeight, 75270);
  EXPECT_EQ(parsedResult.tombstoned, false);
}


TEST(CosmosGaiaParser, StargateTransaction) {
  // Input data
  // Output extracted from
  //
  const std::string explorerResponse =
      R"json(
{
  "height": "267373",
  "txhash": "C6B492851403B92B849672DD15A849A7F1A173AA6DE159B6E3691385E9907B33",
  "data": "0A060A0473656E64",
  "raw_log": "[{\"events\":[{\"type\":\"message\",\"attributes\":[{\"key\":\"action\",\"value\":\"send\"},{\"key\":\"sender\",\"value\":\"cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl\"},{\"key\":\"module\",\"value\":\"bank\"}]},{\"type\":\"transfer\",\"attributes\":[{\"key\":\"recipient\",\"value\":\"cosmos108uy5q9jt59gwugq5yrdhkzcd9jryslmpcstk5\"},{\"key\":\"sender\",\"value\":\"cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl\"},{\"key\":\"amount\",\"value\":\"1000000umuon\"}]}]}]",
  "logs": [
    {
      "events": [
        {
          "type": "message",
          "attributes": [
            {
              "key": "action",
              "value": "send"
            },
            {
              "key": "sender",
              "value": "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl"
            },
            {
              "key": "module",
              "value": "bank"
            }
          ]
        },
        {
          "type": "transfer",
          "attributes": [
            {
              "key": "recipient",
              "value": "cosmos108uy5q9jt59gwugq5yrdhkzcd9jryslmpcstk5"
            },
            {
              "key": "sender",
              "value": "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl"
            },
            {
              "key": "amount",
              "value": "1000000umuon"
            }
          ]
        }
      ]
    }
  ],
  "gas_wanted": "200000",
  "gas_used": "64177",
  "tx": {
    "type": "cosmos-sdk/StdTx",
    "value": {
      "msg": [
        {
          "type": "cosmos-sdk/MsgSend",
          "value": {
            "from_address": "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl",
            "to_address": "cosmos108uy5q9jt59gwugq5yrdhkzcd9jryslmpcstk5",
            "amount": [
              {
                "denom": "umuon",
                "amount": "1000000"
              }
            ]
          }
        }
      ],
      "fee": {
        "amount": [
          {
            "denom": "umuon",
            "amount": "0"
          }
        ],
        "gas": "200000"
      },
      "signatures": [
        {
          "pub_key": {
            "type": "tendermint/PubKeySecp256k1",
            "value": "A4hFmyZTUZlIsSSS8aC0ZHIBEMFHqBVdI9Qjpcw8Idia"
          },
          "signature": "udk17K0/mMsPNAj+0vX5laQpCYjxtgasFmMlbcYWN3kJmuvRwMEQqLXPI6z4EgjPKeI82G0YMSPRzAlw2Um10Q=="
        }
      ],
      "memo": "Ledger Live",
      "timeout_height": "0"
    }
  },
  "timestamp": "2020-11-27T15:49:28Z"
}
)json";
  // Execution of tested function
  rapidjson::Document txJSON;
  txJSON.Parse(explorerResponse.c_str());
  cosmos::Transaction parsedResult;
  stargate_rpcs_parsers::parseTransaction(txJSON.GetObject(), parsedResult, currencies::MUON);

  // Assertions
  EXPECT_EQ(parsedResult.block.getValue().height, 267373);
  EXPECT_EQ(parsedResult.block.getValue().currencyName, currencies::MUON.name);
  EXPECT_EQ(parsedResult.timestamp, DateUtils::fromJSON("2020-11-27T15:49:28Z"));
  EXPECT_EQ(parsedResult.fee.amount.size(), 1);
  EXPECT_EQ(parsedResult.fee.gas.toUint64(), 200000);
  EXPECT_EQ(parsedResult.gasUsed.getValue().toUint64(), 64177);
  EXPECT_EQ(parsedResult.hash, "C6B492851403B92B849672DD15A849A7F1A173AA6DE159B6E3691385E9907B33");
  EXPECT_EQ(parsedResult.memo, "Ledger Live");
  EXPECT_EQ(parsedResult.messages.size(), 1)
      << "There is only 1 message, the internal MsgFees is only added when the "
         "explorer fetches the transaction and runs the post-processing.";
  EXPECT_EQ(parsedResult.messages[0].type, "cosmos-sdk/MsgSend");
}
