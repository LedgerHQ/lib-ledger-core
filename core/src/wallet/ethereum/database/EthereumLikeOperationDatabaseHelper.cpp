/*
 *
 * EthereumLikeOperationDatabaseHelper.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 06/01/2021.
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

#include "EthereumLikeOperationDatabaseHelper.hpp"
#include <database/soci-date.h>
#include <database/soci-option.h>
#include <api/BigInt.hpp>
#include <crypto/SHA256.hpp>
#include <unordered_set>
#include <database/soci-backend-utils.h>
#include <debug/Benchmarker.h>
#include <wallet/common/database/BulkInsertDatabaseHelper.hpp>
#include <wallet/ethereum/database/EthereumLikeTransactionDatabaseHelper.h>
#include <api/ERC20Token.hpp>

using namespace soci;

namespace {
    using namespace ledger::core;

    using Transaction = EthereumLikeBlockchainExplorerTransaction;

    // Transaction
    struct EthereumTransactionBinding {
        std::vector<std::string> uid;
        std::vector<std::string> hash;
        std::vector<uint64_t> nonce;
        std::vector<std::string> value;
        std::vector<Option<std::string>> block;
        std::vector<std::chrono::system_clock::time_point> time;
        std::vector<std::string> sender;
        std::vector<std::string> receiver;
        std::vector<std::string> inputData;
        std::vector<std::string> gasPrice;
        std::vector<std::string> gasLimit;
        std::vector<std::string> gasUsed;
        std::vector<uint64_t> confirmations;
        std::vector<uint64_t> status;

        void update(const Option<std::string> blockUid, const std::string& txUid, const Transaction& tx) {
            uid.push_back(txUid);
            hash.push_back(tx.hash);
            nonce.push_back(tx.nonce);
            value.push_back(tx.value.toHexString());
            block.push_back(blockUid);
            time.push_back(tx.receivedAt);
            sender.push_back(tx.sender);
            receiver.push_back(tx.receiver);
            inputData.push_back(hex::toString(tx.inputData));
            gasPrice.push_back(tx.gasPrice.toHexString());
            gasLimit.push_back(tx.gasLimit.toHexString());
            gasUsed.push_back(tx.gasUsed.getValueOr(BigInt::ZERO).toHexString());
            confirmations.push_back(tx.confirmations);
            status.push_back(tx.status);
        }

    };
    const auto UPSERT_ETHEREUM_TRANSACTION = db::stmt<EthereumTransactionBinding>(
        "INSERT INTO ethereum_transactions VALUES("
        " :tx_uid, :hash, :nonce, :value, :block_uid, :time, :sender, :receiver,"
        " :input_data, :gas_price, :gas_limit, :gas_used, :confirmations,"
        " :status"
        ") ON CONFLICT(transaction_uid) DO UPDATE SET"
        " block_uid = :block_uid, status = :status, gas_used = :gas_used",
        [] (auto& s, auto& b) {
            s,
            use(b.uid, "tx_uid"),
            use(b.hash, "hash"),
            use(b.nonce, "nonce"),
            use(b.value, "value"),
            use(b.block, "block_uid"),
            use(b.time, "time"),
            use(b.sender, "sender"),
            use(b.receiver, "receiver"),
            use(b.inputData, "input_data"),
            use(b.gasPrice, "gas_price"),
            use(b.gasLimit, "gas_limit"),
            use(b.gasUsed, "gas_used"),
            use(b.confirmations, "confirmations"),
            use(b.status, "status");
        }
    );

    // Ethereum operation
    struct EthereumOperationBinding {
        std::vector<std::string> uid;
        std::vector<std::string> txUid;
        std::vector<std::string> hash;

        void update(const std::string& opUid, const std::string& tUid, const std::string& tHash) {
            uid.push_back(opUid);
            txUid.push_back(tUid);
            hash.push_back(tHash);
        }
    };
    const auto UPSERT_ETHEREUM_OPERATION = db::stmt<EthereumOperationBinding>(
        "INSERT INTO ethereum_operations VALUES("
        ":uid, :tx_uid, :tx_hash"
        ") ON CONFLICT DO NOTHING",
        [] (auto& s, auto& b) {
            s, use(b.uid), use(b.txUid), use(b.hash);
        }
    );

    // ERC20 Token
    struct ERC20TokenBinding {
        std::vector<std::string> contractAddress;
        std::vector<std::string> name;
        std::vector<std::string> symbol;
        std::vector<int32_t> numberOfDecimal;

        void update(const api::ERC20Token& token) {
            contractAddress.push_back(token.contractAddress);
            name.push_back(token.name);
            symbol.push_back(token.symbol);
            numberOfDecimal.push_back(token.numberOfDecimal);
        }
    };
    const auto UPSERT_TOKEN = db::stmt<ERC20TokenBinding>(
        "INSERT INTO erc20_tokens VALUES(:contract_address, :name, :symbol, "
        ":number_of_decimal) ON CONFLICT DO NOTHING",
        [] (auto& s, auto& b) {
            s, use(b.contractAddress), use(b.name), use(b.symbol),
            use(b.numberOfDecimal);
        }
    );

    // ERC20 accounts
    struct ERC20AccountBinding {
        std::vector<std::string> uid;
        std::vector<std::string> accountUid;
        std::vector<std::string> contractAddress;

        void update(const std::string& accUid, const std::shared_ptr<ERC20LikeAccount>& acc) {
            uid.push_back(acc->getUid());
            accountUid.push_back(accUid);
            contractAddress.push_back(acc->getToken().contractAddress);
        }
    };

    const auto UPSERT_ERC20_ACCOUNT = db::stmt<ERC20AccountBinding>(
        "INSERT INTO erc20_accounts VALUES("
        ":uid, :ethereum_account_uid, :contract_address"
        ") ON CONFLICT DO NOTHING",
        [] (auto& s, auto& b) {
            s, use(b.uid), use(b.accountUid), use(b.contractAddress);
        }
    );

    // ERC 20 operations
    struct ERC20OperationBinding {
        std::vector<std::string> ercOpUid;
        std::vector<std::string> ethOpUid;
        std::vector<std::string> accountUid;
        std::vector<std::string> opType;
        std::vector<std::string> hash;
        std::vector<std::string> nonce;
        std::vector<std::string> value;
        std::vector<std::chrono::system_clock::time_point> time;
        std::vector<std::string> gasPrice;
        std::vector<std::string> gasLimit;
        std::vector<std::string> gasUsed;
        std::vector<int64_t> blockHeight;
        std::vector<std::string> sender;
        std::vector<std::string> receiver;
        std::vector<std::string> data;
        std::vector<int32_t> status;

        void update(const std::string& ercAccUid, ERC20LikeOperation& op) {
            ercOpUid.push_back(op.getOperationUid());
            ethOpUid.push_back(op.getETHOperationUid());
            accountUid.push_back(ercAccUid);
            opType.push_back(api::to_string(op.getOperationType()));
            hash.push_back(op.getHash());
            nonce.push_back(op.getNonce()->toString(16));
            value.push_back(op.getValue()->toString(16));
            time.push_back(op.getTime());
            gasPrice.push_back(op.getGasPrice()->toString(16));
            gasLimit.push_back(op.getGasLimit()->toString(16));
            blockHeight.push_back(op.getBlockHeight().value_or(0));
            sender.push_back(op.getSender());
            receiver.push_back(op.getReceiver());
            data.push_back(hex::toString(op.getData()));
            gasUsed.push_back(op.getUsedGas()->toString(16));
            status.push_back(op.getStatus());
        }
    };

    const auto UPSERT_ERC20_OPERATION = db::stmt<ERC20OperationBinding>(
            "INSERT INTO erc20_operations VALUES("
             ":uid, :eth_op_uid, :account_uid, :op_type, :hash, :nonce, :value, :date, :sender,"
             ":receiver, :data, :gas_price, :gas_limit, :gas_used, :status, :block_height"
             ") ON CONFLICT(uid) DO UPDATE SET "
             " status = :status, gas_used = :gas_used",
        [] (auto& s, auto& b) {
            s,
            use(b.ercOpUid, "uid"),
            use(b.ethOpUid, "eth_op_uid"),
            use(b.accountUid, "account_uid"),
            use(b.opType, "op_type"),
            use(b.hash, "hash"),
            use(b.nonce, "nonce"),
            use(b.value, "value"),
            use(b.time, "date"),
            use(b.sender, "sender"),
            use(b.receiver, "receiver"),
            use(b.data, "data"),
            use(b.gasPrice, "gas_price"),
            use(b.gasLimit, "gas_limit"),
            use(b.gasUsed, "gas_used"),
            use(b.status, "status"),
            use(b.blockHeight, "block_height");
        }
    );

    // Internal operations
    struct InternalOperationBinding {
        std::vector<std::string> internalTxUid;
        std::vector<std::string> opUid;
        std::vector<std::string> opType;
        std::vector<std::string> value;
        std::vector<std::string> from;
        std::vector<std::string> to;
        std::vector<std::string> gasLimit;
        std::vector<std::string> gasUsed;
        std::vector<std::string> inputData;

        void update(int index, const std::string& oUid, const InternalTx& tx,
                    const EthereumLikeBlockchainExplorerTransaction& transaction,
                    const std::string& accountAddress) {
            auto type = tx.from == accountAddress ? api::OperationType::SEND :
                        tx.to == accountAddress ? api::OperationType::RECEIVE :
                        api::OperationType::NONE;
            auto uid = OperationDatabaseHelper::createUid(oUid, fmt::format("{}-{}-{}",
                    tx.from, hex::toString(tx.inputData), index), type);
            if (tx.from != transaction.sender || tx.to != transaction.receiver ||
                hex::toString(tx.inputData )!= hex::toString(transaction.inputData)) {
                internalTxUid.push_back(uid);
                opUid.push_back(oUid);
                opType.push_back(api::to_string(type));
                value.push_back(tx.value.toHexString());
                from.push_back(tx.from);
                to.push_back(tx.to);
                gasLimit.push_back(tx.gasLimit.toHexString());
                gasUsed.push_back(tx.gasUsed.map<std::string>([](const auto &g) {
                    return g.toHexString();
                }).getValueOr("00"));
                inputData.push_back(hex::toString(tx.inputData));
            }
        }
    };
    const auto UPSERT_INTERNAL_OPERATION = db::stmt<InternalOperationBinding>(
            "INSERT INTO internal_operations VALUES("
            ":uid, :eth_op_uid, :type, :value, :sender, :receiver, "
            ":gas_limit, :gas_used, :input_data) ON CONFLICT DO NOTHING"
            , [] (auto& s, auto& b) {
                s, use(b.internalTxUid), use(b.opUid), use(b.opType), use(b.value),
                use(b.from), use(b.to), use(b.gasLimit), use(b.gasUsed),
                use(b.inputData);
            }
    );

    const auto UPSERT_OPERATION = db::stmt<OperationBinding>(
            "INSERT INTO operations VALUES("
            ":uid, :account_uid, :wallet_uid, :type, :date, :senders, :recipients, :amount,"
            ":fees, :block_uid, :currency_name, :trust"
            ") ON CONFLICT(uid) DO UPDATE SET block_uid = :block_uid, trust = :trust,"
            " amount = :amount, fees = :fees ", [] (auto& s, auto& b) {
                s, use(b.uid, "uid"), use(b.accountUid, "account_uid"),
                    use(b.walletUid, "wallet_uid"), use(b.type, "type"),
                    use(b.date, "date"), use(b.senders, "senders"),
                    use(b.receivers, "recipients"), use(b.amount, "amount"),
                    use(b.fees, "fees"), use(b.blockUid, "block_uid"),
                    use(b.currencyName, "currency_name"),
                    use(b.serializedTrust, "trust");
            });
}

namespace ledger {
    namespace core {

        void EthereumLikeOperationDatabaseHelper::bulkInsert(session &sql,
                const std::vector<Operation> &ops, const std::string& accountAddress) {
            if (ops.empty())
                return;

            PreparedStatement<OperationBinding> operationStmt;
            PreparedStatement<BlockBinding> blockStmt;
            PreparedStatement<EthereumTransactionBinding> ethTxStmt;
            PreparedStatement<EthereumOperationBinding> ethOpStmt;
            PreparedStatement<ERC20AccountBinding> erc20AccountStmt;
            PreparedStatement<ERC20OperationBinding> erc20OpStmt;
            PreparedStatement<InternalOperationBinding> internalOpStmt;
            PreparedStatement<ERC20TokenBinding> tokenStmt;

            UPSERT_OPERATION(sql, operationStmt);
            BulkInsertDatabaseHelper::UPSERT_BLOCK(sql, blockStmt);
            UPSERT_ERC20_ACCOUNT(sql, erc20AccountStmt);
            UPSERT_ERC20_OPERATION(sql, erc20OpStmt);
            UPSERT_ETHEREUM_OPERATION(sql, ethOpStmt);
            UPSERT_ETHEREUM_TRANSACTION(sql, ethTxStmt);
            UPSERT_INTERNAL_OPERATION(sql, internalOpStmt);
            UPSERT_TOKEN(sql, tokenStmt);

            for (const auto& op : ops) {
                if (op.block.nonEmpty())  {
                    blockStmt.bindings.update(op.block.getValue());
                }
                const auto blockUid = op.block.map<std::string>([] (const auto& b) {
                    return b.getUid();
                });
                const auto accountUid = op.accountUid;
                const auto& tx = op.ethereumTransaction.getValue();
                const auto opUid = op.uid;
                auto txUid = EthereumLikeTransactionDatabaseHelper::createEthereumTransactionUid(op.accountUid, tx.hash);
                const auto& data = std::dynamic_pointer_cast<EthereumOperationAttachedData>(op.attachedData);
                auto internalOpIndex = 0;
                for (const auto& internalTx : tx.internalTransactions) {
                    internalOpStmt.bindings.update(internalOpIndex, opUid, internalTx, tx, accountAddress);
                    internalOpIndex += 1;
                }
                if (data) {
                    for (const auto &erc20Account : data->accounts) {
                        auto token = erc20Account->getToken();
                        tokenStmt.bindings.update(token);
                        erc20AccountStmt.bindings.update(accountUid, erc20Account);
                    }
                    for (auto &erc20Op : data->erc20Operations) {
                        erc20OpStmt.bindings.update(
                                std::get<0>(erc20Op), std::get<1>(erc20Op));
                    }
                }
                ethTxStmt.bindings.update(blockUid, txUid, tx);
                ethOpStmt.bindings.update(op.uid, txUid, tx.hash);
                operationStmt.bindings.update(op);
            }
            // Block
            if (!blockStmt.bindings.uid.empty())
                blockStmt.execute();
            // ERC Token
            if (!tokenStmt.bindings.name.empty())
                tokenStmt.execute();
            // ERC20 accounts
            if (!erc20AccountStmt.bindings.uid.empty())
                erc20AccountStmt.execute();
            // Transaction
            ethTxStmt.execute();
            // Operations
            operationStmt.execute();
            // ETH operation
            ethOpStmt.execute();
            // Internal op
            if (!internalOpStmt.bindings.opUid.empty())
                internalOpStmt.execute();
            // ERC20 operation
            if (!erc20OpStmt.bindings.hash.empty())
                erc20OpStmt.execute();

        }

    }
}