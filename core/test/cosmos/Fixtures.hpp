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

                        Message setupDelegateMessage(const std::chrono::system_clock::time_point& timeRef);
                        Message setupDepositMessage(const std::chrono::system_clock::time_point& timeRef);
                        Message setupRedelegateMessage(const std::chrono::system_clock::time_point& timeRef);
                        Message setupSendMessage(const std::chrono::system_clock::time_point& timeRef);
                        Message setupSubmitProposalMessage(const std::chrono::system_clock::time_point& timeRef);
                        Message setupUndelegateMessage(const std::chrono::system_clock::time_point& timeRef);
                        Message setupVoteMessage(const std::chrono::system_clock::time_point& timeRef);
                        Message setupWithdrawDelegationRewardMessage(const std::chrono::system_clock::time_point& timeRef);

                        Transaction setupTransaction(const std::vector<Message>& msgs, const std::chrono::system_clock::time_point& timeRef);

                        void assertSameDelegateMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameDepositMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameRedelegateMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameSendMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameSubmitProposalMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameUndelegateMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameVoteMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameWithdrawDelegationRewardMessage(const Message& msgRef, const Message& msgResult);
                        void assertSameTransaction(const Transaction& txRef, const Transaction& txResult);
                }
        }
}

#endif // __FIXTURES_H_
