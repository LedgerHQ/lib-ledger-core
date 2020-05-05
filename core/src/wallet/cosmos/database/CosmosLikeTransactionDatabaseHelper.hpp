/*
 *
 * CosmosLikeTransactionDatabaseHelper
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
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


#ifndef LEDGER_CORE_COSMOSLIKETRANSACTIONDATABASEHELPER_H
#define LEDGER_CORE_COSMOSLIKETRANSACTIONDATABASEHELPER_H

#include <wallet/cosmos/cosmos.hpp>

#include <soci.h>
#include <string>


namespace ledger {
    namespace core {

        class CosmosLikeTransactionDatabaseHelper {

        public:
            static bool transactionExists(soci::session &sql, const std::string &cosmosTxUid);

            /// \brief Tx --> DB.
            /// tx is modified with an updated uid (so it is an [in,out] param)
            static void putTransaction(soci::session &sql,
                                       const std::string &accountUid,
                                       cosmos::Transaction &tx);

            /// \brief DB --> Tx.
            static bool getTransactionByHash(soci::session &sql,
                                             const std::string &hash,
                                             cosmos::Transaction &tx);

            /// \brief DB --> Msg.
            static bool getMessageByUid(soci::session &sql,
                                        const std::string &msgUid,
                                        cosmos::Message &msg);

            /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table.
            /// This function fills the common data, and then dispatch the message type specifics to
            /// another helper function.
            /// \param [in] sql The sql session to connect to the database (used when additional joins are necessary)
            /// \param [in] row The row from cosmos_messages table to use to fill the message data
            /// \param [out] msg The cosmos::Message to fill.
            static void inflateMessage(soci::session &sql, const soci::row& row, cosmos::Message& msg);

            /// \brief Insert information from a message in the database (cosmos_messages).
            /// \param [in] sql The sql session to connect to the database
            /// \param [in] txUid Database unique identifier of the transaction holding the message
            /// \param [in] msg The message to insert in the database
            /// \param [in] log The logging information about this message execution on the network.
            static void insertMessage(soci::session &sql, const std::string& txUid, const cosmos::Message& msg, const cosmos::MessageLog& log);

            private:
                /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgSend specific data.
                /// \param [in] row The row from cosmos_messages table to use
                /// \param [out] msg The cosmos::Message to fill
                static void inflateMsgSendSpecifics(const soci::row& row, cosmos::Message& msg);
                /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgDelegate specific data.
                /// \param [in] row The row from cosmos_messages table to use
                /// \param [out] msg The cosmos::Message to fill
                static void inflateMsgDelegateSpecifics(const soci::row& row, cosmos::Message& msg);
                /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgUndelegate specific data.
                /// \param [in] row The row from cosmos_messages table to use
                /// \param [out] msg The cosmos::Message to fill
                static void inflateMsgUndelegateSpecifics(const soci::row& row, cosmos::Message& msg);
                /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgBeginRedelegate specific data.
                /// \param [in] row The row from cosmos_messages table to use
                /// \param [out] msg The cosmos::Message to fill
                static void inflateMsgBeginRedelegateSpecifics(const soci::row& row, cosmos::Message& msg);
                /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgSubmitProposal specific data.
                /// \param [in] row The row from cosmos_messages table to use
                /// \param [out] msg The cosmos::Message to fill
                static void inflateMsgSubmitProposalSpecifics(const soci::row& row, cosmos::Message& msg);
                /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgVote specific data.
                /// \param [in] row The row from cosmos_messages table to use
                /// \param [out] msg The cosmos::Message to fill
                static void inflateMsgVoteSpecifics(const soci::row& row, cosmos::Message& msg);
                /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgDeposit specific data.
                /// \param [in] row The row from cosmos_messages table to use
                /// \param [out] msg The cosmos::Message to fill
                static void inflateMsgDepositSpecifics(const soci::row& row, cosmos::Message& msg);
                /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgWithdrawDelegationReward specific data.
                /// \param [in] row The row from cosmos_messages table to use
                /// \param [out] msg The cosmos::Message to fill
                static void inflateMsgWithdrawDelegationRewardSpecifics(const soci::row& row, cosmos::Message& msg);
                /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgWithdrawDelegatorReward specific data.
                /// \param [in] row The row from cosmos_messages table to use
                /// \param [out] msg The cosmos::Message to fill
                static void inflateMsgWithdrawDelegatorRewardSpecifics(const soci::row& row, cosmos::Message& msg);
                /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgWithdrawValidatorCommission specific data.
                /// \param [in] row The row from cosmos_messages table to use
                /// \param [out] msg The cosmos::Message to fill
                static void inflateMsgWithdrawValidatorCommissionSpecifics(const soci::row& row, cosmos::Message& msg);
                /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgSetWithdrawAddress specific data.
                /// \param [in] row The row from cosmos_messages table to use
                /// \param [out] msg The cosmos::Message to fill
                static void inflateMsgSetWithdrawAddressSpecifics(const soci::row& row, cosmos::Message& msg);
                /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgUnjail specific data.
                /// \param [in] row The row from cosmos_messages table to use
                /// \param [out] msg The cosmos::Message to fill
                static void inflateMsgUnjailSpecifics(const soci::row& row, cosmos::Message& msg);
                /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgMultiSend specific data.
                /// \param [in] sql The sql session to connect to the database (used to join cosmos_multisend_io table)
                /// \param [in] row The row from cosmos_messages table to use
                /// \param [out] msg The cosmos::Message to fill
                static void inflateMsgMultiSendSpecifics(soci::session &sql, const soci::row& row, cosmos::Message& msg);
                /// \brief Fill a cosmos::Message from a database entry in cosmos_messages table with MsgFees specific data.
                /// \param [in] row The row from cosmos_messages table to use
                /// \param [out] msg The cosmos::Message to fill
                static void inflateMsgFeesSpecifics(const soci::row& row, cosmos::Message& msg);

                /// \brief insert a *wrapped* cosmos::MsgSend into the database table cosmos_messages table with MsgSend specific data.
                /// \param [in] sql The sql session to connect to the database
                /// \param [in] txUid Database identifier of the transaction for joins
                /// \param [in] msg The cosmos::Message to add (with cosmos::MsgSend data)
                /// \param [in] log The cosmos::MessageLog data to append to the message in the database
                static void insertMsgSend(soci::session& sql, const std::string &txUid, const cosmos::Message& msg, const cosmos::MessageLog& log);

                /// \brief insert a *wrapped* cosmos::MsgDelegate into the database table cosmos_messages table with MsgDelegate specific data.
                /// \param [in] sql The sql session to connect to the database
                /// \param [in] txUid Database identifier of the transaction for joins
                /// \param [in] msg The cosmos::Message to add (with cosmos::MsgDelegate data)
                /// \param [in] log The cosmos::MessageLog data to append to the message in the database
                static void insertMsgDelegate(soci::session& sql, const std::string &txUid, const cosmos::Message& msg, const cosmos::MessageLog& log);

                /// \brief insert a *wrapped* cosmos::MsgUndelegate into the database table cosmos_messages table with MsgUndelegate specific data.
                /// \param [in] sql The sql session to connect to the database
                /// \param [in] txUid Database identifier of the transaction for joins
                /// \param [in] msg The cosmos::Message to add (with cosmos::MsgUndelegate data)
                /// \param [in] log The cosmos::MessageLog data to append to the message in the database
                static void insertMsgUndelegate(soci::session& sql, const std::string &txUid, const cosmos::Message& msg, const cosmos::MessageLog& log);

                /// \brief insert a *wrapped* cosmos::MsgBeginRedelegate into the database table cosmos_messages table with MsgBeginRedelegate specific data.
                /// \param [in] sql The sql session to connect to the database
                /// \param [in] txUid Database identifier of the transaction for joins
                /// \param [in] msg The cosmos::Message to add (with cosmos::MsgBeginRedelegate data)
                /// \param [in] log The cosmos::MessageLog data to append to the message in the database
                static void insertMsgBeginRedelegate(soci::session& sql, const std::string &txUid, const cosmos::Message& msg, const cosmos::MessageLog& log);

                /// \brief insert a *wrapped* cosmos::MsgSubmitProposal into the database table cosmos_messages table with MsgSubmitProposal specific data.
                /// \param [in] sql The sql session to connect to the database
                /// \param [in] txUid Database identifier of the transaction for joins
                /// \param [in] msg The cosmos::Message to add (with cosmos::MsgSubmitProposal data)
                /// \param [in] log The cosmos::MessageLog data to append to the message in the database
                static void insertMsgSubmitProposal(soci::session& sql, const std::string &txUid, const cosmos::Message& msg, const cosmos::MessageLog& log);

                /// \brief insert a *wrapped* cosmos::MsgVote into the database table cosmos_messages table with MsgVote specific data.
                /// \param [in] sql The sql session to connect to the database
                /// \param [in] txUid Database identifier of the transaction for joins
                /// \param [in] msg The cosmos::Message to add (with cosmos::MsgVote data)
                /// \param [in] log The cosmos::MessageLog data to append to the message in the database
                static void insertMsgVote(soci::session& sql, const std::string &txUid, const cosmos::Message& msg, const cosmos::MessageLog& log);

                /// \brief insert a *wrapped* cosmos::MsgDeposit into the database table cosmos_messages table with MsgDeposit specific data.
                /// \param [in] sql The sql session to connect to the database
                /// \param [in] txUid Database identifier of the transaction for joins
                /// \param [in] msg The cosmos::Message to add (with cosmos::MsgDeposit data)
                /// \param [in] log The cosmos::MessageLog data to append to the message in the database
                static void insertMsgDeposit(soci::session& sql, const std::string &txUid, const cosmos::Message& msg, const cosmos::MessageLog& log);

                /// \brief insert a *wrapped* cosmos::MsgWithdrawDelegationReward into the database table cosmos_messages table with MsgWithdrawDelegationReward specific data.
                /// \param [in] sql The sql session to connect to the database
                /// \param [in] txUid Database identifier of the transaction for joins
                /// \param [in] msg The cosmos::Message to add (with cosmos::MsgWithdrawDelegationReward data)
                /// \param [in] log The cosmos::MessageLog data to append to the message in the database
                static void insertMsgWithdrawDelegationReward(soci::session& sql, const std::string &txUid, const cosmos::Message& msg, const cosmos::MessageLog& log);

                /// \brief insert a *wrapped* cosmos::MsgWithdrawDelegatorReward into the database table cosmos_messages table with MsgWithdrawDelegatorReward specific data.
                /// \param [in] sql The sql session to connect to the database
                /// \param [in] txUid Database identifier of the transaction for joins
                /// \param [in] msg The cosmos::Message to add (with cosmos::MsgWithdrawDelegatorReward data)
                /// \param [in] log The cosmos::MessageLog data to append to the message in the database
                static void insertMsgWithdrawDelegatorReward(soci::session& sql, const std::string &txUid, const cosmos::Message& msg, const cosmos::MessageLog& log);

                /// \brief insert a *wrapped* cosmos::MsgWithdrawValidatorCommission into the database table cosmos_messages table with MsgWithdrawValidatorCommission specific data.
                /// \param [in] sql The sql session to connect to the database
                /// \param [in] txUid Database identifier of the transaction for joins
                /// \param [in] msg The cosmos::Message to add (with cosmos::MsgWithdrawValidatorCommission data)
                /// \param [in] log The cosmos::MessageLog data to append to the message in the database
                static void insertMsgWithdrawValidatorCommission(soci::session& sql, const std::string &txUid, const cosmos::Message& msg, const cosmos::MessageLog& log);

                /// \brief insert a *wrapped* cosmos::MsgSetWithdrawAddress into the database table cosmos_messages table with MsgSetWithdrawAddress specific data.
                /// \param [in] sql The sql session to connect to the database
                /// \param [in] txUid Database identifier of the transaction for joins
                /// \param [in] msg The cosmos::Message to add (with cosmos::MsgSetWithdrawAddress data)
                /// \param [in] log The cosmos::MessageLog data to append to the message in the database
                static void insertMsgSetWithdrawAddress(soci::session& sql, const std::string &txUid, const cosmos::Message& msg, const cosmos::MessageLog& log);

                /// \brief insert a *wrapped* cosmos::MsgUnjail into the database table cosmos_messages table with MsgUnjail specific data.
                /// \param [in] sql The sql session to connect to the database
                /// \param [in] txUid Database identifier of the transaction for joins
                /// \param [in] msg The cosmos::Message to add (with cosmos::MsgUnjail data)
                /// \param [in] log The cosmos::MessageLog data to append to the message in the database
                static void insertMsgUnjail(soci::session& sql, const std::string &txUid, const cosmos::Message& msg, const cosmos::MessageLog& log);

                /// \brief insert a *wrapped* cosmos::MsgMultiSend into the database table cosmos_messages table with MsgMultiSend specific data.
                /// \param [in] sql The sql session to connect to the database
                /// \param [in] txUid Database identifier of the transaction for joins
                /// \param [in] msg The cosmos::Message to add (with cosmos::MsgMultiSend data)
                /// \param [in] log The cosmos::MessageLog data to append to the message in the database
                static void insertMsgMultiSend(soci::session& sql, const std::string &txUid, const cosmos::Message& msg, const cosmos::MessageLog& log);

                /// \brief insert a *wrapped* cosmos::MsgFees into the database table cosmos_messages table with MsgFees specific data.
                /// \param [in] sql The sql session to connect to the database
                /// \param [in] txUid Database identifier of the transaction for joins
                /// \param [in] msg The cosmos::Message to add (with cosmos::MsgFees data)
                /// \param [in] log The cosmos::MessageLog data to append to the message in the database
                static void insertMsgFees(soci::session& sql, const std::string &txUid, const cosmos::Message& msg, const cosmos::MessageLog& log);

                /// \brief insert a *wrapped* cosmos::MsgGeneric into the database table cosmos_messages table without any specific data.
                /// Only the type of the message is stored, for debugging purposes
                /// \param [in] sql The sql session to connect to the database
                /// \param [in] txUid Database identifier of the transaction for joins
                /// \param [in] msg The cosmos::Message to add
                /// \param [in] log The cosmos::MessageLog data to append to the message in the database
                static void insertMsgGeneric(soci::session& sql, const std::string &txUid, const cosmos::Message& msg, const cosmos::MessageLog& log);
        };
    }
}

#endif //LEDGER_CORE_COSMOSLIKETRANSACTIONDATABASEHELPER_H
