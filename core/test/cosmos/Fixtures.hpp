#ifndef __FIXTURES_H_
#define __FIXTURES_H_

#include <test/integration/BaseFixture.h>

#include <wallet/cosmos/CosmosLikeAccount.hpp>
#include <wallet/cosmos/cosmos.hpp>


using namespace ledger::core::cosmos;

namespace ledger {
        namespace testing {
                namespace cosmos {

                        const std::string DEFAULT_ADDRESS = "cosmos1sd4tl9aljmmezzudugs7zlaya7pg2895tyn79r";
                        const std::string DEFAULT_HEX_PUB_KEY = "03d672c1b90c84d9d97522e9a73252a432b77d90a78bf81cdbe35270d9d3dc1c34";

                        std::shared_ptr<core::CosmosLikeAccount> createCosmosLikeAccount(
                                const std::shared_ptr<core::AbstractWallet>& wallet,
                                int32_t index,
                                const core::api::AccountCreationInfo &info);
                        std::shared_ptr<core::CosmosLikeAccount> createCosmosLikeAccount(
                                const std::shared_ptr<core::AbstractWallet>& wallet,
                                int32_t index,
                                const core::api::ExtendedKeyAccountCreationInfo &info);

                        Message setupDelegateMessage();
                        Message setupDepositMessage();
                        Message setupRedelegateMessage();
                        Message setupSendMessage();
                        Message setupSubmitProposalMessage();
                        Message setupUndelegateMessage();
                        Message setupVoteMessage();
                        Message setupWithdrawDelegationRewardMessage();
                        Message setupFeesMessage();

                        // Build a transaction to be broadcasted to the blockhain
                        Transaction setupTransactionRequest(const std::vector<Message>& msgs);

                        // Build a transaction as it is received from the blockhain
                        Transaction setupTransactionResponse(const std::vector<Message>& msgs, const std::chrono::system_clock::time_point& timeRef);

                        void assertSameDelegateMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameDepositMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameRedelegateMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameSendMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameSubmitProposalMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameUndelegateMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameVoteMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameWithdrawDelegationRewardMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameFeesMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameTransaction(const Transaction& txRef, const Transaction& txResult);
                }
        }
}

#endif // __FIXTURES_H_
