#include <integration/BaseFixture.hpp>
#include "Fixtures.hpp"

#include <gtest/gtest.h>
#include <rapidjson/document.h>
#include <core/utils/DateUtils.hpp>
#include <cosmos/explorers/RpcsParsers.hpp>

using namespace ledger::testing::cosmos;

TEST(CosmosGaiaParser, Unbonding)
{
    // Input data
    // Output (somewhat modified to add another entry) of
    // /staking/delegators/cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl/unbonding_delegations
    const std::string explorerResponse =
        // clang-format off
    R"json(
    {
      "height": "1356360",
      "result": [
        {
          "delegator_address": "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl",
          "validator_address": "cosmosvaloper1vf44d85es37hwl9f4h9gv0e064m0lla60j9luj",
          "entries": [
            {
              "creation_height": "1346685",
              "completion_time": "2020-04-21T12:28:37.550789506Z",
              "initial_balance": "20000",
              "balance": "20000"
            }
          ]
        },
        {
          "delegator_address": "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl",
          "validator_address": "cosmosvaloper1clpqr4nrk4khgkxj78fcwwh6dl3uw4epsluffn",
          "entries": [
            {
              "creation_height": "1177125",
              "completion_time": "2020-04-07T10:39:59.296697797Z",
              "initial_balance": "500",
              "balance": "500"
            },
            {
              "creation_height": "1346685",
              "completion_time": "2020-04-21T12:28:37.550789506Z",
              "initial_balance": "7000",
              "balance": "6500"
            }
          ]
        }
      ]
    }
    )json";
    // clang-format on

    // Execution of tested function
    rapidjson::Document unbondingJSON;
    unbondingJSON.Parse(explorerResponse.c_str());
    cosmos::UnbondingList parsedResult;
    rpcs_parsers::parseUnbondingList(unbondingJSON.GetObject(), parsedResult);

    // Expected values
    const auto delegator = std::string("cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl");

    const auto validator_a = std::string("cosmosvaloper1vf44d85es37hwl9f4h9gv0e064m0lla60j9luj");
    const auto validator_b = std::string("cosmosvaloper1clpqr4nrk4khgkxj78fcwwh6dl3uw4epsluffn");
    const auto val_b_height_a = std::string("1177125");
    const auto val_b_height_b = std::string("1346685");

    // Assertions
    EXPECT_EQ(parsedResult.size(), 2);
    for (const auto &unbonding : parsedResult) {
        if (unbonding->validatorAddress == validator_a) {
            EXPECT_EQ(unbonding->delegatorAddress, delegator);
            EXPECT_EQ(unbonding->entries.size(), 1);
            for (const auto &entry : unbonding->entries) {
                EXPECT_EQ(entry.creationHeight.toString(), "1346685");
                EXPECT_EQ(DateUtils::toJSON(entry.completionTime), "2020-04-21T12:28:37Z");
                EXPECT_EQ(entry.initialBalance.toString(), "20000");
                EXPECT_EQ(entry.balance.toString(), "20000");
            }
        }
        else if (unbonding->validatorAddress == validator_b) {
            EXPECT_EQ(unbonding->delegatorAddress, delegator);
            EXPECT_EQ(unbonding->entries.size(), 2);
            for (const auto &entry : unbonding->entries) {
                if (entry.creationHeight.toString() == val_b_height_a) {
                    EXPECT_EQ(DateUtils::toJSON(entry.completionTime), "2020-04-07T10:39:59Z");
                    EXPECT_EQ(entry.initialBalance.toString(), "500");
                    EXPECT_EQ(entry.balance.toString(), "500");
                }
                else if (entry.creationHeight.toString() == val_b_height_b) {
                    EXPECT_EQ(DateUtils::toJSON(entry.completionTime), "2020-04-21T12:28:37Z");
                    EXPECT_EQ(entry.initialBalance.toString(), "7000");
                    EXPECT_EQ(entry.balance.toString(), "6500");
                }
                else {
                    FAIL() << fmt::format(
                        R"esc(Creation height "{}" for validator "{}" is not expected)esc",
                        entry.creationHeight.toString(),
                        validator_b);
                }
            }
        }
        else {
            FAIL() << fmt::format(
                R"esc(validator "{}" is not expected)esc", unbonding->validatorAddress);
        }
    }
}

TEST(CosmosGaiaParser, Redelegation)
{
    // Input data
    // Output (somewhat modified) of
    // /staking/redelegations?delegator=cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl
    const std::string explorerResponse =
        // clang-format off
    R"json(
    {
      "height": "1356355",
      "result": [
        {
          "delegator_address": "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl",
          "validator_src_address": "cosmosvaloper1sjllsnramtg3ewxqwwrwjxfgc4n4ef9u2lcnj0",
          "validator_dst_address": "cosmosvaloper1clpqr4nrk4khgkxj78fcwwh6dl3uw4epsluffn",
          "entries": [
            {
              "creation_height": 1107334,
              "completion_time": "2020-04-01T15:46:03.941380099Z",
              "initial_balance": "1850",
              "shares_dst": "1850.000000000000000000",
              "balance": "1850"
            }
          ]
        },
        {
          "delegator_address": "cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl",
          "validator_src_address": "cosmosvaloper1ey69r37gfxvxg62sh4r0ktpuc46pzjrm873ae8",
          "validator_dst_address": "cosmosvaloper1vf44d85es37hwl9f4h9gv0e064m0lla60j9luj",
          "entries": [
            {
              "creation_height": 1178354,
              "completion_time": "2020-04-07T13:08:44.940330001Z",
              "initial_balance": "1000000",
              "shares_dst": "1000000.000000000000000000",
              "balance": "1000000"
            }
          ]
        }
      ]
    }
    )json";
    // clang-format on

    // Execution of tested function
    rapidjson::Document redelegationJSON;
    redelegationJSON.Parse(explorerResponse.c_str());
    cosmos::RedelegationList parsedResult;
    rpcs_parsers::parseRedelegationList(redelegationJSON.GetObject(), parsedResult);

    // Expected values
    const auto delegator = std::string("cosmos1g84934jpu3v5de5yqukkkhxmcvsw3u2ajxvpdl");

    const auto redelegation_a_src =
        std::string("cosmosvaloper1sjllsnramtg3ewxqwwrwjxfgc4n4ef9u2lcnj0");
    const auto redelegation_a_dst =
        std::string("cosmosvaloper1clpqr4nrk4khgkxj78fcwwh6dl3uw4epsluffn");

    const auto redelegation_b_src =
        std::string("cosmosvaloper1ey69r37gfxvxg62sh4r0ktpuc46pzjrm873ae8");
    const auto redelegation_b_dst =
        std::string("cosmosvaloper1vf44d85es37hwl9f4h9gv0e064m0lla60j9luj");

    // Assertions
    EXPECT_EQ(parsedResult.size(), 2);
    for (const auto &redelegation : parsedResult) {
        if (redelegation->srcValidatorAddress == redelegation_a_src &&
            redelegation->dstValidatorAddress == redelegation_a_dst) {
            EXPECT_EQ(redelegation->delegatorAddress, delegator);
            EXPECT_EQ(redelegation->entries.size(), 1);
            for (const auto &entry : redelegation->entries) {
                EXPECT_EQ(entry.creationHeight.toString(), "1107334");
                EXPECT_EQ(DateUtils::toJSON(entry.completionTime), "2020-04-01T15:46:03Z");
                EXPECT_EQ(entry.initialBalance.toString(), "1850");
                EXPECT_EQ(entry.balance.toString(), "1850");
            }
        }
        else if (
            redelegation->srcValidatorAddress == redelegation_b_src &&
            redelegation->dstValidatorAddress == redelegation_b_dst) {
            EXPECT_EQ(redelegation->delegatorAddress, delegator);
            EXPECT_EQ(redelegation->entries.size(), 1);
            for (const auto &entry : redelegation->entries) {
                EXPECT_EQ(entry.creationHeight.toString(), "1178354");
                EXPECT_EQ(DateUtils::toJSON(entry.completionTime), "2020-04-07T13:08:44Z");
                EXPECT_EQ(entry.initialBalance.toString(), "1000000");
                EXPECT_EQ(entry.balance.toString(), "1000000");
            }
        }
        else {
            FAIL() << fmt::format(
                R"esc(Redelegation from validator "{}" to validator "{}"is not expected)esc",
                redelegation->srcValidatorAddress,
                redelegation->dstValidatorAddress);
        }
    }
}
