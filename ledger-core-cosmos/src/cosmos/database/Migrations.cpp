#include <cosmos/database/Migrations.hpp>

namespace ledger {
namespace core {
int constexpr CosmosMigration::COIN_ID;
uint32_t constexpr CosmosMigration::CURRENT_VERSION;

template <>
void migrate<1, CosmosMigration>(soci::session &sql,
                                 api::DatabaseBackendType type) {
  sql << "CREATE TABLE cosmos_currencies("
         "name VARCHAR(255) PRIMARY KEY NOT NULL REFERENCES currencies(name) "
         "ON DELETE CASCADE ON UPDATE CASCADE,"
         "identifier VARCHAR(255) NOT NULL,"
         "xpub_version VARCHAR(255) NOT NULL,"
         "pubkey_prefix VARCHAR(255) NOT NULL,"
         "address_prefix VARCHAR(255) NOT NULL,"
         "message_prefix VARCHAR(255) NOT NULL,"
         "chain_id VARCHAR(255) NOT NULL,"
         "additional_CIPs TEXT"
         ")";

  sql << "CREATE TABLE cosmos_accounts("
         "uid VARCHAR(255) NOT NULL PRIMARY KEY REFERENCES accounts(uid) ON "
         "DELETE CASCADE ON UPDATE CASCADE,"
         "wallet_uid VARCHAR(255) NOT NULL REFERENCES wallets(uid) ON DELETE "
         "CASCADE ON UPDATE CASCADE,"
         "idx INTEGER NOT NULL,"
         "pubkey VARCHAR(255) NOT NULL,"
         "account_type VARCHAR(255),"
         "account_number VARCHAR(255),"
         "sequence VARCHAR(255),"
         "balances VARCHAR(255),"
         "withdraw_address VARCHAR(255),"
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

  // * NOTE : Special Msg types
  // ** MsgMultiSend
  // MsgMultiSend can have an arbitrary number of inputs and outputs
  // (only invariant is sum(inputs) == sum(outputs))
  // Therefore the various inputs and outputs are stored in another
  // table to avoid varchars longer than 256 bytes
  //
  // ** MsgCreateValidator
  // This message is not handled in database, and behaviour with these
  // messages is undefined.
  //
  // ** MsgEditValidator
  // This message is not handled in database, and behaviour with these
  // messages is undefined.
  sql << "CREATE TABLE cosmos_messages("
         "uid VARCHAR(255) PRIMARY KEY NOT NULL,"
         "transaction_uid VARCHAR(255) NOT NULL "
         "REFERENCES cosmos_transactions(uid) ON DELETE CASCADE ON UPDATE "
         "CASCADE,"
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
         // MsgBeginRedelegate
         "validator_src_address VARCHAR(255),"
         "validator_dst_address VARCHAR(255),"
         // MsgSubmitProposal
         "content_type TEXT,"
         "content_title TEXT,"
         "content_description TEXT,"
         "proposer VARCHAR(255),"
         // MsgVote
         "voter VARCHAR(255),"
         "proposal_id VARCHAR(255),"
         "vote_option VARCHAR(255),"
         // MsgDeposit
         "depositor VARCHAR(255)"
         ")";

  sql << "CREATE TABLE cosmos_multisend_io("
         "message_uid VARCHAR(255) NOT NULL "
         "REFERENCES cosmos_messages(uid) ON DELETE CASCADE ON UPDATE CASCADE,"
         // not null when input
         "from_address VARCHAR(255),"
         // not null when output
         "to_address VARCHAR(255),"
         "amount VARCHAR(255) NOT NULL"
         ")";

  sql << "CREATE TABLE cosmos_operations("
         "uid VARCHAR(255) PRIMARY KEY NOT NULL "
         "REFERENCES operations(uid) ON DELETE CASCADE,"
         "message_uid VARCHAR(255) NOT NULL "
         "REFERENCES cosmos_messages(uid) ON DELETE CASCADE ON UPDATE CASCADE"
         ")";
}

template <>
void rollback<1, CosmosMigration>(soci::session &sql,
                                  api::DatabaseBackendType type) {
  sql << "DROP TABLE cosmos_multisend_io";
  sql << "DROP TABLE cosmos_operations";
  sql << "DROP TABLE cosmos_messages";
  sql << "DROP TABLE cosmos_transactions";
  sql << "DROP TABLE cosmos_accounts";
  sql << "DROP TABLE cosmos_currencies";
}
} // namespace core
} // namespace ledger
