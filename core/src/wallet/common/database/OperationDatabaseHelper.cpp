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
#include <crypto/SHA256.hpp>
#include <wallet/bitcoin/database/BitcoinLikeTransactionDatabaseHelper.h>
#include <database/soci-number.h>
#include <database/soci-date.h>
#include <database/soci-option.h>
#include <bytes/serialization.hpp>
#include <collections/strings.hpp>
#include <wallet/bitcoin/keychains/P2PKHBitcoinLikeKeychain.hpp>

using namespace soci;

namespace ledger {
    namespace core {

        std::string OperationDatabaseHelper::createUid(const std::string &accountUid, const std::string &txId,
                                                       const api::OperationType type) {
            return SHA256::stringToHexHash(fmt::format("uid:{}+{}+{}", accountUid, txId, api::to_string(type)));
        }

        void OperationDatabaseHelper::putOperation(soci::session &sql, const Operation &operation) {
            auto count = 0;
            std::stringstream trust;
            serialization::saveXML<TrustIndicator>(*operation.trust, trust);
            sql << "SELECT COUNT(*) FROM operations WHERE uid = :uid", use(operation.uid), into(count);
            auto newOperation = count == 0;
            if (!newOperation) {
                sql << "UPDATE operations SET block_height = :height, trust = :trust WHERE uid = :uid"
                        , use(operation.blockHeight)
                        , use(trust.str())
                        , use(operation.uid);
            } else {
                auto type = api::to_string(operation.type);
                std::stringstream senders;
                std::stringstream recipients;
                std::string separator(",");
                strings::join(operation.senders, senders, separator);
                strings::join(operation.recipients, recipients, separator);
                fmt::print("{}\n", senders.str());
                auto sndrs = senders.str();
                auto rcvrs = recipients.str();
                sql << "INSERT INTO operations VALUES("
                            ":uid, :accout_uid, :wallet_uid, :type, :date, :senders, :recipients, :amount,"
                            ":fees, :block_height, :currency_name, :trust"
                        ")"
                        , use(operation.uid), use(operation.accountUid), use(operation.walletUid), use(type), use(operation.date)
                        , use(sndrs), use(rcvrs), use(operation.amount.toInt64())
                        , use(operation.fees), use(operation.blockHeight), use(operation.currencyName), use(trust.str());


            }
            updateBitcoinOperation(sql, operation, newOperation);
        }

        void
        OperationDatabaseHelper::updateBitcoinOperation(soci::session &sql, const Operation &operation, bool insert) {
            if (operation.bitcoinTransaction.nonEmpty()) {
                BitcoinLikeTransactionDatabaseHelper::putTransaction(sql, operation.bitcoinTransaction
                                                                                   .getValue());
                if (insert)
                    sql << "INSERT INTO bitcoin_operations VALUES(:uid, :tx_hash)", use(operation.uid)
                            , use(operation.bitcoinTransaction.getValue().hash);
            }
        }

    }
}