/*
 *
 * BulkInsertDatabaseHelper.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 05/01/2021.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2021 Ledger
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

#include "BulkInsertDatabaseHelper.hpp"
#include <bytes/serialization.hpp>
#include <wallet/common/TrustIndicator.h>
#include <wallet/common/database/BlockDatabaseHelper.h>
#include <database/soci-date.h>
#include <database/soci-option.h>

namespace ledger {
    namespace core {
        using namespace soci;

        const StatementDeclaration<OperationBinding> BulkInsertDatabaseHelper::UPSERT_OPERATION =
                db::stmt<OperationBinding>(
                        "INSERT INTO operations VALUES("
                        ":uid, :account_uid, :wallet_uid, :type, :date, :senders, :recipients, :amount,"
                        ":fees, :block_uid, :currency_name, :trust"
                        ") ON CONFLICT(uid) DO UPDATE SET block_uid = :block_uid, trust = :trust,"
                        " amount = :amount", [] (auto& s, auto& b) {
                            s, use(b.uid, "uid"), use(b.accountUid, "account_uid"),
                                    use(b.walletUid, "wallet_uid"), use(b.type, "type"),
                                    use(b.date, "date"), use(b.senders, "senders"),
                                    use(b.receivers, "recipients"), use(b.amount, "amount"),
                                    use(b.fees, "fees"), use(b.blockUid, "block_uid"),
                                    use(b.currencyName, "currency_name"),
                                    use(b.serializedTrust, "trust");
                        });
        const StatementDeclaration<BlockBinding> BulkInsertDatabaseHelper::UPSERT_BLOCK =
                db::stmt<BlockBinding>(
                        "INSERT INTO blocks VALUES(:uid, :hash, :height, :time, :currency_name)"
                        " ON CONFLICT DO NOTHING",
                        [] (auto& s, auto&  b) {
                            s, use(b.uid), use(b.hash), use(b.height), use(b.time),
                                    use(b.currencyName);
                        });

        void BulkInsertDatabaseHelper::updateBlock(soci::session& sql, const Block &block) {
            PreparedStatement<BlockBinding> stmt;
            UPSERT_BLOCK(sql, stmt);
            stmt.bindings.update(block);
            stmt.execute();
        }


        void OperationBinding::update(const Operation &operation) {
            amount.push_back(operation.amount.toHexString());
            blockUid.push_back(operation.block.map<std::string>([] (const Block& block) {
                return block.getUid();
            }));
            std::string trust;
            serialization::saveBase64<TrustIndicator>(*operation.trust, trust);
            serializedTrust.push_back(trust);
            std::string separator(",");
            std::stringstream sndrs;
            std::stringstream rcvrs;
            strings::join(operation.senders, sndrs, separator);
            strings::join(operation.recipients, rcvrs, separator);
            senders.push_back(sndrs.str());
            receivers.push_back(rcvrs.str());
            fees.push_back(operation.fees.getValueOr(BigInt::ZERO).toHexString());

            uid.push_back(operation.uid);
            accountUid.push_back(operation.accountUid);
            walletUid.push_back(operation.walletUid);
            date.push_back(operation.date);
            currencyName.push_back(operation.currencyName);
            type.push_back(api::to_string(operation.type));
        }

        void OperationBinding::reset() {
            type.clear();
            senders.clear();
            receivers.clear();
            amount.clear();
            fees.clear();
            blockUid.clear();
            serializedTrust.clear();
            uid.clear();
            accountUid.clear();
            walletUid.clear();
            date.clear();
            currencyName.clear();
        }

        void BlockBinding::update(const Block &b) {
            auto u = BlockDatabaseHelper::createBlockUid(b);
            uid.push_back(u);
            hash.push_back(b.hash);
            height.push_back(b.height);
            time.push_back(b.time);
            currencyName.push_back(b.currencyName);
        }

        void BlockBinding::clear() {
            uid.clear();
            hash.clear();
            height.clear();
            time.clear();
            currencyName.clear();
        }
    }
}