#include <cosmos/database/Migrations.hpp>

namespace ledger {
    namespace core {
        int constexpr CosmosMigration::COIN_ID;
        uint32_t constexpr CosmosMigration::CURRENT_VERSION;

        template <> void migrate<1, CosmosMigration>(soci::session& sql) {
            sql << "CREATE TABLE cosmos_currencies("
                "name VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES currencies(name) ON DELETE CASCADE ON UPDATE CASCADE,"
                "identifier VARCHAR(255) NOT NULL,"
                "xpub_version VARCHAR(255) NOT NULL,"
                "pubkey_prefix VARCHAR(255) NOT NULL,"
                "address_prefix VARCHAR(255) NOT NULL,"
                "message_prefix VARCHAR(255) NOT NULL,"
                "chain_id VARCHAR(255) NOT NULL,"
                "additional_CIPs TEXT"
                ")";

            sql << "CREATE TABLE cosmos_accounts("
                "uid VARCHAR(255) NOT NULL PRIMARY KEY REFERENCES accounts(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                "wallet_uid VARCHAR(255) NOT NULL REFERENCES wallets(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                "idx INTEGER NOT NULL,"
                "address VARCHAR(255) NOT NULL,"
                "account_type VARCHAR(255),"
                "account_number VARCHAR(255),"
                "sequence VARCHAR(255),"
                "balances VARCHAR(255),"
                "last_update TEXT"
                ")";

            sql << "CREATE TABLE cosmos_transactions("
                "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                "hash VARCHAR(255) NOT NULL,"
                "block_uid VARCHAR(255) REFERENCES blocks(uid) ON DELETE CASCADE,"
                "time VARCHAR(255) NOT NULL,"
                "fee_amount VARCHAR(255) NOT NULL,"
                "gas VARCHAR(255) NOT NULL,"
                "gas_used VARCHAR(255),"
                "memo TEXT"
                ")";

            // * TODO : handle missing Msg types
            // ** MsgMultiSend
            // // MsgMultiSend - high level transaction of the coin module
            // type MsgMultiSend struct {
            //  Inputs  []Input  `json:"inputs"`
            //  Outputs []Output `json:"outputs"`
            // }
            // // Input models transaction input
            // type Input struct {
            //  Address sdk.AccAddress `json:"address"`
            //  Coins   sdk.Coins      `json:"coins"`
            // }
            // // Output models transaction outputs
            // type Output struct {
            //  Address sdk.AccAddress `json:"address"`
            //  Coins   sdk.Coins      `json:"coins"`
            // }
            //
            // ** MsgCreateValidator
            // // MsgCreateValidator - struct for bonding transactions
            // type MsgCreateValidator struct {
            //  Description       Description    `json:"description"`
            //  Commission        CommissionMsg  `json:"commission"`
            //  MinSelfDelegation sdk.Int        `json:"min_self_delegation"`
            //  DelegatorAddress  sdk.AccAddress `json:"delegator_address"`
            //  ValidatorAddress  sdk.ValAddress `json:"validator_address"`
            //  PubKey            crypto.PubKey  `json:"pubkey"`
            //  Value             sdk.Coin       `json:"value"`
            // }
            //
            // ** MsgEditValidator
            // // MsgEditValidator - struct for editing a validator
            // type MsgEditValidator struct {
            //  Description
            //  ValidatorAddress sdk.ValAddress `json:"address"`
            //
            //  // We pass a reference to the new commission rate and min self delegation as it's not mandatory to
            //  // update. If not updated, the deserialized rate will be zero with no way to
            //  // distinguish if an update was intended.
            //  //
            //  // REF: #2373
            //  CommissionRate    *sdk.Dec `json:"commission_rate"`
            //  MinSelfDelegation *sdk.Int `json:"min_self_delegation"`
            // }
            sql << "CREATE TABLE cosmos_messages("
                "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
                "transaction_uid VARCHAR(255) NOT NULL "
                    "REFERENCES cosmos_transactions(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
                "message_type VARCHAR(255) NOT NULL,"
                "log TEXT,"
                "success INTEGER,"
                "msg_index INTEGER NOT NULL,"
                // MsgSend
                "from_address VARCHAR(255),"
                "to_address VARCHAR(255),"
                "amount VARCHAR(255),"
                // MsgDelegate & MsgUndelegate
                "delegator_address VARCHAR(255),"
                "validator_address VARCHAR(255),"
                // MsgRedelegate
                "validator_src_address VARCHAR(255),"
                "validator_dst_address VARCHAR(255),"
                // MsgSubmitProposal
                "content_type TEXT,"
                "content_title TEXT,"
                "content_description TEXT,"
                "proposer VARCHAR(255),"
                // MsgVote
                "voter VARCHAR(40),"
                "proposal_id VARCHAR(255),"
                "vote_option VARCHAR(255),"
                // MsgDeposit
                "depositor VARCHAR(255)"
                ")";

            sql << "CREATE TABLE cosmos_operations("
                "uid VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES operations(uid) ON DELETE CASCADE,"
                "message_uid VARCHAR(255) NOT NULL REFERENCES cosmos_messages(uid)"
                ")";
        }

        template <> void rollback<1, CosmosMigration>(soci::session& sql) {
            sql << "DROP TABLE cosmos_transactions";

            sql << "DROP TABLE cosmos_operations";

            sql << "DROP TABLE cosmos_accounts";

            sql << "DROP TABLE cosmos_currencies";

            sql << "DROP TABLE cosmos_messages";
        }
    }
}
