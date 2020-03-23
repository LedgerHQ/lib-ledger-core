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

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include <async/algorithm.h>
#include <api/Configuration.hpp>
#include <utils/Exception.hpp>

#include <wallet/cosmos/explorers/RpcsParsers.hpp>
#include <wallet/cosmos/CosmosLikeCurrencies.hpp>
#include <wallet/cosmos/CosmosLikeConstants.hpp>
#include <wallet/cosmos/CosmosNetworks.hpp>
#include <wallet/cosmos/api_impl/CosmosLikeTransactionApi.hpp>

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
            return _http->GET(fmt::format("/blocks/{}", blockHeight), ACCEPT_HEADER)
                .json(true)
                .mapPtr<cosmos::Block>(getContext(), [] (const HttpRequest::JsonResult& response) {
                    auto result = std::make_shared<cosmos::Block>();
                    const auto& document = std::get<1>(response)->GetObject();
                    rpcs_parsers::parseBlock(document, currencies::ATOM.name, *result);
                    return result;
                });
        }

        FuturePtr<ledger::core::cosmos::Account>
        GaiaCosmosLikeBlockchainExplorer::getAccount(const std::string &account) {
            return _http->GET(fmt::format("/auth/accounts/{}", account), ACCEPT_HEADER)
                .json(true)
                .mapPtr<cosmos::Account>(getContext(), [] (const HttpRequest::JsonResult& response) {
                    auto result = std::make_shared<cosmos::Account>();
                    const auto& document = std::get<1>(response)->GetObject();
                    if (!document.HasMember("result")) {
                        throw make_exception(
                            api::ErrorCode::API_ERROR,
                            "The API response from explorer is missing the \"result\" key");
                    }
                    rpcs_parsers::parseAccount(document["result"], *result);
                    return result;
                });
        }

        FuturePtr<cosmos::Block> GaiaCosmosLikeBlockchainExplorer::getCurrentBlock() {
            return _http->GET(fmt::format("/blocks/latest"), ACCEPT_HEADER)
                .json(true)
                .map<std::shared_ptr<cosmos::Block>>(getContext(),
                [] (const HttpRequest::JsonResult& response) {
                    auto result = std::make_shared<cosmos::Block>();
                    const auto& document = std::get<1>(response)->GetObject();
                    rpcs_parsers::parseBlock(document, currencies::ATOM.name, *result);
                    return result;
                });
        }

        Future<cosmos::TransactionList> GaiaCosmosLikeBlockchainExplorer::getTransactions(
            const CosmosLikeBlockchainExplorer::TransactionFilter& filter, int page, int limit) const {
            return _http->GET(fmt::format("/txs?{}&page={}&limit={}", filter, page, limit), ACCEPT_HEADER)
                .json(true)
                .map<cosmos::TransactionList>(
                    getContext(), [](const HttpRequest::JsonResult& response) {
                        cosmos::TransactionList result;
                        const auto& document = std::get<1>(response)->GetObject();
                        if (!document.HasMember("txs") ||
                            !document["txs"].IsArray()) {
                            throw make_exception(
                                api::ErrorCode::API_ERROR,
                                "The API response from explorer is missing the \"txs\" key");
                        }
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
            return _http->GET(fmt::format("/txs/{}", hash), ACCEPT_HEADER)
                .json(true)
                .mapPtr<cosmos::Transaction>(getContext(), [] (const HttpRequest::JsonResult& response) {
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
                    blockHeightFilter,
                    filterWithAttribute(kEventTypeMessage, kAttributeKeySender, address)
                }),
                1,
                50);
            auto received_transactions = getTransactions(
                fuseFilters({
                    blockHeightFilter,
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
            return _http->GET("/blocks/latest", ACCEPT_HEADER)
                .json(true)
                .mapPtr<ledger::core::Block>(getContext(), [](const HttpRequest::JsonResult& response) {
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
                getPendingRewardsBalance(account)
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
                        if (!json.HasMember("result")) {
                            throw make_exception(
                                api::ErrorCode::API_ERROR,
                                "The API response from explorer is missing the \"result\" key");
                        }

                        const auto &del_val_entries = json.GetObject()["result"].GetArray();
                        BigInt total_amt = BigInt::ZERO;
                        for (const auto& delegation_entry : del_val_entries) {
                            total_amt = total_amt + BigInt::fromDecimal(delegation_entry.GetObject()["balance"].GetString());
                        }

                        return std::make_shared<BigInt>(total_amt);
                    });
        }
        /// Get total pending rewards
        FuturePtr<BigInt> GaiaCosmosLikeBlockchainExplorer::getPendingRewardsBalance(
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
                        if (!json.HasMember("result")) {
                            throw make_exception(
                                api::ErrorCode::API_ERROR,
                                "The API response from explorer is missing the \"result\" key");
                        }

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
                        if (!json.HasMember("result")) {
                            throw make_exception(
                                api::ErrorCode::API_ERROR,
                                "The API response from explorer is missing the \"result\" key");
                        }

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
                    getContext(), [](const HttpRequest::JsonResult& response) {
                        cosmos::ValidatorList result;
                        const auto& document = std::get<1>(response)->GetObject();
                        if (!document.HasMember("result")) {
                            throw make_exception(
                                api::ErrorCode::API_ERROR,
                                "The API response from explorer is missing the \"result\" key");
                        }
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

        Future<cosmos::Validator> GaiaCosmosLikeBlockchainExplorer::getValidatorInfo(const std::string& valOperAddress) const {
            const bool parseJsonNumbersAsStrings = true;
            return _http->GET(fmt::format("/staking/validators/{}", valOperAddress))
                .json(parseJsonNumbersAsStrings)
                .map<cosmos::Validator>(
                    getContext(), [](const HttpRequest::JsonResult& response) {
                        const auto& document = std::get<1>(response)->GetObject();
                        if (!document.HasMember("result")) {
                            throw make_exception(
                                api::ErrorCode::API_ERROR,
                                "The API response from explorer is missing the \"result\" key");
                        }
                        const auto& validatorNode = document["result"].GetObject();
                        cosmos::Validator val;
                        rpcs_parsers::parseValidatorSetEntry(validatorNode, val);
                        return val;
                    });
        }

        FuturePtr<std::vector<cosmos::Delegation>> GaiaCosmosLikeBlockchainExplorer::getDelegations(const std::string& delegatorAddr) const {
            return _http->GET(fmt::format("/staking/delegators/{}/delegations", delegatorAddr), ACCEPT_HEADER)
                .json(true)
                .mapPtr<std::vector<cosmos::Delegation>>(getContext(), [](const HttpRequest::JsonResult& response) {
                    const auto& document = std::get<1>(response)->GetObject();
                    const auto& results = document["result"].GetArray();
                    auto delegations = std::make_shared<std::vector<cosmos::Delegation>>();
                    for (auto& result : results) {
                        cosmos::Delegation delegation;
                        rpcs_parsers::parseDelegation(result, delegation);
                        delegations->push_back(delegation);
                    }
                    return delegations;
                });
        }

        FuturePtr<std::vector<cosmos::Reward>> GaiaCosmosLikeBlockchainExplorer::getPendingRewards(const std::string& delegatorAddr) const {
            return _http->GET(fmt::format("/distribution/delegators/{}/rewards", delegatorAddr), ACCEPT_HEADER)
                .json(true)
                .mapPtr<std::vector<cosmos::Reward>>(getContext(), [](const HttpRequest::JsonResult& response) {
                    const auto& document = std::get<1>(response)->GetObject();
                    const auto& results = document["result"].GetObject()[kRewards].GetArray();
                    auto rewards = std::make_shared<std::vector<cosmos::Reward>>();
                    for (auto& result : results) {
                        cosmos::Reward reward;
                        rpcs_parsers::parseReward(result, reward);
                        rewards->push_back(reward);
                    }
                    return rewards;
                });
        }

        namespace {

        // TODO: probably move most of these in a separate folder/file

        rapidjson::Value makeAmount(const api::CosmosLikeAmount& amount,
                                    rapidjson::Document::AllocatorType& allocator)
        {
            auto buffer = rapidjson::Value(rapidjson::kStringType);
            auto jsonAmount = rapidjson::Value(rapidjson::kObjectType);
            buffer.SetString(amount.amount.c_str(), allocator);
            jsonAmount.AddMember(cosmos::constants::kAmount, buffer, allocator);
            buffer.SetString(amount.denom.c_str(), allocator);
            jsonAmount.AddMember(cosmos::constants::kDenom, buffer, allocator);
            return jsonAmount;
        }

        rapidjson::Value makeAmountArray(const std::vector<api::CosmosLikeAmount>& amounts,
                                         rapidjson::Document::AllocatorType& allocator)
        {
            auto jsonAmounts = rapidjson::Value(rapidjson::kArrayType);
            for (const auto& amount : amounts) {
                auto amountObject = makeAmount(amount, allocator);
                jsonAmounts.PushBack(amountObject, allocator);
            }
            return jsonAmounts;
        }

        rapidjson::Value makeStringValue(const std::string& str,
                                         rapidjson::Document::AllocatorType& allocator)
        {
            auto json = rapidjson::Value(rapidjson::kStringType);
            json.SetString(str.c_str(), allocator);
            return json;
        }

        class BaseReq
        {
        public:
            BaseReq(std::string from,
                    std::string memo,
                    std::string chainId,
                    std::string accountNumber,
                    std::string accountSequence,
                    std::string gas,
                    std::string gasAdjustment,
                    cosmos::Fee fees,
                    bool simulate)
                : from_(std::move(from))
                , memo_(std::move(memo))
                , chainId_(std::move(chainId))
                , accountNumber_(std::move(accountNumber))
                , accountSequence_(std::move(accountSequence))
                , gas_(std::move(gas))
                , gasAdjustment_(std::move(gasAdjustment))
                , fees_(std::move(fees))
                , simulate_(simulate)
            {}

            rapidjson::Value toJson(rapidjson::Document::AllocatorType& allocator) const
            {
                auto baseReq = rapidjson::Value(rapidjson::kObjectType);
                auto buffer = rapidjson::Value(rapidjson::kStringType);

                buffer = makeStringValue(from_, allocator);
                baseReq.AddMember(cosmos::constants::kFrom, buffer, allocator);

                buffer = makeStringValue(memo_, allocator);
                baseReq.AddMember(cosmos::constants::kMemo, buffer, allocator);

                buffer = makeStringValue(chainId_, allocator);
                baseReq.AddMember(cosmos::constants::kChainId, buffer, allocator);

                buffer = makeStringValue(accountNumber_, allocator);
                baseReq.AddMember(cosmos::constants::kAccountNumber, buffer, allocator);

                buffer = makeStringValue(accountSequence_, allocator);
                baseReq.AddMember(cosmos::constants::kSequence, buffer, allocator);

                buffer = makeStringValue(gas_, allocator);
                baseReq.AddMember(cosmos::constants::kGas, buffer, allocator);

                auto fees = makeAmountArray(fees_.amount, allocator);
                baseReq.AddMember(cosmos::constants::kFees, fees, allocator);

                auto simulate = rapidjson::Value(rapidjson::kTrueType);
                baseReq.AddMember(cosmos::constants::kSimulate, simulate, allocator);

                return baseReq;
            }

        private:
            std::string from_;
            std::string memo_;
            std::string chainId_;
            std::string accountNumber_;
            std::string accountSequence_;
            std::string gas_;
            std::string gasAdjustment_;
            cosmos::Fee fees_;
            bool simulate_;
        };

        rapidjson::Value makeBaseReq(const std::shared_ptr<api::CosmosLikeTransaction>& transaction,
                                     const std::shared_ptr<api::CosmosLikeMessage>& message,
                                     rapidjson::Document::AllocatorType& allocator)
        {
            const auto tx = std::dynamic_pointer_cast<CosmosLikeTransactionApi>(transaction);
            const auto msg = std::dynamic_pointer_cast<CosmosLikeMessage>(message);
            const auto baseReq = BaseReq(
                    msg->getFromAddress(),
                    tx->getMemo(),
                    networks::getCosmosLikeNetworkParameters(tx->getCurrency().name).ChainId,
                    tx->getAccountNumber(),
                    tx->getAccountSequence(),
                    tx->getTxData().fee.gas.toString(),
                    std::string("1"),
                    tx->getTxData().fee,
                    true);

            return baseReq.toJson(allocator);
        }

        struct JsonObject
        {
            rapidjson::GenericStringRef<char> name;
            rapidjson::Value& value;

            JsonObject(const char* name, rapidjson::Value& value)
                : name(rapidjson::StringRef(name)), value(value)
            {}
        };

        template <typename... JsonObj>
        std::string makeJsonFrom(rapidjson::Document& document,
                                 JsonObj&&... jsonObject)
        {
            auto& allocator = document.GetAllocator();

            // TODO: use fold expression when C++17
            // (document.AddMember(std::forward<JsonObj>(jsonObject).name, std::forward<JsonObj>(jsonObject).value, allocator), ...);
            using _t = int[];
            (void)_t{0, (document.AddMember(std::forward<JsonObj>(jsonObject).name,
                        std::forward<JsonObj>(jsonObject).value, allocator), 0)...};

            auto buffer = rapidjson::StringBuffer();
            auto writer = rapidjson::Writer<rapidjson::StringBuffer>(buffer);
            // TODO: sortJson is defined static in CosmosLikeTransactionApi.cpp (either make it visible or move this to CosmosLikeTransactionApi)
            // sortJson(document);
            document.Accept(writer);

            return buffer.GetString();
        }

        } // namespace

        Future<BigInt>
        GaiaCosmosLikeBlockchainExplorer::genericPostRequestForSimulation(
                const std::string& endpoint,
                const std::string& transaction) const
        {
            const auto tx = std::vector<uint8_t>(std::begin(transaction), std::end(transaction));
            const auto headers =
                std::unordered_map<std::string, std::string>{ { "Content-Type", "application/json" } };
            return _http->POST(endpoint, tx, headers)
                .json(true)
                .map<BigInt>(getContext(), [](const HttpRequest::JsonResult &result) {
                        auto& json = *std::get<1>(result);
                        const auto estimatedGasLimit =
                            json.GetObject()[cosmos::constants::kGasEstimate].GetString();
                        return BigInt::fromString(estimatedGasLimit);
                    });
        }

        Future<BigInt>
        GaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimitForTransfer(
                const std::shared_ptr<api::CosmosLikeTransaction>& transaction,
                const std::shared_ptr<api::CosmosLikeMessage>& message) const
        {
            auto document = rapidjson::Document();
            document.SetObject();
            auto& allocator = document.GetAllocator();

            auto baseReq = JsonObject(cosmos::constants::kBaseReq,
                    makeBaseReq(transaction, message, allocator).Move());

            const auto unwrappedMessage = CosmosLikeMessage::unwrapMsgSend(message);

            auto amount = JsonObject(cosmos::constants::kAmount,
                    makeAmountArray(unwrappedMessage.amount, allocator).Move());

            const auto rawTransaction = makeJsonFrom(document, baseReq, amount);
            const auto endpoint = fmt::format(cosmos::constants::kGaiaTransfersEndpoint,
                    unwrappedMessage.toAddress);

            return genericPostRequestForSimulation(endpoint, rawTransaction);
        }

        Future<BigInt>
        GaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimitForRewards(
                const std::shared_ptr<api::CosmosLikeTransaction>& transaction,
                const std::shared_ptr<api::CosmosLikeMessage>& message) const
        {
            auto document = rapidjson::Document();
            document.SetObject();
            auto& allocator = document.GetAllocator();

            auto baseReq = JsonObject(cosmos::constants::kBaseReq,
                    makeBaseReq(transaction, message, allocator).Move());

            const auto unwrappedMessage = CosmosLikeMessage::unwrapMsgWithdrawDelegatorReward(message);

            const auto rawTransaction = makeJsonFrom(document, baseReq);
            const auto endpoint = fmt::format(cosmos::constants::kGaiaRewardsEndpoint,
                    unwrappedMessage.delegatorAddress);

            return genericPostRequestForSimulation(endpoint, rawTransaction);
        }

        Future<BigInt>
        GaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimitForDelegations(
                const std::shared_ptr<api::CosmosLikeTransaction>& transaction,
                const std::shared_ptr<api::CosmosLikeMessage>& message) const
        {
            auto document = rapidjson::Document();
            document.SetObject();
            auto& allocator = document.GetAllocator();

            auto baseReq = JsonObject(cosmos::constants::kBaseReq,
                    makeBaseReq(transaction, message, allocator).Move());

            const auto unwrappedMessage = CosmosLikeMessage::unwrapMsgDelegate(message);

            const auto delegatorAddress = JsonObject(cosmos::constants::kDelegatorAddress,
                    makeStringValue(unwrappedMessage.delegatorAddress, allocator).Move());
            const auto validatorAddress = JsonObject(cosmos::constants::kValidatorAddress,
                    makeStringValue(unwrappedMessage.validatorAddress, allocator).Move());
            const auto amount = JsonObject(cosmos::constants::kAmount,
                    makeAmount(unwrappedMessage.amount, allocator).Move());

            const auto rawTransaction = makeJsonFrom(document, baseReq, delegatorAddress,
                    validatorAddress, amount);
            const auto endpoint = fmt::format(cosmos::constants::kGaiaDelegationsEndpoint,
                    unwrappedMessage.delegatorAddress);

            return genericPostRequestForSimulation(endpoint, rawTransaction);
        }

        Future<BigInt>
        GaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimitForUnbounding(
                const std::shared_ptr<api::CosmosLikeTransaction>& transaction,
                const std::shared_ptr<api::CosmosLikeMessage>& message) const
        {
            auto document = rapidjson::Document();
            document.SetObject();
            auto& allocator = document.GetAllocator();

            auto baseReq = JsonObject(cosmos::constants::kBaseReq,
                    makeBaseReq(transaction, message, allocator).Move());

            const auto unwrappedMessage = CosmosLikeMessage::unwrapMsgUndelegate(message);

            const auto delegatorAddress = JsonObject(cosmos::constants::kDelegatorAddress,
                    makeStringValue(unwrappedMessage.delegatorAddress, allocator).Move());
            const auto validatorAddress = JsonObject(cosmos::constants::kValidatorAddress,
                    makeStringValue(unwrappedMessage.validatorAddress, allocator).Move());
            const auto amount = JsonObject(cosmos::constants::kAmount,
                    makeAmount(unwrappedMessage.amount, allocator).Move());

            const auto rawTransaction = makeJsonFrom(document, baseReq, delegatorAddress,
                    validatorAddress, amount);
            const auto endpoint = fmt::format(cosmos::constants::kGaiaUnbondingsEndpoint,
                    unwrappedMessage.delegatorAddress);

            return genericPostRequestForSimulation(endpoint, rawTransaction);
        }

        Future<BigInt>
        GaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimitForRedelegations(
                const std::shared_ptr<api::CosmosLikeTransaction>& transaction,
                const std::shared_ptr<api::CosmosLikeMessage>& message) const
        {
            auto document = rapidjson::Document();
            document.SetObject();
            auto& allocator = document.GetAllocator();

            auto baseReq = JsonObject(cosmos::constants::kBaseReq,
                    makeBaseReq(transaction, message, allocator).Move());

            const auto unwrappedMessage = CosmosLikeMessage::unwrapMsgBeginRedelegate(message);

            const auto delegatorAddress = JsonObject(cosmos::constants::kDelegatorAddress,
                    makeStringValue(unwrappedMessage.delegatorAddress, allocator).Move());
            const auto validatorSourceAddress = JsonObject(cosmos::constants::kValidatorSrcAddress,
                    makeStringValue(unwrappedMessage.validatorSourceAddress, allocator).Move());
            const auto validatorDestinationAddress = JsonObject(cosmos::constants::kValidatorDstAddress,
                    makeStringValue(unwrappedMessage.validatorDestinationAddress, allocator).Move());
            const auto amount = JsonObject(cosmos::constants::kAmount,
                    makeAmount(unwrappedMessage.amount, allocator).Move());

            const auto rawTransaction = makeJsonFrom(document, baseReq, delegatorAddress,
                    validatorSourceAddress, validatorDestinationAddress, amount);
            const auto endpoint = fmt::format(cosmos::constants::kGaiaRedelegationsEndpoint,
                    unwrappedMessage.delegatorAddress);

            return genericPostRequestForSimulation(endpoint, rawTransaction);
        }

        Future<BigInt>
        GaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimit(
                const std::shared_ptr<api::CosmosLikeTransaction>& transaction,
                const std::shared_ptr<api::CosmosLikeMessage>& message) const
        {
            switch (message->getMessageType()) {
                case api::CosmosLikeMsgType::MSGSEND:
                    return getEstimatedGasLimitForTransfer(transaction, message);
                case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATORREWARD:
                    return getEstimatedGasLimitForRewards(transaction, message);
                case api::CosmosLikeMsgType::MSGDELEGATE:
                    return getEstimatedGasLimitForDelegations(transaction, message);
                case api::CosmosLikeMsgType::MSGUNDELEGATE:
                    return getEstimatedGasLimitForUnbounding(transaction, message);
                case api::CosmosLikeMsgType::MSGBEGINREDELEGATE:
                    return getEstimatedGasLimitForRedelegations(transaction, message);
                default:
                    return Future<BigInt>(nullptr)
                        .map<BigInt>(getContext(), [](BigInt) {
                                return BigInt::ZERO;
                        });
            }
        }

        FuturePtr<BigInt>
        GaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimit(
                const std::shared_ptr<api::CosmosLikeTransaction>& transaction) const
        {
            const auto& messages = transaction->getMessages();
            auto estimations = std::vector<Future<BigInt>>();
            std::transform(std::begin(messages), std::end(messages),
                    std::back_inserter(estimations),
                    [&](const std::shared_ptr<api::CosmosLikeMessage>& message) {
                        return getEstimatedGasLimit(transaction, message);
                    });

            /// Because of limitations in the REST API, we cannot estimate
            /// directly the gas needed for a transaction with several messages.
            /// As a workaround, we split the transaction in several transactions
            /// containing only one message and sum the costs of all transactions.
            /// This implies that the cost of the signature is paid as many times
            /// as there are messages, instead of one. We assume that this cost is
            /// low compared to the total cost.
            return async::sequence(getContext(), estimations)
                .flatMapPtr<BigInt>(getContext(), [](const auto& estimations) {
                        const auto result = std::accumulate(std::begin(estimations),
                                std::end(estimations), BigInt::ZERO);
                        return FuturePtr<BigInt>::successful(std::make_shared<BigInt>(result));
                });
        }

        }  // namespace core
}
