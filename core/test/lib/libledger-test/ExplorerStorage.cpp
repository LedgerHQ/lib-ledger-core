#include "ExplorerStorage.hpp"
#include <algorithm>
#include <rapidjson/reader.h>
#include <rapidjson/writer.h>
#include <rapidjson/stream.h>
#include <fmt/format.h>
#include <wallet/bitcoin/explorers/api/TransactionParser.hpp>
#include <utils/DateUtils.hpp>

namespace ledger {
    namespace core {
        namespace test {

            bool transactionContainAddresses(const BitcoinLikeBlockchainExplorerTransaction& tr, const std::unordered_set<std::string>& addresses) {
                return
                    (std::find_if(
                        tr.inputs.begin(),
                        tr.inputs.end(),
                        [&](const auto& input) {return input.address.hasValue() && (addresses.find(input.address.getValue()) != addresses.end()); }) != tr.inputs.end())
                    ||
                    (std::find_if(
                        tr.outputs.begin(),
                        tr.outputs.end(),
                        [&](const auto& output) {return output.address.hasValue() && (addresses.find(output.address.getValue()) != addresses.end()); }) != tr.outputs.end());
            }

            void ExplorerStorage::addTransaction(const std::string& jsonTransaction) {
                std::string dummy;
                BitcoinLikeBlockchainExplorerTransaction transaction;
                TransactionParser parser(dummy);
                parser.init(&transaction);
                rapidjson::Reader reader;
                rapidjson::StringStream ss(jsonTransaction.c_str());
                if (reader.Parse<rapidjson::kParseNumbersAsStringsFlag>(ss, parser).IsError())
                    throw std::runtime_error("Can't parse transaction");
                if (transaction.block.isEmpty()) {
                    _memPool.push_back(std::make_pair(transaction, jsonTransaction));
                    return;
                }
                _transactions.push_back(std::make_pair(transaction, jsonTransaction));
                std::sort(
                    _transactions.begin(),
                    _transactions.end(),
                    [](const auto& a, const auto& b) { return  a.first.block.getValue().height < b.first.block.getValue().height; });
            }

            void ExplorerStorage::removeTransaction(const std::string& hash) {
                auto it = std::find_if(_transactions.begin(), _transactions.end(), [&](auto& x) {return x.first.hash == hash; });
                if (it != _transactions.end())
                    _transactions.erase(it);
                it = std::find_if(_memPool.begin(), _memPool.end(), [&](auto& x) {return x.first.hash == hash; });
                if (it != _memPool.end())
                    _memPool.erase(it);
            }

            std::vector<std::string> ExplorerStorage::getTransactions(
                const std::vector<std::string>& addresses,
                const std::string& blockHash) {
                std::unordered_set<std::string> addrs(addresses.begin(), addresses.end());
                std::vector<std::string> result;
                auto block = std::find_if(_transactions.begin(), _transactions.end(), [&](auto& x) {
                    return x.first.block.getValue().hash == blockHash;
                });
                std::vector<std::pair<BitcoinLikeBlockchainExplorerTransaction, std::string>> confirmedTransactions;
                if (blockHash != "")
                    std::copy_if(_transactions.begin(), _transactions.end(), std::back_inserter(confirmedTransactions), [&](auto& x) {
                        return x.first.block.getValue().height > block->first.block->height;
                    });
                else
                   confirmedTransactions = _transactions;
                for (auto& tx : confirmedTransactions) {
                    if (transactionContainAddresses(tx.first, addrs))
                        result.push_back(tx.second);
                }
                for (auto& tx : _memPool) {
                    if (transactionContainAddresses(tx.first, addrs))
                        result.push_back(tx.second);
                }
                return result;
            }

            std::string ExplorerStorage::getLastBlock() {
                if (_transactions.empty())
                    return "{}";
                rapidjson::StringBuffer s;
                rapidjson::Writer<rapidjson::StringBuffer> writer(s);
                auto lastBlock = _transactions.back().first.block.getValue();
                writer.StartObject();
                writer.Key("hash");
                writer.String(lastBlock.hash.c_str());
                writer.Key("height");
                writer.Uint64(lastBlock.height);
                writer.Key("time");
                writer.String(DateUtils::toJSON(lastBlock.time).c_str());
                writer.Key("txs");
                writer.StartArray();
                writer.EndArray();
                writer.EndObject();

                return s.GetString();
            }
        }
    }
}