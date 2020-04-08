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
            std::initializer_list<boost::string_view> filters) {
            std::string filter;
            return std::accumulate(filters.begin(), filters.end(), filter,
                                   [](std::string acc, boost::string_view val) {
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
        GaiaCosmosLikeBlockchainExplorer::getAccount(const std::string &account) const {
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

        namespace {

        template <typename T>
        void addMsgFeesTo(cosmos::Transaction& transaction, const T& node)
        {
            /// No need to check for this as this check is done in
            /// rpcs_parsers::parseTransaction and this function is
            /// only called once parseTransaction has finished.
            const auto& vNode = node[kTx].GetObject()[kValue].GetObject();

            const auto index = transaction.messages.size();
            auto msgFeesContent = cosmos::MsgFees();

            auto pubKey = std::string();
            rpcs_parsers::parseSignerPubKey(vNode, pubKey);
            const auto decoded = cereal::base64::decode(pubKey);
            const auto vPubKey = std::vector<uint8_t>(std::begin(decoded), std::end(decoded));
            msgFeesContent.payerAddress = CosmosLikeKeychain(vPubKey,
                    DerivationPath(""), currencies::ATOM).getAddress()->toBech32();

            /// The array of fees is reduced to a single CosmosLikeAmount (summing
            /// all the element of the array). The array cannot be stored directly
            /// as it has to be reduced to be stored in the database, and it would
            /// not be possible to recreate the array from the operation (with a
            /// single CosmosLikeAmount) in the database.
            const auto fees =
                std::accumulate(std::begin(transaction.fee.amount),
                                std::end(transaction.fee.amount),
                                BigInt::ZERO,
                                [](BigInt sum, const api::CosmosLikeAmount& amount) {
                                /// The fees unit must be "uatom" as cosmoshub is not
                                /// expected to take any other currencies to pay fees.
                                    assert(amount.denom == "uatom");
                                    return sum + BigInt::fromString(amount.amount);
                                }).toString();

            msgFeesContent.fees = api::CosmosLikeAmount(fees, "uatom");
            auto msgFees = cosmos::Message();
            msgFees.type =  cosmos::constants::kMsgFees;
            msgFees.content = msgFeesContent;
            const auto msgFeesLog =
                cosmos::MessageLog{static_cast<int32_t>(index), true, ""};
            transaction.logs.push_back(msgFeesLog);
            transaction.messages.push_back(msgFees);
        }

        template <typename T>
        void parseTransactionWithPosttreatment(const T& node, cosmos::Transaction& transaction)
        {
            rpcs_parsers::parseTransaction(node, transaction);
            addMsgFeesTo(transaction, node);
        }

        } // namespace

        FuturePtr<cosmos::TransactionsBulk> GaiaCosmosLikeBlockchainExplorer::getTransactions(
            const CosmosLikeBlockchainExplorer::TransactionFilter& filter, int page, int limit) const {
            return _http->GET(fmt::format("/txs?{}&page={}&limit={}", filter, page, limit), ACCEPT_HEADER)
                .json(true)
                .mapPtr<cosmos::TransactionsBulk>(
                    getContext(), [](const HttpRequest::JsonResult& response) {
                        auto result = std::make_shared<cosmos::TransactionsBulk>();

                        const auto& document = std::get<1>(response)->GetObject();
                        if (!document.HasMember("txs") ||
                            !document["txs"].IsArray()) {
                            throw make_exception(
                                api::ErrorCode::API_ERROR,
                                "The API response from explorer is missing the \"txs\" key");
                        }
                        const auto& transactions = document["txs"].GetArray();
                        for (const auto& node : transactions) {
                            auto tx = cosmos::Transaction();
                            parseTransactionWithPosttreatment(node, tx);
                            result->transactions.emplace_back(tx);
                        }

                        if (!document.HasMember(cosmos::constants::kCount) ||
                            !document[cosmos::constants::kCount].IsString() ||
                            !document.HasMember(cosmos::constants::kTotalCount) ||
                            !document[cosmos::constants::kTotalCount].IsString()) {
                            result->hasNext = false;
                        } else {
                            const auto count = std::stoi(document[cosmos::constants::kCount].GetString());
                            const auto total_count = std::stoi(document[cosmos::constants::kTotalCount].GetString());
                            result->hasNext = (count < total_count);
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
                    parseTransactionWithPosttreatment(document, *tx);
                    return tx;
                });
        }

        namespace {

        std::shared_ptr<cosmos::TransactionsBulk> concatenateBulks(
                const std::vector<std::shared_ptr<cosmos::TransactionsBulk>>& bulks)
        {
            auto result = std::make_shared<cosmos::TransactionsBulk>();
            for (const auto& bulk : bulks) {
                // Forced to copy because of flatMap API
                result->transactions.insert(result->transactions.end(),
                                            bulk->transactions.begin(),
                                            bulk->transactions.end());
                result->hasNext |= bulk->hasNext;
            }
            return result;
        }

        } // namespace

        FuturePtr<cosmos::TransactionsBulk>
        GaiaCosmosLikeBlockchainExplorer::getTransactionsForAddress(const std::string& address,
                                                                    uint32_t fromBlockHeight) const {

            auto blockHeightFilter = filterWithAttribute(kTx, kMinHeight, std::to_string(fromBlockHeight));

            // NOTE: MsgUn/Re/Delegate + MsgWithdrawDelegatorReward are all covered by 'message.sender'
            auto sent_transactions = getTransactions(
                fuseFilters({
                    blockHeightFilter,
                    filterWithAttribute(kEventTypeMessage, kAttributeKeySender, address)
                }), 1, 5);
            auto received_transactions = getTransactions(
                fuseFilters({
                    blockHeightFilter,
                    filterWithAttribute(kEventTypeTransfer, kAttributeKeyRecipient, address)
                }), 1, 5);
            std::vector<FuturePtr<cosmos::TransactionsBulk>> transaction_promises(
                {sent_transactions,
                 received_transactions});

            return async::sequence(getContext(), transaction_promises)
                .flatMapPtr<cosmos::TransactionsBulk>(getContext(), [](const auto& vector_of_bulks) {
                    return FuturePtr<cosmos::TransactionsBulk>::successful(concatenateBulks(vector_of_bulks));
                });
        }

        FuturePtr<cosmos::TransactionsBulk> GaiaCosmosLikeBlockchainExplorer::getTransactionsForAddresses(
            const std::vector<std::string>& addresses, uint32_t fromBlockHeight) const {
            std::vector<FuturePtr<cosmos::TransactionsBulk>> address_transactions;
            std::transform(addresses.begin(), addresses.end(), std::back_inserter(address_transactions),
                           [&] (const auto& address) {
                               return getTransactionsForAddress(address, fromBlockHeight);
                           });
            return async::sequence(getContext(), address_transactions)
                .flatMapPtr<cosmos::TransactionsBulk>(getContext(), [](const auto& vector_of_bulks) {
                    return FuturePtr<cosmos::TransactionsBulk>::successful(concatenateBulks(vector_of_bulks));
                });
        }

        FuturePtr<cosmos::TransactionsBulk>
        GaiaCosmosLikeBlockchainExplorer::getTransactions(
            const std::vector<std::string>& addresses,
            uint32_t fromBlockHeight,
            Option<void*> session) {

            return getTransactionsForAddresses(addresses, fromBlockHeight);
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
                            // TODO : the content of this payload actually depends on the mode of the transaction
                            // block has both check_tx and deliver_tx
                            // sync has check_tx
                            // async has nothing
                            auto& json = *std::get<1>(result);
                            if (json.HasMember("raw_log")) {
                                return json["raw_log"].GetString();
                            }
                            return "";
                        });
        }

        Future<int64_t> GaiaCosmosLikeBlockchainExplorer::getTimestamp() const {
            return Future<int64_t>::successful(
                std::chrono::duration_cast<std::chrono::seconds>(
                    std::chrono::system_clock::now().time_since_epoch())
                    .count());
        }

        // Pending statuses
        Future<cosmos::UnbondingList> GaiaCosmosLikeBlockchainExplorer::getUnbondingsByDelegator(
            const std::string &delegatorAddress) const
        {
            const auto endpoint = fmt::format(kGaiaUnbondingsEndpoint, delegatorAddress);
            std::unordered_map<std::string, std::string> headers{
                {"Content-Type", "application/json"}};
            const bool jsonParseNumbersAsString = false;

            return _http->GET(endpoint, headers)
                .json(jsonParseNumbersAsString)
                .map<cosmos::UnbondingList>(
                    getContext(),
                    [endpoint](const HttpRequest::JsonResult &result) -> cosmos::UnbondingList {
                        auto &json = *std::get<1>(result);
                        // This is just an additional safety check on the content
                        // which will get parsed
                        if (!json.HasMember("result")) {
                            throw make_exception(
                                api::ErrorCode::API_ERROR,
                                "The API response from explorer is missing the \"result\" key");
                        }
                        const auto &fullExplorerResponse = json.GetObject();

                        cosmos::UnbondingList resultingList;
                        rpcs_parsers::parseUnbondingList(fullExplorerResponse, resultingList);
                        return resultingList;
                    });
        }

        Future<cosmos::RedelegationList>
        GaiaCosmosLikeBlockchainExplorer::getRedelegationsByDelegator(
            const std::string &delegatorAddress) const
        {
            const auto endpoint = fmt::format("{}?delegator={}", kGaiaQueryRedelegationsEndpoint, delegatorAddress);
            std::unordered_map<std::string, std::string> headers{
                {"Content-Type", "application/json"}};
            const bool jsonParseNumbersAsString = false;

            return _http->GET(endpoint, headers)
                .json(jsonParseNumbersAsString)
                .map<cosmos::RedelegationList>(
                    getContext(),
                    [endpoint](const HttpRequest::JsonResult &result) -> cosmos::RedelegationList {
                        auto &json = *std::get<1>(result);
                        // This is just an additional safety check on the content
                        // which will get parsed
                        if (!json.HasMember("result")) {
                            throw make_exception(
                                api::ErrorCode::API_ERROR,
                                "The API response from explorer is missing the \"result\" key");
                        }
                        const auto &fullExplorerResponse = json.GetObject();

                        cosmos::RedelegationList resultingList;
                        rpcs_parsers::parseRedelegationList(fullExplorerResponse, resultingList);
                        return resultingList;
                    });
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
            // Chain 3 explorer calls to get all the relevant information
            const bool parseJsonNumbersAsStrings = true;
            return _http->GET(fmt::format("/staking/validators/{}", valOperAddress))
                .json(parseJsonNumbersAsStrings)
                .template flatMap<cosmos::Validator>(
                    getContext(),
                    [this](const HttpRequest::JsonResult &response) -> Future<cosmos::Validator> {
                        const auto &document = std::get<1>(response)->GetObject();
                        if (!document.HasMember("result")) {
                            throw make_exception(
                                api::ErrorCode::API_ERROR,
                                "The API response from explorer is missing the \"result\" key");
                        }
                        const auto &validatorNode = document["result"].GetObject();
                        cosmos::Validator val;
                        rpcs_parsers::parseValidatorSetEntry(validatorNode, val);
                        return Future<cosmos::Validator>::successful(val);
                    })
                .template flatMap<cosmos::Validator>(
                    getContext(),
                    [this, parseJsonNumbersAsStrings](
                        const cosmos::Validator &inputVal) -> Future<cosmos::Validator> {
                        auto retval = cosmos::Validator(inputVal);
                        return _http->GET(fmt::format(kGaiaDistInfoEndpoint, retval.operatorAddress))
                            .json(parseJsonNumbersAsStrings)
                            .template flatMap<cosmos::Validator> (
                                getContext(),
                                [retval](const HttpRequest::JsonResult &response) mutable -> Future<cosmos::Validator> {
                                    const auto &document = std::get<1>(response)->GetObject();
                                    rpcs_parsers::parseDistInfo(document, retval.distInfo);
                                    return Future<cosmos::Validator>::successful(retval);
                                });
                    })
                .template flatMap<cosmos::Validator>(
                    getContext(),
                    [this, parseJsonNumbersAsStrings](
                        const cosmos::Validator &inputVal) -> Future<cosmos::Validator> {
                        auto retval = cosmos::Validator(inputVal);
                        return _http->GET(fmt::format(kGaiaSignInfoEndpoint, retval.consensusPubkey))
                            .json(parseJsonNumbersAsStrings)
                            .template flatMap<cosmos::Validator>(
                                getContext(),
                                [retval](const HttpRequest::JsonResult &response) mutable -> Future<cosmos::Validator> {
                                    const auto &document = std::get<1>(response)->GetObject();
                                    rpcs_parsers::parseSignInfo(document, retval.signInfo);
                                    return Future<cosmos::Validator>::successful(retval);
                                });
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

        void makeAmount(const api::CosmosLikeAmount& amount,
                        rapidjson::Value& value,
                        rapidjson::Document::AllocatorType& allocator)
        {
            auto buffer = rapidjson::Value(rapidjson::kStringType);
            buffer.SetString(amount.amount.c_str(), allocator);
            value.AddMember(cosmos::constants::kAmount, buffer, allocator);
            buffer.SetString(amount.denom.c_str(), allocator);
            value.AddMember(cosmos::constants::kDenom, buffer, allocator);
        }

        void makeAmountArray(const std::vector<api::CosmosLikeAmount>& amounts,
                             rapidjson::Value& value,
                             rapidjson::Document::AllocatorType& allocator)
        {
            for (const auto& amount : amounts) {
                auto amountObject = rapidjson::Value(rapidjson::kObjectType);
                makeAmount(amount, amountObject, allocator);
                value.PushBack(amountObject, allocator);
            }
        }

        void makeStringValue(const std::string& str,
                             rapidjson::Value& value,
                             rapidjson::Document::AllocatorType& allocator)
        {
            value.SetString(str.c_str(), allocator);
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

            void toJson(rapidjson::Value& value,
                        rapidjson::Document::AllocatorType& allocator) const
            {
                auto buffer = rapidjson::Value(rapidjson::kStringType);

                makeStringValue(from_, buffer, allocator);
                value.AddMember(cosmos::constants::kFrom, buffer, allocator);

                makeStringValue(memo_, buffer, allocator);
                value.AddMember(cosmos::constants::kMemo, buffer, allocator);

                makeStringValue(chainId_, buffer, allocator);
                value.AddMember(cosmos::constants::kChainId, buffer, allocator);

                makeStringValue(accountNumber_, buffer, allocator);
                value.AddMember(cosmos::constants::kAccountNumber, buffer, allocator);

                makeStringValue(accountSequence_, buffer, allocator);
                value.AddMember(cosmos::constants::kSequence, buffer, allocator);

                makeStringValue(gas_, buffer, allocator);
                value.AddMember(cosmos::constants::kGas, buffer, allocator);

                makeStringValue(gasAdjustment_, buffer, allocator);
                value.AddMember(cosmos::constants::kGasAdjustment, buffer, allocator);

                auto fees = rapidjson::Value(rapidjson::kArrayType);
                makeAmountArray(fees_.amount, fees, allocator);
                value.AddMember(cosmos::constants::kFees, fees, allocator);

                auto simulate = rapidjson::Value(rapidjson::kTrueType);
                value.AddMember(cosmos::constants::kSimulate, simulate, allocator);
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

        void makeBaseReq(const std::shared_ptr<api::CosmosLikeTransaction>& transaction,
                         const std::shared_ptr<api::CosmosLikeMessage>& message,
                         double gasAdjustment,
                         rapidjson::Value& value,
                         rapidjson::Document::AllocatorType& allocator)
        {
            const auto tx = std::dynamic_pointer_cast<CosmosLikeTransactionApi>(transaction);
            const auto msg = std::dynamic_pointer_cast<CosmosLikeMessage>(message);
            std::string gasAdjust = std::to_string(gasAdjustment);
            // Ensure that to_string result uses '.' as separator
            std::replace(gasAdjust.begin(), gasAdjust.end(), ',', '.');
            const auto baseReq = BaseReq(
                    msg->getFromAddress(),
                    tx->getMemo(),
                    networks::getCosmosLikeNetworkParameters(tx->getCurrency().name).ChainId,
                    tx->getAccountNumber(),
                    tx->getAccountSequence(),
                    tx->getTxData().fee.gas.toString(),
                    gasAdjust,
                    tx->getTxData().fee,
                    true);

            baseReq.toJson(value, allocator);
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

        Future<BigInt> GaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimitForTransfer(
            const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
            const std::shared_ptr<api::CosmosLikeMessage> &message,
            double gasAdjustment) const
        {
            auto document = rapidjson::Document();
            document.SetObject();
            auto& allocator = document.GetAllocator();

            auto baseReqValue = rapidjson::Value(rapidjson::kObjectType);
            makeBaseReq(transaction, message, gasAdjustment, baseReqValue, allocator);
            auto baseReq = JsonObject(cosmos::constants::kBaseReq, baseReqValue);

            const auto unwrappedMessage = CosmosLikeMessage::unwrapMsgSend(message);

            auto amountValue = rapidjson::Value(rapidjson::kArrayType);
            makeAmountArray(unwrappedMessage.amount, amountValue, allocator);
            auto amount = JsonObject(cosmos::constants::kAmount, amountValue);

            const auto rawTransaction = makeJsonFrom(document, baseReq, amount);
            const auto endpoint = fmt::format(cosmos::constants::kGaiaTransfersEndpoint,
                    unwrappedMessage.toAddress);

            return genericPostRequestForSimulation(endpoint, rawTransaction);
        }

        Future<BigInt> GaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimitForRewards(
            const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
            const std::shared_ptr<api::CosmosLikeMessage> &message,
            double gasAdjustment) const
        {
            auto document = rapidjson::Document();
            document.SetObject();
            auto& allocator = document.GetAllocator();

            auto baseReqValue = rapidjson::Value(rapidjson::kObjectType);
            makeBaseReq(transaction, message, gasAdjustment, baseReqValue, allocator);
            auto baseReq = JsonObject(cosmos::constants::kBaseReq, baseReqValue);

            const auto unwrappedMessage = CosmosLikeMessage::unwrapMsgWithdrawDelegationReward(message);

            const auto rawTransaction = makeJsonFrom(document, baseReq);
            const auto endpoint = fmt::format("{}/{}", fmt::format(cosmos::constants::kGaiaRewardsEndpoint,
                    unwrappedMessage.delegatorAddress), unwrappedMessage.validatorAddress);

            return genericPostRequestForSimulation(endpoint, rawTransaction);
        }

        Future<BigInt> GaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimitForDelegations(
            const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
            const std::shared_ptr<api::CosmosLikeMessage> &message,
            double gasAdjustment) const
        {
            auto document = rapidjson::Document();
            document.SetObject();
            auto& allocator = document.GetAllocator();

            auto baseReqValue = rapidjson::Value(rapidjson::kObjectType);
            makeBaseReq(transaction, message, gasAdjustment, baseReqValue, allocator);
            auto baseReq = JsonObject(cosmos::constants::kBaseReq, baseReqValue);

            const auto unwrappedMessage = CosmosLikeMessage::unwrapMsgDelegate(message);

            auto delegatorAddressValue = rapidjson::Value(rapidjson::kStringType);
            makeStringValue(unwrappedMessage.delegatorAddress, delegatorAddressValue, allocator);
            const auto delegatorAddress = JsonObject(cosmos::constants::kDelegatorAddress, delegatorAddressValue);

            auto validatorAddressValue = rapidjson::Value(rapidjson::kStringType);
            makeStringValue(unwrappedMessage.validatorAddress, validatorAddressValue, allocator);
            const auto validatorAddress = JsonObject(cosmos::constants::kValidatorAddress, validatorAddressValue);

            auto amountValue = rapidjson::Value(rapidjson::kObjectType);
            makeAmount(unwrappedMessage.amount, amountValue, allocator);
            const auto amount = JsonObject(cosmos::constants::kAmount, amountValue);

            const auto rawTransaction = makeJsonFrom(document, baseReq, delegatorAddress,
                    validatorAddress, amount);
            const auto endpoint = fmt::format(cosmos::constants::kGaiaDelegationsEndpoint,
                    unwrappedMessage.delegatorAddress);

            return genericPostRequestForSimulation(endpoint, rawTransaction);
        }

        Future<BigInt> GaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimitForUnbounding(
            const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
            const std::shared_ptr<api::CosmosLikeMessage> &message,
            double gasAdjustment) const
        {
            auto document = rapidjson::Document();
            document.SetObject();
            auto& allocator = document.GetAllocator();

            auto baseReqValue = rapidjson::Value(rapidjson::kObjectType);
            makeBaseReq(transaction, message, gasAdjustment, baseReqValue, allocator);
            auto baseReq = JsonObject(cosmos::constants::kBaseReq, baseReqValue);

            const auto unwrappedMessage = CosmosLikeMessage::unwrapMsgUndelegate(message);

            auto delegatorAddressValue = rapidjson::Value(rapidjson::kStringType);
            makeStringValue(unwrappedMessage.delegatorAddress, delegatorAddressValue, allocator);
            const auto delegatorAddress = JsonObject(cosmos::constants::kDelegatorAddress, delegatorAddressValue);

            auto validatorAddressValue = rapidjson::Value(rapidjson::kStringType);
            makeStringValue(unwrappedMessage.validatorAddress, validatorAddressValue, allocator);
            const auto validatorAddress = JsonObject(cosmos::constants::kValidatorAddress, validatorAddressValue);

            auto amountValue = rapidjson::Value(rapidjson::kObjectType);
            makeAmount(unwrappedMessage.amount, amountValue, allocator);
            const auto amount = JsonObject(cosmos::constants::kAmount, amountValue);

            const auto rawTransaction = makeJsonFrom(document, baseReq, delegatorAddress,
                    validatorAddress, amount);
            const auto endpoint = fmt::format(cosmos::constants::kGaiaUnbondingsEndpoint,
                    unwrappedMessage.delegatorAddress);

            return genericPostRequestForSimulation(endpoint, rawTransaction);
        }

        Future<BigInt> GaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimitForRedelegations(
            const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
            const std::shared_ptr<api::CosmosLikeMessage> &message,
            double gasAdjustment) const
        {
            auto document = rapidjson::Document();
            document.SetObject();
            auto& allocator = document.GetAllocator();

            auto baseReqValue = rapidjson::Value(rapidjson::kObjectType);
            makeBaseReq(transaction, message, gasAdjustment, baseReqValue, allocator);
            auto baseReq = JsonObject(cosmos::constants::kBaseReq, baseReqValue);

            const auto unwrappedMessage = CosmosLikeMessage::unwrapMsgBeginRedelegate(message);

            auto delegatorAddressValue = rapidjson::Value(rapidjson::kStringType);
            makeStringValue(unwrappedMessage.delegatorAddress, delegatorAddressValue, allocator);
            const auto delegatorAddress = JsonObject(cosmos::constants::kDelegatorAddress, delegatorAddressValue);

            auto validatorSourceAddressValue = rapidjson::Value(rapidjson::kStringType);
            makeStringValue(unwrappedMessage.validatorSourceAddress,
                    validatorSourceAddressValue, allocator);
            const auto validatorSourceAddress =
                JsonObject(cosmos::constants::kValidatorSrcAddress,
                        validatorSourceAddressValue);

            auto validatorDestinationAddressValueAddress = rapidjson::Value(rapidjson::kStringType);
            makeStringValue(unwrappedMessage.validatorDestinationAddress,
                    validatorDestinationAddressValueAddress, allocator);
            const auto validatorDestinationAddress =
                JsonObject(cosmos::constants::kValidatorDstAddress,
                        validatorDestinationAddressValueAddress);

            auto amountValue = rapidjson::Value(rapidjson::kObjectType);
            makeAmount(unwrappedMessage.amount, amountValue, allocator);
            const auto amount = JsonObject(cosmos::constants::kAmount, amountValue);

            const auto rawTransaction = makeJsonFrom(document, baseReq, delegatorAddress,
                    validatorSourceAddress, validatorDestinationAddress, amount);
            const auto endpoint = fmt::format(cosmos::constants::kGaiaRedelegationsEndpoint,
                    unwrappedMessage.delegatorAddress);

            return genericPostRequestForSimulation(endpoint, rawTransaction);
        }

        Future<BigInt> GaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimit(
            const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
            const std::shared_ptr<api::CosmosLikeMessage> &message,
            double gasAdjustment) const
        {
            switch (message->getMessageType()) {
                case api::CosmosLikeMsgType::MSGSEND:
                    return getEstimatedGasLimitForTransfer(transaction, message, gasAdjustment);
                case api::CosmosLikeMsgType::MSGWITHDRAWDELEGATIONREWARD:
                    return getEstimatedGasLimitForRewards(transaction, message, gasAdjustment);
                case api::CosmosLikeMsgType::MSGDELEGATE:
                    return getEstimatedGasLimitForDelegations(transaction, message, gasAdjustment);
                case api::CosmosLikeMsgType::MSGUNDELEGATE:
                    return getEstimatedGasLimitForUnbounding(transaction, message, gasAdjustment);
                case api::CosmosLikeMsgType::MSGBEGINREDELEGATE:
                    return getEstimatedGasLimitForRedelegations(transaction, message, gasAdjustment);
                default:
                    return Future<BigInt>::successful(BigInt::ZERO);
            }
        }

        FuturePtr<BigInt>
        GaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimit(
            const std::shared_ptr<api::CosmosLikeTransaction>& transaction,
            double gasAdjustment) const
        {
            const auto& messages = transaction->getMessages();
            auto estimations = std::vector<Future<BigInt>>();
            std::transform(std::begin(messages), std::end(messages),
                    std::back_inserter(estimations),
                    [&](const std::shared_ptr<api::CosmosLikeMessage>& message) {
                        return getEstimatedGasLimit(transaction, message, gasAdjustment);
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
