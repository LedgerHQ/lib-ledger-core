/*
 *
 * OperationDatabaseHelper
 * ledger-core
 *
 * Created by Pierre Pollastri on 31/05/2017.
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
#include "OperationDatabaseHelper.h"
#include "BlockDatabaseHelper.h"
#include <crypto/SHA256.hpp>
#include <api/Amount.hpp>
#include <api/BigInt.hpp>
#include <wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h>
#include <database/soci-number.h>
#include <database/soci-date.h>
#include <database/soci-option.h>
#include <wallet/ethereum/database/EthereumLikeTransactionDatabaseHelper.h>
#include <wallet/ripple/database/RippleLikeTransactionDatabaseHelper.h>
#include <wallet/tezos/database/TezosLikeTransactionDatabaseHelper.h>
#include <bytes/serialization.hpp>
#include <collections/strings.hpp>
#include <wallet/common/TrustIndicator.h>

using namespace soci;

namespace ledger {
    namespace core {

        std::string OperationDatabaseHelper::createUid(const std::string &accountUid, const std::string &txId,
                                                       const api::OperationType type) {
            return SHA256::stringToHexHash(fmt::format("uid:{}+{}+{}", accountUid, txId, api::to_string(type)));
        }

        bool OperationDatabaseHelper::putOperation(soci::session &sql, const Operation &operation) {
            auto count = 0;
            std::string serializedTrust;
            serialization::saveBase64<TrustIndicator>(*operation.trust, serializedTrust);
            if (operation.block.nonEmpty()) {
                BlockDatabaseHelper::putBlock(sql, operation.block.getValue());
            }
            auto blockUid = operation.block.map<std::string>([] (const Block& block) {
                return block.getUid();
            });
            sql << "SELECT COUNT(*) FROM operations WHERE uid = :uid", use(operation.uid), into(count);
            auto newOperation = count == 0;
            if (!newOperation) {
                sql << "UPDATE operations SET block_uid = :block_uid, trust = :trust WHERE uid = :uid"
                        , use(blockUid)
                        , use(serializedTrust)
                        , use(operation.uid);
                updateCurrencyOperation(sql, operation, newOperation);
                return false;
            } else {
                auto type = api::to_string(operation.type);
                std::stringstream senders;
                std::stringstream recipients;
                std::string separator(",");
                strings::join(operation.senders, senders, separator);
                strings::join(operation.recipients, recipients, separator);
                auto sndrs = senders.str();
                auto rcvrs = recipients.str();
                auto hexAmount = operation.amount.toHexString();
                auto hexFees = operation.fees.getValueOr(BigInt::ZERO).toHexString();
                sql << "INSERT INTO operations VALUES("
                            ":uid, :accout_uid, :wallet_uid, :type, :date, :senders, :recipients, :amount,"
                            ":fees, :block_uid, :currency_name, :trust"
                        ")"
                        , use(operation.uid), use(operation.accountUid), use(operation.walletUid), use(type), use(operation.date)
                        , use(sndrs), use(rcvrs), use(hexAmount)
                        , use(hexFees), use(blockUid)
                        , use(operation.currencyName), use(serializedTrust);

                updateCurrencyOperation(sql, operation, newOperation);
                return true;
            }

        }


        void
        OperationDatabaseHelper::updateCurrencyOperation(soci::session &sql, const Operation &operation, bool insert) {
            if (operation.bitcoinTransaction.nonEmpty()) {
                auto operationValue = operation.bitcoinTransaction.getValue();
                auto btcTxUid = BitcoinLikeTransactionDatabaseHelper::putTransaction(sql, operation.accountUid, operationValue);
                if (insert)
                    sql << "INSERT INTO bitcoin_operations VALUES(:uid, :tx_uid, :tx_hash)", use(operation.uid), use(btcTxUid), use(operationValue.hash);
            } else if (operation.ethereumTransaction.nonEmpty()) {
                auto operationValue = operation.ethereumTransaction.getValue();
                auto ethTxUid = EthereumLikeTransactionDatabaseHelper::putTransaction(sql, operation.accountUid, operationValue);
                if (insert) {
                    sql << "INSERT INTO ethereum_operations VALUES(:uid, :tx_uid, :tx_hash)", use(operation.uid), use(ethTxUid), use(operationValue.hash);
                }
            } else if (operation.rippleTransaction.nonEmpty()) {
                auto operationValue = operation.rippleTransaction.getValue();
                auto rippleTxUid = RippleLikeTransactionDatabaseHelper::putTransaction(sql, operation.accountUid, operationValue);
                if (insert) {
                    sql << "INSERT INTO ripple_operations VALUES(:uid, :tx_uid, :tx_hash)", use(operation.uid), use(rippleTxUid), use(operationValue.hash);
                }
            } else if (operation.tezosTransaction.nonEmpty()) {
                auto operationValue = operation.tezosTransaction.getValue();
                auto tezosTxUid = TezosLikeTransactionDatabaseHelper::putTransaction(sql, operation.accountUid, operationValue);
                if (insert) {
                    sql << "INSERT INTO tezos_operations VALUES(:uid, :tx_uid, :tx_hash)", use(operation.uid), use(tezosTxUid), use(operationValue.hash);
                }
            }
        }

        void OperationDatabaseHelper::queryOperations(soci::session &sql, int32_t from, int32_t to, bool complete,
                                                      bool excludeDropped, std::vector<Operation> &out) {

        }

        std::size_t
        OperationDatabaseHelper::queryOperations(soci::session &sql,
                                                 const std::string &accountUid,
                                                 std::vector<Operation> &operations,
                                                 std::function<bool(const std::string &address)> filter) {
            rowset<row> rows = (sql.prepare <<
                                            "SELECT op.amount, op.fees, op.type, op.date, op.senders, op.recipients, op.uid"
                                                    " FROM operations AS op "
                                                    " WHERE op.account_uid = :uid ORDER BY op.date",
                                                    use(accountUid));

            auto filterList = [&] (const std::vector<std::string> &list) -> bool {
                for (auto& elem : list) {
                    if (filter(elem)) {
                        return true;
                    }
                }
                return false;
            };

            std::size_t c = 0;
            for (auto& row : rows) {
                auto type = api::from_string<api::OperationType>(row.get<std::string>(2));
                auto senders = strings::split(row.get<std::string>(4), ",");
                auto recipients = strings::split(row.get<std::string>(5), ",");
                if ((type == api::OperationType::SEND && row.get_indicator(4) != i_null && filterList(senders)) ||
                    (type == api::OperationType::RECEIVE && row.get_indicator(5) != i_null && filterList(recipients)) ||
                    type == api::OperationType::NONE) {
                    operations.resize(operations.size() + 1);
                    auto& operation = operations[operations.size() - 1];
                    operation.amount = BigInt::fromHex(row.get<std::string>(0));
                    operation.fees = BigInt::fromHex(row.get<std::string>(1));
                    operation.type = type;
                    operation.date = DateUtils::fromJSON(row.get<std::string>(3));
                    operation.uid = row.get<std::string>(6);
                    c += 1;
                }
            }
            return c;
        }

    }
}
