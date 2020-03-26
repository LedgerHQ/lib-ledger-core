/*
 *
 * transactions_test
 *
 * Created by El Khalil Bellakrid on 11/06/2019.
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
#include "Fixtures.hpp"

#include <wallet/cosmos/api_impl/CosmosLikeTransactionApi.hpp>
#include <wallet/cosmos/CosmosLikeCurrencies.hpp>
#include <wallet/cosmos/CosmosLikeWallet.hpp>
#include <wallet/cosmos/CosmosLikeAccount.hpp>

#include <api/CosmosLikeTransactionBuilder.hpp>
#include <api/CosmosLikeMessage.hpp>
#include <api/StringCallback.hpp>

#include <cosmos/CosmosLikeExtendedPublicKey.hpp>

#include <utils/hex.h>
#include <utils/DateUtils.hpp>

#include <gtest/gtest.h>

using namespace ledger::testing::cosmos;
using namespace ledger::core;

TEST(CosmosTransactionTest, BuildSignedSendTxForBroadcast) {
    const std::string strTx = "{"
        "\"fee\":{\"amount\":[{\"amount\":\"5000\",\"denom\":\"uatom\"}],\"gas\":\"200000\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msg\":[{"
            "\"type\":\"cosmos-sdk/MsgSend\","
            "\"value\":{"
                "\"amount\":[{\"amount\":\"1000000\",\"denom\":\"uatom\"}],"
                "\"from_address\":\"cosmos1d9h8qat57ljhcm\","
                "\"to_address\":\"cosmos1da6hgur4wsmpnjyg\"}"
        "}],"
        "\"signatures\":[{"
            "\"pub_key\":{\"type\":\"tendermint/PubKeySecp256k1\",\"value\":\"AsS+z2hDho2VVupD1GUYtRoTyxpIzWwFohwCnqQjH83k\"},"
            "\"signature\":\"9Nn7Az62vDLW0bMgdcO26kzOeVrtd/M0GxXFsePghch7lY098oi6/MFnr0zKoeyoPxLUjCISn6JRvpVJ22WmBg==\""
        "}]}";
    const auto tx = api::CosmosLikeTransactionBuilder::parseRawSignedTransaction(currencies::ATOM, strTx);

    auto message = tx->getMessages().front();
    auto sendMessage = api::CosmosLikeMessage::unwrapMsgSend(message);

    // ensure the values are correct
    EXPECT_EQ(tx->getFee()->toLong(), 5000L);
    EXPECT_EQ(tx->getGas()->toLong(), 200000L);
    EXPECT_EQ(sendMessage.fromAddress, "cosmos1d9h8qat57ljhcm");
    EXPECT_EQ(sendMessage.toAddress, "cosmos1da6hgur4wsmpnjyg");
    EXPECT_EQ(sendMessage.amount.size(), 1);
    EXPECT_EQ(sendMessage.amount.front().amount, "1000000");
    EXPECT_EQ(sendMessage.amount.front().denom, "uatom");

    const std::string expected = "{\"mode\":\"async\",\"tx\":" + strTx + "}";
    EXPECT_EQ(tx->serializeForBroadcast(), expected);
}

TEST(CosmosTransactionTest, BuildDelegateTxForBroadcast) {
    const std::string strTx = "{"
        "\"fee\":{\"amount\":[{\"amount\":\"5000\",\"denom\":\"uatom\"}],\"gas\":\"200000\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msg\":[{"
            "\"type\":\"cosmos-sdk/MsgDelegate\","
            "\"value\":{"
                "\"amount\":{\"amount\":\"1000000\",\"denom\":\"uatom\"},"
                "\"delegator_address\":\"cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl\","
                "\"validator_address\":\"cosmosvaloper1grgelyng2v6v3t8z87wu3sxgt9m5s03xfytvz7\""
        "}}]}";
    const auto tx = api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(currencies::ATOM, strTx);

    auto message = tx->getMessages().front();
    auto delegateMessage = api::CosmosLikeMessage::unwrapMsgDelegate(message);
    EXPECT_EQ(tx->getFee()->toLong(), 5000L);
    EXPECT_EQ(tx->getGas()->toLong(), 200000L);
    EXPECT_EQ(delegateMessage.delegatorAddress, "cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl");
    EXPECT_EQ(delegateMessage.validatorAddress, "cosmosvaloper1grgelyng2v6v3t8z87wu3sxgt9m5s03xfytvz7");
    EXPECT_EQ(delegateMessage.amount.amount, "1000000");
    EXPECT_EQ(delegateMessage.amount.denom, "uatom");

    const std::string expected = "{\"mode\":\"async\",\"tx\":" + strTx + "}";
    EXPECT_EQ(tx->serializeForBroadcast(), expected);
}

TEST(CosmosTransactionTest, BuildUndelegateTxForBroadcast) {
    const std::string strTx = "{"
        "\"fee\":{\"amount\":[{\"amount\":\"5000\",\"denom\":\"uatom\"}],\"gas\":\"200000\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msg\":[{"
            "\"type\":\"cosmos-sdk/MsgUndelegate\","
            "\"value\":{"
                "\"amount\":{\"amount\":\"1000000\",\"denom\":\"uatom\"},"
                "\"delegator_address\":\"cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl\","
                "\"validator_address\":\"cosmosvaloper1grgelyng2v6v3t8z87wu3sxgt9m5s03xfytvz7\""
        "}}]}";
    const auto tx = api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(currencies::ATOM, strTx);

    auto message = tx->getMessages().front();
    auto undelegateMessage = api::CosmosLikeMessage::unwrapMsgUndelegate(message);
    EXPECT_EQ(tx->getFee()->toLong(), 5000L);
    EXPECT_EQ(tx->getGas()->toLong(), 200000L);
    EXPECT_EQ(undelegateMessage.delegatorAddress, "cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl");
    EXPECT_EQ(undelegateMessage.validatorAddress, "cosmosvaloper1grgelyng2v6v3t8z87wu3sxgt9m5s03xfytvz7");
    EXPECT_EQ(undelegateMessage.amount.amount, "1000000");
    EXPECT_EQ(undelegateMessage.amount.denom, "uatom");

    const std::string expected = "{\"mode\":\"async\",\"tx\":" + strTx + "}";
    EXPECT_EQ(tx->serializeForBroadcast(), expected);
}

TEST(CosmosTransactionTest, BuildBeginRedelegateTxForBroadcast) {
    const std::string strTx = "{"
        "\"fee\":{\"amount\":[{\"amount\":\"5000\",\"denom\":\"uatom\"}],\"gas\":\"200000\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msg\":[{"
            "\"type\":\"cosmos-sdk/MsgBeginRedelegate\","
            "\"value\":{"
                "\"amount\":{\"amount\":\"1000000\",\"denom\":\"uatom\"},"
                "\"delegator_address\":\"cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl\","
                "\"validator_dst_address\":\"cosmosvaloper1sd4tl9aljmmezzudugs7zlaya7pg2895ws8tfs\","
                "\"validator_src_address\":\"cosmosvaloper1grgelyng2v6v3t8z87wu3sxgt9m5s03xfytvz7\""
        "}}]}";
    const auto tx = api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(currencies::ATOM, strTx);

    auto message = tx->getMessages().front();
    auto redelegateMessage = api::CosmosLikeMessage::unwrapMsgBeginRedelegate(message);
    EXPECT_EQ(tx->getFee()->toLong(), 5000L);
    EXPECT_EQ(tx->getGas()->toLong(), 200000L);
    EXPECT_EQ(redelegateMessage.delegatorAddress, "cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl");
    EXPECT_EQ(redelegateMessage.validatorSourceAddress, "cosmosvaloper1grgelyng2v6v3t8z87wu3sxgt9m5s03xfytvz7");
    EXPECT_EQ(redelegateMessage.validatorDestinationAddress, "cosmosvaloper1sd4tl9aljmmezzudugs7zlaya7pg2895ws8tfs");
    EXPECT_EQ(redelegateMessage.amount.amount, "1000000");
    EXPECT_EQ(redelegateMessage.amount.denom, "uatom");

    const std::string expected = "{\"mode\":\"async\",\"tx\":" + strTx + "}";
    EXPECT_EQ(tx->serializeForBroadcast(), expected);
}

TEST(CosmosTransactionTest, BuildSubmitProposalTxForBroadcast) {
    const std::string strTx = "{"
        "\"fee\":{\"amount\":[{\"amount\":\"5000\",\"denom\":\"uatom\"}],\"gas\":\"200000\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msg\":[{"
            "\"type\":\"cosmos-sdk/MsgSubmitProposal\","
            "\"value\":{"
                "\"content\":{"
                "\"description\":\"My awesome proposal\","
                "\"title\":\"Test Proposal\","
                "\"type\":\"Text\"},"
                "\"initial_deposit\":[{\"amount\":\"1000000\",\"denom\":\"uatom\"}],"
                "\"proposer\":\"cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl\""
        "}}]}";
    const auto tx = api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(currencies::ATOM, strTx);

    auto message = tx->getMessages().front();
    auto submitProposalMessage = api::CosmosLikeMessage::unwrapMsgSubmitProposal(message);
    EXPECT_EQ(tx->getFee()->toLong(), 5000L);
    EXPECT_EQ(tx->getGas()->toLong(), 200000L);
    EXPECT_EQ(submitProposalMessage.proposer, "cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl");
    EXPECT_EQ(submitProposalMessage.content.title, "Test Proposal");
    EXPECT_EQ(submitProposalMessage.content.description, "My awesome proposal");
    EXPECT_EQ(submitProposalMessage.initialDeposit.size(), 1);
    EXPECT_EQ(submitProposalMessage.initialDeposit.front().amount, "1000000");
    EXPECT_EQ(submitProposalMessage.initialDeposit.front().denom, "uatom");

    const std::string expected = "{\"mode\":\"async\",\"tx\":" + strTx + "}";
    EXPECT_EQ(tx->serializeForBroadcast(), expected);
}

TEST(CosmosTransactionTest, BuildVoteTxForBroadcast) {
    const std::string strTx = "{"
        "\"fee\":{\"amount\":[{\"amount\":\"5000\",\"denom\":\"uatom\"}],\"gas\":\"200000\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msg\":[{"
            "\"type\":\"cosmos-sdk/MsgVote\","
            "\"value\":{"
                "\"option\":\"Yes\","
                "\"proposal_id\":\"123\","
                "\"voter\":\"cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl\""
        "}}]}";
    const auto tx = api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(currencies::ATOM, strTx);

    auto message = tx->getMessages().front();
    auto voteMessage = api::CosmosLikeMessage::unwrapMsgVote(message);
    EXPECT_EQ(tx->getFee()->toLong(), 5000L);
    EXPECT_EQ(tx->getGas()->toLong(), 200000L);
    EXPECT_EQ(to_string(voteMessage.option), "YES");
    EXPECT_EQ(voteMessage.proposalId, "123");
    EXPECT_EQ(voteMessage.voter, "cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl");

    const std::string expected = "{\"mode\":\"async\",\"tx\":" + strTx + "}";
    EXPECT_EQ(tx->serializeForBroadcast(), expected);
}

TEST(CosmosTransactionTest, BuildDepositTxForBroadcast) {
    const std::string strTx = "{"
        "\"fee\":{\"amount\":[{\"amount\":\"5000\",\"denom\":\"uatom\"}],\"gas\":\"200000\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msg\":[{"
            "\"type\":\"cosmos-sdk/MsgDeposit\","
            "\"value\":{"
                "\"amount\":[{\"amount\":\"1000000\",\"denom\":\"uatom\"}],"
                "\"depositor\":\"cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl\","
                "\"proposal_id\":\"123\""
        "}}]}";
    const auto tx = api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(currencies::ATOM, strTx);

    auto message = tx->getMessages().front();
    auto depositMessage = api::CosmosLikeMessage::unwrapMsgDeposit(message);
    EXPECT_EQ(tx->getFee()->toLong(), 5000L);
    EXPECT_EQ(tx->getGas()->toLong(), 200000L);
    EXPECT_EQ(depositMessage.proposalId, "123");
    EXPECT_EQ(depositMessage.depositor, "cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl");
    EXPECT_EQ(depositMessage.amount.size(), 1);
    EXPECT_EQ(depositMessage.amount.front().amount, "1000000");
    EXPECT_EQ(depositMessage.amount.front().denom, "uatom");

    const std::string expected = "{\"mode\":\"async\",\"tx\":" + strTx + "}";
    EXPECT_EQ(tx->serializeForBroadcast(), expected);
}

TEST(CosmosTransactionTest, BuildWithdrawDelegationRewardTxForBroadcast) {
    const std::string strTx = "{"
        "\"fee\":{\"amount\":[{\"amount\":\"5000\",\"denom\":\"uatom\"}],\"gas\":\"200000\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msg\":[{"
            "\"type\":\"cosmos-sdk/MsgWithdrawDelegationReward\","
            "\"value\":{"
                "\"delegator_address\":\"cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl\","
                "\"validator_address\":\"cosmosvaloper1grgelyng2v6v3t8z87wu3sxgt9m5s03xfytvz7\""
        "}}]}";
    const auto tx = api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(currencies::ATOM, strTx);

    auto message = tx->getMessages().front();
    auto withdrawMessage = api::CosmosLikeMessage::unwrapMsgWithdrawDelegationReward(message);
    EXPECT_EQ(tx->getFee()->toLong(), 5000L);
    EXPECT_EQ(tx->getGas()->toLong(), 200000L);
    EXPECT_EQ(withdrawMessage.delegatorAddress, "cosmos102hty0jv2s29lyc4u0tv97z9v298e24t3vwtpl");
    EXPECT_EQ(withdrawMessage.validatorAddress, "cosmosvaloper1grgelyng2v6v3t8z87wu3sxgt9m5s03xfytvz7");

    const std::string expected = "{\"mode\":\"async\",\"tx\":" + strTx + "}";
    EXPECT_EQ(tx->serializeForBroadcast(), expected);
}


TEST(CosmosTransactionTest, BuildMultiSendTxForBroadcast) {
    // From cosmos/cosmos-sdk tests :
    // https://github.com/cosmos/cosmos-sdk/blob/ebbfaf2a47d3e97a4720f643ca21d5a41676cdc0/x/bank/types/msgs_test.go#L217-L229
    const std::string strTx = "{"
        "\"fee\":{\"amount\":[{\"amount\":\"5001\",\"denom\":\"uatom\"}],\"gas\":\"200020\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msg\":[{"
            "\"type\":\"cosmos-sdk/MsgMultiSend\","
            "\"value\":{"
                "\"inputs\":[{\"address\":\"cosmos1d9h8qat57ljhcm\",\"coins\":[{\"amount\":\"10\",\"denom\":\"uatom\"}]}],"
                "\"outputs\":[{\"address\":\"cosmos1da6hgur4wsmpnjyg\",\"coins\":[{\"amount\":\"10\",\"denom\":\"atom\"}]}]"
        "}}]}";
    const auto tx = api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(currencies::ATOM, strTx);

    auto message = tx->getMessages().front();
    auto multiSendMessage = api::CosmosLikeMessage::unwrapMsgMultiSend(message);
    EXPECT_EQ(tx->getFee()->toLong(), 5001L);
    EXPECT_EQ(tx->getGas()->toLong(), 200020L);
    ASSERT_EQ(multiSendMessage.inputs.size(), 1);
    EXPECT_EQ(multiSendMessage.inputs[0].fromAddress, "cosmos1d9h8qat57ljhcm");
    EXPECT_EQ(multiSendMessage.inputs[0].coins.size(), 1);
    EXPECT_EQ(multiSendMessage.inputs[0].coins[0].amount, "10");
    EXPECT_EQ(multiSendMessage.inputs[0].coins[0].denom, "uatom");
    ASSERT_EQ(multiSendMessage.outputs.size(), 1);
    EXPECT_EQ(multiSendMessage.outputs[0].toAddress, "cosmos1da6hgur4wsmpnjyg");
    EXPECT_EQ(multiSendMessage.outputs[0].coins.size(), 1);
    EXPECT_EQ(multiSendMessage.outputs[0].coins[0].amount, "10");
    EXPECT_EQ(multiSendMessage.outputs[0].coins[0].denom, "atom");

    const std::string expected = "{\"mode\":\"async\",\"tx\":" + strTx + "}";
    EXPECT_EQ(tx->serializeForBroadcast(), expected);
}

TEST(CosmosTransactionTest, BuildCreateValidatorTxForBroadcast) {
    // TODO : find a transaction in Explorer to confirm the format here
    // For the time being we're using protobuf from cosmos-sdk as source :
    // https://github.com/cosmos/cosmos-sdk/blob/53bf2271d5bac054a8f74723732f21055c1b72d4/x/staking/types/types.pb.go
    const std::string strTx = "{"
        "\"fee\":{\"amount\":[{\"amount\":\"5001\",\"denom\":\"uatom\"}],\"gas\":\"200020\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msg\":[{"
            "\"type\":\"cosmos-sdk/MsgCreateValidator\","
            "\"value\":{"
                "\"commission\":{\"max_change_rate\":\"0.05\",\"max_rate\":\"0.60\",\"rate\":\"0.45\",\"update_time\":\"2020-02-25T13:42:29Z\"},"
                "\"delegator_address\":\"cosmostest\","
                "\"description\":{\"details\":\"It flies well\\\\nnewline\",\"identity\":\"Pocket Monsters\",\"moniker\":\"Hélédelle\",\"website\":\"https://www.pokepedia.fr/H%C3%A9l%C3%A9delle\"},"
                "\"min_self_delegation\":\"1\","
                "\"pub_key\":\"0\","
                "\"validator_address\":\"cosmosvalopertest\","
                "\"value\":{\"amount\":\"1059860\",\"denom\":\"uatom\"}"
        "}}]}";
    const auto tx = api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(currencies::ATOM, strTx);

    auto message = tx->getMessages().front();
    auto createValidatorMessage = api::CosmosLikeMessage::unwrapMsgCreateValidator(message);
    EXPECT_EQ(tx->getFee()->toLong(), 5001L);
    EXPECT_EQ(tx->getGas()->toLong(), 200020L);
    EXPECT_EQ(createValidatorMessage.description.moniker, "Hélédelle");
    EXPECT_EQ(createValidatorMessage.description.identity.value(), "Pocket Monsters");
    EXPECT_EQ(createValidatorMessage.description.website.value(), "https://www.pokepedia.fr/H%C3%A9l%C3%A9delle");
    EXPECT_EQ(createValidatorMessage.description.details.value(), "It flies well\\nnewline");
    EXPECT_EQ(createValidatorMessage.commission.rates.rate, "0.45");
    EXPECT_EQ(createValidatorMessage.commission.rates.maxRate, "0.60");
    EXPECT_EQ(createValidatorMessage.commission.rates.maxChangeRate, "0.05");
    EXPECT_EQ(createValidatorMessage.commission.updateTime, DateUtils::fromJSON("2020-02-25T13:42:29Z"));
    EXPECT_EQ(createValidatorMessage.minSelfDelegation, "1");
    EXPECT_EQ(createValidatorMessage.delegatorAddress, "cosmostest");
    EXPECT_EQ(createValidatorMessage.validatorAddress, "cosmosvalopertest");
    EXPECT_EQ(createValidatorMessage.pubkey, "0");
    EXPECT_EQ(createValidatorMessage.value.amount, "1059860");
    EXPECT_EQ(createValidatorMessage.value.denom, "uatom");

    const std::string expected = "{\"mode\":\"async\",\"tx\":" + strTx + "}";
    EXPECT_EQ(tx->serializeForBroadcast(), expected);
}

TEST(CosmosTransactionTest, BuildEditValidatorTxForBroadcast) {
    // TODO : find a transaction in Explorer to confirm the format here
    // For the time being we're using protobuf from cosmos-sdk as source :
    // https://github.com/cosmos/cosmos-sdk/blob/53bf2271d5bac054a8f74723732f21055c1b72d4/x/staking/types/types.pb.go
    const std::string strTx = "{"
        "\"fee\":{\"amount\":[{\"amount\":\"5001\",\"denom\":\"uatom\"}],\"gas\":\"200020\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msg\":[{"
            "\"type\":\"cosmos-sdk/MsgEditValidator\","
            "\"value\":{"
                "\"commission_rate\":\"0.50\","
                "\"description\":{\"details\":\"Cachabouée\",\"identity\":\"évolution de Wailmer\",\"moniker\":\"Wailord\",\"website\":\"https://www.pokepedia.fr/Wailord\"},"
                "\"min_self_delegation\":\"800\","
                "\"validator_address\":\"cosmostest\""
        "}}]}";
    const auto tx = api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(currencies::ATOM, strTx);

    auto message = tx->getMessages().front();
    auto editValidatorMessage = api::CosmosLikeMessage::unwrapMsgEditValidator(message);
    EXPECT_EQ(tx->getFee()->toLong(), 5001L);
    EXPECT_EQ(tx->getGas()->toLong(), 200020L);
    ASSERT_TRUE(editValidatorMessage.description);
    EXPECT_EQ(editValidatorMessage.description.value().moniker, "Wailord");
    ASSERT_TRUE(editValidatorMessage.description.value().identity);
    EXPECT_EQ(editValidatorMessage.description.value().identity.value(), "évolution de Wailmer");
    ASSERT_TRUE(editValidatorMessage.description.value().website);
    EXPECT_EQ(editValidatorMessage.description.value().website.value(), "https://www.pokepedia.fr/Wailord");
    ASSERT_TRUE(editValidatorMessage.description.value().details);
    EXPECT_EQ(editValidatorMessage.description.value().details.value(), "Cachabouée");
    EXPECT_EQ(editValidatorMessage.validatorAddress, "cosmostest");
    ASSERT_TRUE(editValidatorMessage.commissionRate);
    EXPECT_EQ(editValidatorMessage.commissionRate.value(), "0.50");
    ASSERT_TRUE(editValidatorMessage.minSelfDelegation);
    EXPECT_EQ(editValidatorMessage.minSelfDelegation.value(), "800");

    const std::string expected = "{\"mode\":\"async\",\"tx\":" + strTx + "}";
    EXPECT_EQ(tx->serializeForBroadcast(), expected);
}

TEST(CosmosTransactionTest, BuildSetWithdrawAddressTxForBroadcast) {
    // TODO : find a transaction in Explorer to confirm the format here
    const std::string strTx = "{"
        "\"fee\":{\"amount\":[{\"amount\":\"5001\",\"denom\":\"uatom\"}],\"gas\":\"200020\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msg\":[{"
            "\"type\":\"cosmos-sdk/MsgSetWithdrawAddress\","
            "\"value\":{"
                "\"delegator_address\":\"cosmos1dafe\","
                "\"withdraw_address\":\"cosmos1erfdsa\""
        "}}]}";
    const auto tx = api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(currencies::ATOM, strTx);

    auto message = tx->getMessages().front();
    auto setWithdrawAddressMessage = api::CosmosLikeMessage::unwrapMsgSetWithdrawAddress(message);
    EXPECT_EQ(tx->getFee()->toLong(), 5001L);
    EXPECT_EQ(tx->getGas()->toLong(), 200020L);
    EXPECT_EQ(setWithdrawAddressMessage.delegatorAddress, "cosmos1dafe");
    EXPECT_EQ(setWithdrawAddressMessage.withdrawAddress, "cosmos1erfdsa");

    const std::string expected = "{\"mode\":\"async\",\"tx\":" + strTx + "}";
    EXPECT_EQ(tx->serializeForBroadcast(), expected);
}

TEST(CosmosTransactionTest, BuildWithdrawDelegatorRewardsTxForBroadcast) {
    // TODO : find a transaction in Explorer to confirm the format here
    const std::string strTx = "{"
        "\"fee\":{\"amount\":[{\"amount\":\"5001\",\"denom\":\"uatom\"}],\"gas\":\"200020\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msg\":[{"
            "\"type\":\"cosmos-sdk/MsgWithdrawDelegatorReward\","
            "\"value\":{"
                "\"delegator_address\":\"cosmos1targ\","
                "\"validator_address\":\"cosmosvaloperwdfae\""
        "}}]}";
    const auto tx = api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(currencies::ATOM, strTx);

    auto message = tx->getMessages().front();
    auto withdrawDorRewardMessage = api::CosmosLikeMessage::unwrapMsgWithdrawDelegatorReward(message);
    EXPECT_EQ(tx->getFee()->toLong(), 5001L);
    EXPECT_EQ(tx->getGas()->toLong(), 200020L);
    EXPECT_EQ(withdrawDorRewardMessage.delegatorAddress, "cosmos1targ");
    EXPECT_EQ(withdrawDorRewardMessage.validatorAddress, "cosmosvaloperwdfae");

    const std::string expected = "{\"mode\":\"async\",\"tx\":" + strTx + "}";
    EXPECT_EQ(tx->serializeForBroadcast(), expected);
}

TEST(CosmosTransactionTest, BuildWithdrawValidatorCommissionTxForBroadcast) {
    // TODO : find a transaction in Explorer to confirm the format here
    const std::string strTx = "{"
        "\"fee\":{\"amount\":[{\"amount\":\"5001\",\"denom\":\"uatom\"}],\"gas\":\"200020\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msg\":[{"
            "\"type\":\"cosmos-sdk/MsgWithdrawValidatorCommission\","
            "\"value\":{\"validator_address\":\"cosmosvaloper1234567890\"}}"
        "]}";
    const auto tx = api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(currencies::ATOM, strTx);

    auto message = tx->getMessages().front();
    auto withdrawVorCommissionMessage = api::CosmosLikeMessage::unwrapMsgWithdrawValidatorCommission(message);
    EXPECT_EQ(tx->getFee()->toLong(), 5001L);
    EXPECT_EQ(tx->getGas()->toLong(), 200020L);
    EXPECT_EQ(withdrawVorCommissionMessage.validatorAddress, "cosmosvaloper1234567890");

    const std::string expected = "{\"mode\":\"async\",\"tx\":" + strTx + "}";
    EXPECT_EQ(tx->serializeForBroadcast(), expected);
}

TEST(CosmosTransactionTest, BuildUnjailTxForBroadcast) {
    // TODO : find a transaction in Explorer to confirm the format here
    const std::string strTx = "{"
        "\"fee\":{\"amount\":[{\"amount\":\"5001\",\"denom\":\"uatom\"}],\"gas\":\"200020\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msg\":[{"
            "\"type\":\"cosmos-sdk/MsgUnjail\","
            "\"value\":{\"validator_address\":\"cosmosvaloper1dalton\"}}"
        "]}";
    const auto tx = api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(currencies::ATOM, strTx);

    auto message = tx->getMessages().front();
    auto unjailMessage = api::CosmosLikeMessage::unwrapMsgUnjail(message);
    EXPECT_EQ(tx->getFee()->toLong(), 5001L);
    EXPECT_EQ(tx->getGas()->toLong(), 200020L);
    EXPECT_EQ(unjailMessage.validatorAddress, "cosmosvaloper1dalton");

    const std::string expected = "{\"mode\":\"async\",\"tx\":" + strTx + "}";
    EXPECT_EQ(tx->serializeForBroadcast(), expected);
}

TEST(CosmosTransactionTest, BuildSendTxForSignature) {
    const std::string strTx = "{"
        "\"account_number\":\"6571\","
        "\"chain_id\":\"cosmoshub-3\","
        "\"fee\":{\"amount\":[{\"amount\":\"5000\",\"denom\":\"uatom\"}],\"gas\":\"200000\"},"
        "\"memo\":\"Sent from Ledger\","
        "\"msgs\":[{"
            "\"type\":\"cosmos-sdk/MsgSend\","
            "\"value\":{"
                "\"amount\":[{\"amount\":\"1000000\",\"denom\":\"uatom\"}],"
                "\"from_address\":\"cosmos1d9h8qat57ljhcm\","
                "\"to_address\":\"cosmos1da6hgur4wsmpnjyg\""
        "}}],"
        "\"sequence\":\"0\""
        "}";

    const auto tx = api::CosmosLikeTransactionBuilder::parseRawUnsignedTransaction(currencies::ATOM, strTx);

    EXPECT_EQ(tx->serializeForSignature(), strTx);
}
