/*
 *
 * GaiaCosmosLikeBlockchainExplorer.cpp
 * ledger-core
 *
 * Created by Pierre Pollastri on 29/11/2019.
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

#include <wallet/cosmos/explorers/GaiaCosmosLikeBlockchainExplorer.hpp>

#include <async/algorithm.h>
#include <api/Configuration.hpp>
#include <utils/Exception.hpp>

#include <wallet/cosmos/explorers/RpcsParsers.hpp>
#include <wallet/cosmos/CosmosLikeCurrencies.hpp>
#include <wallet/cosmos/CosmosLikeConstants.hpp>

#include <numeric>
#include <algorithm>

namespace ledger {
    namespace core {

        using MsgType = cosmos::MsgType;

        static CosmosLikeBlockchainExplorer::TransactionFilter eventAttribute(
            const char eventType[], const char attributeKey[]) {
            return fmt::format("{}.{}", eventType, attributeKey);
        }

        CosmosLikeBlockchainExplorer::TransactionFilter
        GaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
            const char eventType[], const char attributeKey[], const std::string& value) {
            return fmt::format("{}={}", eventAttribute(eventType, attributeKey), value);
        }

        CosmosLikeBlockchainExplorer::TransactionFilter
        GaiaCosmosLikeBlockchainExplorer::fuseFilters(
            std::initializer_list<std::experimental::string_view> filters) {
            std::string filter;
            return std::accumulate(filters.begin(), filters.end(), filter,
                                   [](std::string acc, std::experimental::string_view val) {
                                       if (acc.empty()) {
                                           return std::string(val.data());
                                       }
                                       return acc + "&" + val.data();
                                   });
        }

        // Address at the end of the filter is...
        static const std::vector<CosmosLikeBlockchainExplorer::TransactionFilter> GAIA_FILTER {
        };

        GaiaCosmosLikeBlockchainExplorer::GaiaCosmosLikeBlockchainExplorer(
                const std::shared_ptr<api::ExecutionContext> &context, const std::shared_ptr<HttpClient> &http,
                const api::CosmosLikeNetworkParameters &parameters,
                const std::shared_ptr<api::DynamicObject> &configuration) :
                DedicatedContext(context),
                CosmosLikeBlockchainExplorer(configuration, {api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT}),
                _http(http), _parameters(parameters) {

        }

        const std::vector<CosmosLikeBlockchainExplorer::TransactionFilter> &
        GaiaCosmosLikeBlockchainExplorer::getTransactionFilters() {
            return GAIA_FILTER;
        }

        FuturePtr<cosmos::Block> GaiaCosmosLikeBlockchainExplorer::getBlock(uint64_t &blockHeight) {
            return _http->GET(fmt::format("/blocks/{}", blockHeight)).json(true).mapPtr<cosmos::Block>(getContext(), [=] (const HttpRequest::JsonResult& response) {
                auto result = std::make_shared<cosmos::Block>();
                const auto& document = std::get<1>(response)->GetObject();
                rpcs_parsers::parseBlock(document, currencies::ATOM.name, *result);
                return result;
            });
        }

        FuturePtr<ledger::core::cosmos::Account>
        GaiaCosmosLikeBlockchainExplorer::getAccount(const std::string &account) {
            return _http->GET(fmt::format("/auth/accounts/{}", account)).json(true).mapPtr<cosmos::Account>(getContext(), [=] (const HttpRequest::JsonResult& response) {
                auto result = std::make_shared<cosmos::Account>();
                const auto& document = std::get<1>(response)->GetObject();
                // TODO : raise a clean exception when document has no "result" member
                rpcs_parsers::parseAccount(document["result"], *result);
                return result;
            });
        }

        FuturePtr<cosmos::Block> GaiaCosmosLikeBlockchainExplorer::getCurrentBlock() {
            return _http->GET(fmt::format("/blocks/latest")).json(true)
            .map<std::shared_ptr<cosmos::Block>>(getContext(),
             [=] (const HttpRequest::JsonResult& response) {
                 auto result = std::make_shared<cosmos::Block>();
                 const auto& document = std::get<1>(response)->GetObject();
                 rpcs_parsers::parseBlock(document, currencies::ATOM.name, *result);
                 return result;
             });
        }

        Future<cosmos::TransactionList> GaiaCosmosLikeBlockchainExplorer::getTransactions(
            const CosmosLikeBlockchainExplorer::TransactionFilter& filter, int page, int limit) const {
            return _http->GET(fmt::format("/txs?{}&page={}&limit={}", filter, page, limit))
                .json(true)
                .map<cosmos::TransactionList>(
                    getContext(), [=](const HttpRequest::JsonResult& response) {
                        cosmos::TransactionList result;
                        const auto& document = std::get<1>(response)->GetObject();
                        // TODO : raise a clean exception when document has no "txs" member
                        const auto& transactions = document["txs"].GetArray();
                        for (const auto& node : transactions) {
                            auto tx = std::make_shared<cosmos::Transaction>();
                            rpcs_parsers::parseTransaction(node, *tx);
                            result.emplace_back(tx);
                        }
                        return result;
                    });
        }

        FuturePtr<cosmos::Transaction>
        GaiaCosmosLikeBlockchainExplorer::getTransactionByHash(const std::string &hash) {
            return _http->GET(fmt::format("/txs/{}", hash))
                .json(true)
                .mapPtr<cosmos::Transaction>(getContext(), [=] (const HttpRequest::JsonResult& response) {
                    const auto& document = std::get<1>(response)->GetObject();
                    auto tx = std::make_shared<cosmos::Transaction>();
                    rpcs_parsers::parseTransaction(document, *tx);
                    return tx;
                });
        }

        Future<cosmos::TransactionList>
        GaiaCosmosLikeBlockchainExplorer::getTransactionsForAddress(const std::string& address,
                                                                    uint32_t fromBlockHeight) const {


            auto blockHeightFilter = filterWithAttribute(kTx, kMinHeight, std::to_string(fromBlockHeight));

            // NOTE: MsgUn/Re/Delegate + MsgWithdrawDelegatorReward are all covered by 'message.sender'
            auto sent_transactions = getTransactions(
                fuseFilters({
                    // TODO : commented out until our backend supports the filter
                    // blockHeightFilter,
                    filterWithAttribute(kEventTypeMessage, kAttributeKeySender, address)
                }),
                1,
                50);
            auto received_transactions = getTransactions(
                fuseFilters({
                    // TODO : commented out until our backend supports the filter
                    // blockHeightFilter,
                    filterWithAttribute(kEventTypeTransfer, kAttributeKeyRecipient, address)
                }),
                1,
                50);
            std::vector<Future<cosmos::TransactionList>> transaction_promises(
                {sent_transactions,
                 received_transactions});

            return async::sequence(getContext(), transaction_promises)
                .flatMap<cosmos::TransactionList>(getContext(), [](auto& vector_of_lists) {
                    cosmos::TransactionList result;
                    for (auto&& list : vector_of_lists) {
                        // Forced to copy because of flatMap API
                        result.insert(result.end(), list.begin(), list.end());
                    }
                    return Future<cosmos::TransactionList>::successful(result);
                });
        }

        Future<cosmos::TransactionList> GaiaCosmosLikeBlockchainExplorer::getTransactionsForAddresses(
            const std::vector<std::string>& addresses, uint32_t fromBlockHeight) const {
            std::vector<Future<cosmos::TransactionList>> address_transactions;
            std::transform(addresses.begin(), addresses.end(), std::back_inserter(address_transactions),
                           [&] (const auto& address) -> Future<cosmos::TransactionList> {
                               return getTransactionsForAddress(address, fromBlockHeight);
                           });
            return async::sequence(getContext(), address_transactions)
                .flatMap<cosmos::TransactionList>(getContext(), [](auto& vector_of_lists) {
                    cosmos::TransactionList result;
                    for (auto&& list : vector_of_lists) {
                        // Forced to copy because of flatMap API
                        result.insert(result.end(), list.begin(), list.end());
                    }
                    return Future<cosmos::TransactionList>::successful(result);
                });
        }

        FuturePtr<cosmos::TransactionsBulk>
        GaiaCosmosLikeBlockchainExplorer::getTransactions(
            const std::vector<std::string>& addresses,
            uint32_t fromBlockHeight,
            Option<void*> session) {

            return getTransactionsForAddresses(addresses, fromBlockHeight)
                .mapPtr<cosmos::TransactionsBulk>(
                    getContext(), [](auto& transaction_list) {
                        std::vector<cosmos::Transaction> c_transaction_list;
                        auto result =
                            std::make_shared<cosmos::TransactionsBulk>();
                        std::transform(
                            transaction_list.begin(),
                            transaction_list.end(),
                            std::back_inserter(c_transaction_list),
                            [](auto transaction) -> cosmos::Transaction { return *transaction; });
                        result->transactions = c_transaction_list;
                        result->hasNext = false;
                        return result;
                    });
        }

        Future<void *> GaiaCosmosLikeBlockchainExplorer::startSession() {
            return Future<void *>::successful(new std::string("", 0));
        }

        Future<Unit> GaiaCosmosLikeBlockchainExplorer::killSession(void* session) {
            return Future<Unit>::successful(unit);
        }

        FuturePtr<ledger::core::Block> GaiaCosmosLikeBlockchainExplorer::getCurrentBlock() const {
            return _http->GET("/blocks/latest")
                .json(true)
                .mapPtr<ledger::core::Block>(getContext(), [=](const HttpRequest::JsonResult& response) {
                    const auto& document = std::get<1>(response)->GetObject();
                    auto block = std::make_shared<cosmos::Block>();
                    rpcs_parsers::parseBlock(document, currencies::ATOM.name, *block);
                    return std::make_shared<ledger::core::Block>(*block);
                });
        }

        Future<Bytes> GaiaCosmosLikeBlockchainExplorer::getRawTransaction(
            const String& transactionHash) {
            throw(Exception(
                api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Get Raw Transaction is unimplemented"));
        }

        FuturePtr<cosmos::Transaction> GaiaCosmosLikeBlockchainExplorer::getTransactionByHash(
            const String& transactionHash) const {
            return getTransactionByHash(transactionHash);
        }

        Future<String> GaiaCosmosLikeBlockchainExplorer::pushTransaction(const std::vector<uint8_t>& transaction) {
            std::unordered_map<std::string, std::string> headers{{"Accept", "application/json"}};

            return _http->POST("/txs", transaction, headers)
                        .json().template map<String>(_executionContext, [] (const HttpRequest::JsonResult& result) -> String {
                            auto& json = *std::get<1>(result);
                            return json["result"].GetString();
                        });
        }

        Future<int64_t> GaiaCosmosLikeBlockchainExplorer::getTimestamp() const {
            return Future<int64_t>::successful(
                std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count());
        }

        // Balances
        /// Get Total Balance
        FuturePtr<BigInt> GaiaCosmosLikeBlockchainExplorer::getTotalBalance(
            const std::string &account) const
        {
            std::vector<FuturePtr<BigInt>> balance_promises({
                getSpendableBalance(account),
                getDelegatedBalance(account),
                getUnbondingBalance(account),
                getPendingRewards(account)
                });

            return async::sequence(getContext(), balance_promises)
                .flatMap<std::shared_ptr<BigInt>>(getContext(), [](auto& vector_of_balances) {
                    BigInt result;
                    for (const auto& balance : vector_of_balances) {
                        result = result + *balance;
                    }
                    return FuturePtr<BigInt>::successful(std::make_shared<BigInt>(result));
                });
        }
        /// Get total balance in delegation
        FuturePtr<BigInt> GaiaCosmosLikeBlockchainExplorer::getDelegatedBalance(
            const std::string &account) const
        {
            const auto endpoint = fmt::format(cosmos::constants::kGaiaDelegationsEndpoint, account);

            std::unordered_map<std::string, std::string> headers{
                {"Content-Type", "application/json"}};
            const bool jsonParseNumbersAsString = true;

            return _http->GET(endpoint, headers)
                .json(jsonParseNumbersAsString)
                .mapPtr<BigInt>(
                    getContext(),
                    [endpoint](const HttpRequest::JsonResult &result) -> std::shared_ptr<BigInt> {
                        auto &json = *std::get<1>(result);
                        // TODO : clean error handling

                        const auto &del_val_entries = json.GetObject()["result"].GetArray();
                        BigInt total_amt = BigInt::ZERO;
                        for (const auto& delegation_entry : del_val_entries) {
                            total_amt = total_amt + BigInt::fromDecimal(delegation_entry.GetObject()["balance"].GetString());
                        }

                        return std::make_shared<BigInt>(total_amt);
                    });
        }
        /// Get total pending rewards
        FuturePtr<BigInt> GaiaCosmosLikeBlockchainExplorer::getPendingRewards(
            const std::string &account) const
        {
            const auto endpoint = fmt::format(cosmos::constants::kGaiaRewardsEndpoint, account);

            std::unordered_map<std::string, std::string> headers{
                {"Content-Type", "application/json"}};
            const bool jsonParseNumbersAsString = true;

            return _http->GET(endpoint, headers)
                .json(jsonParseNumbersAsString)
                .mapPtr<BigInt>(
                    getContext(),
                    [endpoint](const HttpRequest::JsonResult &result) -> std::shared_ptr<BigInt> {
                        auto &json = *std::get<1>(result);

                        if (!json.IsObject() ||
                            !json.GetObject().HasMember("result") ||
                            !json.GetObject()["result"].IsObject() ||
                            !json.GetObject()["result"].GetObject().HasMember("total") ||
                            !json.GetObject()["result"].GetObject()["total"].IsArray()
                            ) {
                            throw make_exception(
                                api::ErrorCode::HTTP_ERROR,
                                fmt::format("Failed to get total for {}", endpoint));
                        }

                        if (json.GetObject()["result"].GetObject()["total"].GetArray().Size() <= 0) {
                            return std::make_shared<BigInt>(BigInt::ZERO);
                        }

                        std::string floating_amount = json.GetObject()["result"].GetObject()["total"]
                            .GetArray()[0]
                            .GetObject()["amount"]
                            .GetString();
                        floating_amount.erase(floating_amount.find('.'), std::string::npos);
                        return std::make_shared<BigInt>(floating_amount);
                    });
        }

        /// Get total unbonding balance
        FuturePtr<BigInt> GaiaCosmosLikeBlockchainExplorer::getUnbondingBalance(
            const std::string &account) const
        {
            const auto endpoint = fmt::format(cosmos::constants::kGaiaUnbondingsEndpoint, account);

            std::unordered_map<std::string, std::string> headers{
                {"Content-Type", "application/json"}};
            const bool jsonParseNumbersAsString = true;

            return _http->GET(endpoint, headers)
                .json(jsonParseNumbersAsString)
                .mapPtr<BigInt>(
                    getContext(),
                    [endpoint](const HttpRequest::JsonResult &result) -> std::shared_ptr<BigInt> {
                        auto &json = *std::get<1>(result);
                        // TODO : clean error handling

                        const auto &del_entries = json.GetObject()["result"].GetArray();
                        BigInt total_amt = BigInt::ZERO;
                        for (const auto& del_val_entries : del_entries) {
                            const auto &entries = del_val_entries.GetObject()["entries"].GetArray();
                            for (const auto &unbonding_entry : entries) {
                                total_amt = total_amt + BigInt::fromDecimal(unbonding_entry.GetObject()["balance"].GetString());
                            }
                        }

                        return std::make_shared<BigInt>(total_amt);
                    });
        }
        /// Get total available (spendable) balance
        FuturePtr<BigInt> GaiaCosmosLikeBlockchainExplorer::getSpendableBalance(
            const std::string &account) const
        {
            const auto endpoint = fmt::format(cosmos::constants::kGaiaBalancesEndpoint, account);

            std::unordered_map<std::string, std::string> headers{
                {"Content-Type", "application/json"}};
            const bool jsonParseNumbersAsString = true;

            return _http->GET(endpoint, headers)
                .json(jsonParseNumbersAsString)
                .mapPtr<BigInt>(
                    getContext(),
                    [endpoint](const HttpRequest::JsonResult &result) -> std::shared_ptr<BigInt> {
                        auto &json = *std::get<1>(result);
                        // TODO : clean error handling

                        const auto &balances = json.GetObject()["result"].GetArray();
                        BigInt total_amt = BigInt::ZERO;
                        // HACK : Assuming only uatom is in the balances array
                        for (const auto& balance_entry : balances) {
                            total_amt = total_amt + BigInt::fromDecimal(balance_entry.GetObject()[cosmos::constants::kAmount].GetString());
                        }

                        return std::make_shared<BigInt>(total_amt);
                    });
        }

            // Validators
        Future<cosmos::ValidatorList> GaiaCosmosLikeBlockchainExplorer::getActiveValidatorSet() const {
            const bool parseJsonNumbersAsStrings = true;
            auto basicValidatorList =_http->GET("/staking/validators?status=bonded&page=1&limit=130")
                .json(parseJsonNumbersAsStrings)
                .map<cosmos::ValidatorList>(
                    getContext(), [=](const HttpRequest::JsonResult& response) {
                        cosmos::ValidatorList result;
                        const auto& document = std::get<1>(response)->GetObject();
                        // TODO : raise a clean exception when document has no "txs" member
                        const auto& validators = document["result"].GetArray();
                        for (const auto& node : validators) {
                            cosmos::Validator val;
                            rpcs_parsers::parseValidatorSetEntry(node.GetObject(), val);
                            result.emplace_back(std::move(val));
                        }
                        return result;
                    });

            return basicValidatorList;
        }

        }  // namespace core
}
