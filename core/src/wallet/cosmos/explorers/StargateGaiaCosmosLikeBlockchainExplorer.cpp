/*
 *
 * StargateGaiaCosmosLikeBlockchainExplorer.cpp
 * ledger-core
 *
 * Created by Gerry Agbobada on 2021-01-21
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

#include <algorithm>
#include <numeric>

#include <api/Configuration.hpp>
#include <async/algorithm.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <utils/Exception.hpp>
#include <wallet/cosmos/CosmosLikeConstants.hpp>
#include <wallet/cosmos/CosmosLikeCurrencies.hpp>
#include <wallet/cosmos/CosmosNetworks.hpp>
#include <wallet/cosmos/api_impl/CosmosLikeTransactionApi.hpp>
#include <wallet/cosmos/explorers/StargateGaiaCosmosLikeBlockchainExplorer.hpp>
#include <wallet/cosmos/explorers/StargateRpcsParsers.hpp>

namespace ledger {
namespace core {

using namespace stargate_rpcs_parsers;

using MsgType = cosmos::MsgType;

static const std::unordered_map<std::string, std::string> ACCEPT_HEADER{
    {"Accept", "application/json"}};

static CosmosLikeBlockchainExplorer::TransactionFilter eventAttribute(
    const char eventType[], const char attributeKey[])
{
    return fmt::format("{}.{}", eventType, attributeKey);
}

CosmosLikeBlockchainExplorer::TransactionFilter
StargateGaiaCosmosLikeBlockchainExplorer::filterWithAttribute(
    const char eventType[], const char attributeKey[], const std::string &value)
{
    return fmt::format("{}={}", eventAttribute(eventType, attributeKey), value);
}

CosmosLikeBlockchainExplorer::TransactionFilter StargateGaiaCosmosLikeBlockchainExplorer::fuseFilters(
    std::initializer_list<boost::string_view> filters)
{
    std::string filter;
    return std::accumulate(
        filters.begin(), filters.end(), filter, [](std::string acc, boost::string_view val) {
            if (acc.empty()) {
                return std::string(val.data());
            }
            return acc + "&" + val.data();
        });
}

// Address at the end of the filter is...
static const std::vector<CosmosLikeBlockchainExplorer::TransactionFilter> GAIA_FILTER{};

StargateGaiaCosmosLikeBlockchainExplorer::StargateGaiaCosmosLikeBlockchainExplorer(
    const std::shared_ptr<api::ExecutionContext> &context,
    const std::shared_ptr<HttpClient> &http,
    const api::Currency currency,
    const std::shared_ptr<api::DynamicObject> &configuration) :
    DedicatedContext(context),
    CosmosLikeBlockchainExplorer(
        configuration, {api::Configuration::BLOCKCHAIN_EXPLORER_API_ENDPOINT}),
    _http(http),
    _currency(currency)
{
}

const std::vector<CosmosLikeBlockchainExplorer::TransactionFilter>
    &StargateGaiaCosmosLikeBlockchainExplorer::getTransactionFilters()
{
    return GAIA_FILTER;
}

FuturePtr<cosmos::Block> StargateGaiaCosmosLikeBlockchainExplorer::getBlock(uint64_t &blockHeight) const
{
    return _http->GET(fmt::format(kGrpcBlocksEndpoint, blockHeight), ACCEPT_HEADER)
        .json(true)
        .mapPtr<cosmos::Block>(getContext(), [this](const HttpRequest::JsonResult &response) {
            auto result = std::make_shared<cosmos::Block>();
            const auto &document = std::get<1>(response)->GetObject();
            parseBlock(document, _currency, *result);
            return result;
        });
}

FuturePtr<ledger::core::cosmos::Account> StargateGaiaCosmosLikeBlockchainExplorer::getAccount(
    const std::string &account) const
{
    // NOTE : those futures do not need to be chained
    // as only "account" is needed as argument to the futures.
    // First call gets the /auth/accounts information (general info)
    // Second call gets the "withdrawAddress" (where rewards go)
    const bool parseJsonNumbersAsStrings = true;
    return _http->GET(fmt::format(kGrpcAccountEndpoint, _ns, _version, account), ACCEPT_HEADER)
        .json(parseJsonNumbersAsStrings)
        .template flatMap<cosmos::Account>(
            getContext(),
            [](const HttpRequest::JsonResult &response) {
                auto result = cosmos::Account();
                const auto &document = std::get<1>(response)->GetObject();
                if (!document.HasMember(kAccount)) {
                    throw make_exception(
                        api::ErrorCode::API_ERROR,
                        fmt::format("The API response from explorer is missing the \"{}\" key", kAccount));
                }
                parseAccount(document, result);
                return Future<cosmos::Account>::successful(result);
            })
        .template flatMapPtr<cosmos::Account>(
            getContext(),
            [this, parseJsonNumbersAsStrings, account](
                const cosmos::Account &inputAcc) -> FuturePtr<cosmos::Account> {
                auto retval = cosmos::Account(inputAcc);
                return _http->GET(fmt::format(kGrpcWithdrawAddressEndpoint, _ns, _version, account))
                    .json(parseJsonNumbersAsStrings)
                    .template flatMapPtr<cosmos::Account>(
                        getContext(),
                        [retval](const HttpRequest::JsonResult &response) mutable
                        -> FuturePtr<cosmos::Account> {
                            const auto &document = std::get<1>(response)->GetObject();
                            const auto withdrawAddress = document[kWithdrawAddress].GetString();
                            retval.withdrawAddress = withdrawAddress;
                            return FuturePtr<cosmos::Account>::successful(
                                std::make_shared<cosmos::Account>(retval));
                        });
            });
}

FuturePtr<cosmos::Block> StargateGaiaCosmosLikeBlockchainExplorer::getCurrentBlock()
{
    return _http->GET(fmt::format(kGrpcLatestBlockEndpoint), ACCEPT_HEADER)
        .json(true)
        .map<std::shared_ptr<cosmos::Block>>(
            getContext(), [this](const HttpRequest::JsonResult &response) {
                auto result = std::make_shared<cosmos::Block>();
                const auto &document = std::get<1>(response)->GetObject();
                parseBlock(document, _currency, *result);
                return result;
            });
}

template <typename T>
void StargateGaiaCosmosLikeBlockchainExplorer::addMsgFeesTo(cosmos::Transaction &transaction, const T &node) const
{
    /// No need to check for this as this check is done in
    /// parseTransaction and this function is
    /// only called once parseTransaction has finished.
    const auto &vNode = node[kTx].GetObject()[kValue].GetObject();

    const auto index = transaction.messages.size();
    auto msgFeesContent = cosmos::MsgFees();

    auto pubKey = std::string();
    parseSignerPubKey(vNode, pubKey);
    const auto decoded = cereal::base64::decode(pubKey);
    const auto vPubKey = std::vector<uint8_t>(std::begin(decoded), std::end(decoded));
    msgFeesContent.payerAddress =
        CosmosLikeKeychain(vPubKey, DerivationPath(""), _currency).getAddress()->toBech32();

    /// The array of fees is reduced to a single CosmosLikeAmount (summing
    /// all the element of the array). The array cannot be stored directly
    /// as it has to be reduced to be stored in the database, and it would
    /// not be possible to recreate the array from the operation (with a
    /// single CosmosLikeAmount) in the database.
    const auto fees = std::accumulate(
                          std::begin(transaction.fee.amount),
                          std::end(transaction.fee.amount),
                          BigInt::ZERO,
                          [this](BigInt sum, const api::CosmosLikeAmount &amount) {
                              assert(amount.denom == _currency.units[0].code);
                              return sum + BigInt::fromString(amount.amount);
                          })
                          .toString();

    msgFeesContent.fees = api::CosmosLikeAmount(fees, _currency.units[0].code);
    auto msgFees = cosmos::Message();
    msgFees.type = kMsgFees;
    msgFees.content = msgFeesContent;
    const auto msgFeesLog = cosmos::MessageLog{static_cast<int32_t>(index), true, ""};
    transaction.logs.push_back(msgFeesLog);
    transaction.messages.push_back(msgFees);
}


template <typename T>
void StargateGaiaCosmosLikeBlockchainExplorer::parseTransactionWithPosttreatment(
    const T &node, cosmos::Transaction &transaction) const
{
    parseTransaction(node, transaction, _currency);
    addMsgFeesTo(transaction, node);
}


FuturePtr<cosmos::TransactionsBulk> StargateGaiaCosmosLikeBlockchainExplorer::getTransactions(
    const CosmosLikeBlockchainExplorer::TransactionFilter &filter, int page, int limit) const
{
    // NOTE : the amount of memory involved here asks for a second pass to make
    // sure that the least amount of copies is done.
    return _http->GET(fmt::format(kGrpcTransactionsWithPageLimitEnpoint, filter, page, limit), ACCEPT_HEADER)
        .json(true)
        .flatMap<cosmos::TransactionsBulk>(
            getContext(), [this](const HttpRequest::JsonResult &response)
            -> Future<cosmos::TransactionsBulk> {
                cosmos::TransactionsBulk result;
                const auto &document = std::get<1>(response)->GetObject();
                // An "empty" response is not even an array
                if (!document.HasMember(kTxArray) || !document[kTxArray].IsArray()) {
                    result.hasNext = false;
                    return Future<cosmos::TransactionsBulk>::successful(result);
                }
                const auto &transactions = document[kTxArray].GetArray();
                for (const auto &node : transactions) {
                    auto tx = cosmos::Transaction();
                    parseTransactionWithPosttreatment(node, tx);
                    result.transactions.emplace_back(tx);
                }

                if (!document.HasMember(kCount) ||
                    !document[kCount].IsString() ||
                    !document.HasMember(kTotalCount) ||
                    !document[kTotalCount].IsString()) {
                    result.hasNext = false;
                }
                else {
                    const auto count = std::stoi(document[kCount].GetString());
                    const auto total_count =
                        std::stoi(document[kTotalCount].GetString());
                    result.hasNext = (count < total_count);
                }
                return Future<cosmos::TransactionsBulk>::successful(result);
            })
        // Fill all the parsed transactions in the bulk with their
        // associated block data
        .flatMapPtr<cosmos::TransactionsBulk>(
            getContext(),
            [this](const cosmos::TransactionsBulk &inputBulk) mutable
            -> FuturePtr<cosmos::TransactionsBulk> {
                auto const &inputTxs = inputBulk.transactions;
                std::vector<FuturePtr<cosmos::Transaction>> filledTxsFutures;
                filledTxsFutures.reserve(inputTxs.size());
                std::transform(
                    inputTxs.cbegin(),
                    inputTxs.cend(),
                    std::back_inserter(filledTxsFutures),
                    [this](const cosmos::Transaction &inputTx) {
                        return this->inflateTransactionWithBlockData(inputTx);
                    });
                return async::sequence(getContext(), filledTxsFutures)
                    .flatMapPtr<cosmos::TransactionsBulk>(
                        getContext(),
                        [inputBulk = std::move(inputBulk)](
                            const std::vector<std::shared_ptr<cosmos::Transaction>>
                                &filledTxsList) mutable {
                            std::transform(
                                filledTxsList.cbegin(),
                                filledTxsList.cend(),
                                inputBulk.transactions.begin(),
                                [](const std::shared_ptr<cosmos::Transaction> filledTx) {
                                    return *filledTx;
                                });
                            return FuturePtr<cosmos::TransactionsBulk>::successful(
                                std::make_shared<cosmos::TransactionsBulk>(inputBulk));
                        });
            });
}

FuturePtr<cosmos::Transaction> StargateGaiaCosmosLikeBlockchainExplorer::inflateTransactionWithBlockData(
    const cosmos::Transaction &inputTx) const
{
    auto completeTx = cosmos::Transaction(inputTx);
    if (!completeTx.block) {
        return FuturePtr<cosmos::Transaction>::successful(
            std::make_shared<cosmos::Transaction>(std::move(completeTx)));
    }
    auto height = completeTx.block.getValue().height;
    return getBlock(height).flatMapPtr<cosmos::Transaction>(
        getContext(),
        [completeTx = std::move(completeTx)](
            const auto &blockData) mutable -> FuturePtr<cosmos::Transaction> {
            completeTx.block = *blockData;
            return FuturePtr<cosmos::Transaction>::successful(
                std::make_shared<cosmos::Transaction>(completeTx));
        });
}

FuturePtr<cosmos::Transaction> StargateGaiaCosmosLikeBlockchainExplorer::getTransactionByHash(
    const std::string &hash)
{
    return _http->GET(fmt::format(kGrpcTransactionEndpoint, hash), ACCEPT_HEADER)
        .json(true)
        .flatMap<cosmos::Transaction>(getContext(), [this](const HttpRequest::JsonResult &response) -> Future<cosmos::Transaction> {
            const auto &document = std::get<1>(response)->GetObject();
            cosmos::Transaction tx;
            parseTransactionWithPosttreatment(document, tx);
            return Future<cosmos::Transaction>::successful(tx);
        })
        .flatMapPtr<cosmos::Transaction>(
            getContext(),
            [this](const auto &inputTx) {
                return this->inflateTransactionWithBlockData(inputTx);
            });
}

namespace {

std::shared_ptr<cosmos::TransactionsBulk> concatenateBulks(
    const std::vector<std::shared_ptr<cosmos::TransactionsBulk>> &bulks)
{
    /// This has to be done because of the way the synchronizer synchronizes batches.
    /// After receiving a batch, if hasNext is set to true in the fetched bulk, it
    /// recursively asks for another batch, starting from the height of the last
    /// element contained in the bulk.
    /// This swap makes sure that, by putting at the end of the vector the bulk
    /// with hasNext set to true whose last element has the lowest block height,
    /// using the last transaction's block height in the concatenated bulk to
    /// synchronize the next batch will not skip any blocks.
    auto &cp = const_cast<std::vector<std::shared_ptr<cosmos::TransactionsBulk>> &>(bulks);
    auto last = std::begin(cp);
    for (auto it = std::begin(cp); it != std::end(cp); ++it) {
        if ((*it)->hasNext) {
            if (!(*last)->hasNext || ((*it)->transactions.back().block->height <
                                      (*last)->transactions.back().block->height)) {
                last = it;
            }
        }
    }
    std::iter_swap(--std::end(cp), last);

    auto result = std::make_shared<cosmos::TransactionsBulk>();
    for (const auto &bulk : bulks) {
        // Forced to copy because of flatMap API
        result->transactions.insert(
            result->transactions.end(), bulk->transactions.begin(), bulk->transactions.end());
        result->hasNext |= bulk->hasNext;
    }
    return result;
}

}  // namespace

FuturePtr<cosmos::TransactionsBulk> StargateGaiaCosmosLikeBlockchainExplorer::getTransactionsForAddress(
    const std::string &address, uint32_t fromBlockHeight) const
{
    // Gaia explorer currently caps at 100 on tx per page.
    const auto explorerTxBatchSize = 100;
    auto blockHeightFilter = filterWithAttribute(kTx, kMinHeight, std::to_string(fromBlockHeight));

    // NOTE: MsgUn/Re/Delegate + MsgWithdrawDelegatorReward are all covered by 'message.sender'
    auto sent_transactions = getTransactions(
        fuseFilters({blockHeightFilter,
                     filterWithAttribute(kEventTypeMessage, kAttributeKeySender, address)}),
        1,
        explorerTxBatchSize);
    auto received_transactions = getTransactions(
        fuseFilters({blockHeightFilter,
                     filterWithAttribute(kEventTypeTransfer, kAttributeKeyRecipient, address)}),
        1,
        explorerTxBatchSize);
    std::vector<FuturePtr<cosmos::TransactionsBulk>> transaction_promises(
        {sent_transactions, received_transactions});

    return async::sequence(getContext(), transaction_promises)
        .flatMapPtr<cosmos::TransactionsBulk>(getContext(), [](const auto &vector_of_bulks) {
            return FuturePtr<cosmos::TransactionsBulk>::successful(
                concatenateBulks(vector_of_bulks));
        });
}

FuturePtr<cosmos::TransactionsBulk> StargateGaiaCosmosLikeBlockchainExplorer::getTransactionsForAddresses(
    const std::vector<std::string> &addresses, uint32_t fromBlockHeight) const
{
    std::vector<FuturePtr<cosmos::TransactionsBulk>> address_transactions;
    std::transform(
        addresses.begin(),
        addresses.end(),
        std::back_inserter(address_transactions),
        [&](const auto &address) {
            return this->getTransactionsForAddress(address, fromBlockHeight);
        });
    return async::sequence(getContext(), address_transactions)
        .flatMapPtr<cosmos::TransactionsBulk>(getContext(), [](const auto &vector_of_bulks) {
            return FuturePtr<cosmos::TransactionsBulk>::successful(
                concatenateBulks(vector_of_bulks));
        });
}

FuturePtr<cosmos::TransactionsBulk> StargateGaiaCosmosLikeBlockchainExplorer::getTransactions(
    const std::vector<std::string> &addresses, uint32_t fromBlockHeight, Option<void *> session)
{
    return getTransactionsForAddresses(addresses, fromBlockHeight);
}

Future<void *> StargateGaiaCosmosLikeBlockchainExplorer::startSession()
{
    return Future<void *>::successful(new std::string("", 0));
}

Future<Unit> StargateGaiaCosmosLikeBlockchainExplorer::killSession(void *session)
{
    return Future<Unit>::successful(unit);
}

FuturePtr<ledger::core::Block> StargateGaiaCosmosLikeBlockchainExplorer::getCurrentBlock() const
{
    return _http->GET(kGrpcLatestBlockEndpoint, ACCEPT_HEADER)
        .json(true)
        .mapPtr<ledger::core::Block>(getContext(), [this](const HttpRequest::JsonResult &response) {
            const auto &document = std::get<1>(response)->GetObject();
            auto block = std::make_shared<cosmos::Block>();
            parseBlock(document, _currency, *block);
            return std::make_shared<ledger::core::Block>(*block);
        });
}

Future<Bytes> StargateGaiaCosmosLikeBlockchainExplorer::getRawTransaction(const String &transactionHash)
{
    throw(Exception(
        api::ErrorCode::IMPLEMENTATION_IS_MISSING, "Get Raw Transaction is unimplemented"));
}

FuturePtr<cosmos::Transaction> StargateGaiaCosmosLikeBlockchainExplorer::getTransactionByHash(
    const String &transactionHash) const
{
    return getTransactionByHash(transactionHash);
}

Future<String> StargateGaiaCosmosLikeBlockchainExplorer::pushTransaction(
    const std::vector<uint8_t> &transaction)
{
    return _http->POST("/txs", transaction, ACCEPT_HEADER)
        .json()
        .template map<String>(
            _executionContext, [](const HttpRequest::JsonResult &result) -> String {
                // The content of this payload actually depends on the mode of the transaction
                // block has both check_tx and deliver_tx
                // sync has check_tx
                // async has nothing
                auto &json = *std::get<1>(result);
                rapidjson::StringBuffer buffer;
                rapidjson::Writer<rapidjson::StringBuffer> writer(buffer);
                json.Accept(writer);
                return buffer.GetString();
            });
}

Future<int64_t> StargateGaiaCosmosLikeBlockchainExplorer::getTimestamp() const
{
    return Future<int64_t>::successful(std::chrono::duration_cast<std::chrono::seconds>(
                                           std::chrono::system_clock::now().time_since_epoch())
                                           .count());
}

// Pending statuses
Future<cosmos::UnbondingList> StargateGaiaCosmosLikeBlockchainExplorer::getUnbondingsByDelegator(
    const std::string &delegatorAddress) const
{
    const auto endpoint = fmt::format(kGrpcUnbondingsEndpoint, _ns, _version, delegatorAddress);
    const bool jsonParseNumbersAsString = false;
    const auto ignoreFailStatusCode = true;

    return _http->GET(endpoint, ACCEPT_HEADER)
        .json(jsonParseNumbersAsString, ignoreFailStatusCode)
        .map<cosmos::UnbondingList>(
            getContext(),
            [endpoint](const HttpRequest::JsonResult &result) -> cosmos::UnbondingList {
                if (std::get<0>(result)->getStatusCode() >= 300) {
                      return cosmos::UnbondingList();
                }
                auto &json = *std::get<1>(result);
                const auto &fullExplorerResponse = json.GetObject();

                cosmos::UnbondingList resultingList;
                parseUnbondingList(fullExplorerResponse, resultingList);
                return resultingList;
            });
}

Future<cosmos::RedelegationList> StargateGaiaCosmosLikeBlockchainExplorer::getRedelegationsByDelegator(
    const std::string &delegatorAddress) const
{
    const auto endpoint =
        fmt::format(kGrpcRedelegationsEndpoint, _ns, _version, delegatorAddress);
    const bool jsonParseNumbersAsString = false;
    const auto ignoreFailStatusCode = true;

    return _http->GET(endpoint, ACCEPT_HEADER)
        .json(jsonParseNumbersAsString, ignoreFailStatusCode)
        .map<cosmos::RedelegationList>(
            getContext(),
            [endpoint](const HttpRequest::JsonResult &result) -> cosmos::RedelegationList {
                if (std::get<0>(result)->getStatusCode() >= 300) {
                      return cosmos::RedelegationList();
                }
                auto &json = *std::get<1>(result);
                const auto &fullExplorerResponse = json.GetObject();

                cosmos::RedelegationList resultingList;
                parseRedelegationList(fullExplorerResponse, resultingList);
                return resultingList;
            });
}

// Balances
/// Get Total Balance
FuturePtr<BigInt> StargateGaiaCosmosLikeBlockchainExplorer::getTotalBalance(
    const std::string &account) const
{
    std::vector<FuturePtr<BigInt>> balance_promises({getSpendableBalance(account),
                                                     getDelegatedBalance(account),
                                                     getUnbondingBalance(account),
                                                     getPendingRewardsBalance(account)});

    return async::sequence(getContext(), balance_promises)
        .flatMap<std::shared_ptr<BigInt>>(getContext(), [](auto &vector_of_balances) {
            BigInt result;
            for (const auto &balance : vector_of_balances) {
                result = result + *balance;
            }
            return FuturePtr<BigInt>::successful(std::make_shared<BigInt>(result));
        });
}
/// Get Total Balance
FuturePtr<BigInt> StargateGaiaCosmosLikeBlockchainExplorer::getTotalBalanceWithoutPendingRewards(
    const std::string &account) const
{
    std::vector<FuturePtr<BigInt>> balance_promises(
        {getSpendableBalance(account), getDelegatedBalance(account), getUnbondingBalance(account)});

    return async::sequence(getContext(), balance_promises)
        .flatMap<std::shared_ptr<BigInt>>(getContext(), [](auto &vector_of_balances) {
            BigInt result;
            for (const auto &balance : vector_of_balances) {
                result = result + *balance;
            }
            return FuturePtr<BigInt>::successful(std::make_shared<BigInt>(result));
        });
}
/// Get total balance in delegation
FuturePtr<BigInt> StargateGaiaCosmosLikeBlockchainExplorer::getDelegatedBalance(
    const std::string &account) const
{
    const auto endpoint = fmt::format(cosmos::constants::kGrpcDelegationsEndpoint, _ns, _version, account);

    const bool jsonParseNumbersAsString = true;
    const auto ignoreFailStatusCode = true;

    return _http->GET(endpoint, ACCEPT_HEADER)
        .json(jsonParseNumbersAsString, ignoreFailStatusCode)
        .mapPtr<BigInt>(
            getContext(),
            [this, endpoint](const HttpRequest::JsonResult &result) -> std::shared_ptr<BigInt> {
                // Catching http errors because Cosmos code returns 500 when there is no delegation.
                if (std::get<0>(result)->getStatusCode() >= 300) {
                return std::make_shared<BigInt>(BigInt::ZERO);
                }
                auto &json = *std::get<1>(result);
                if (!json.HasMember(kDelegationResponses)) {
                    throw make_exception(
                        api::ErrorCode::API_ERROR,
                        fmt::format("The API response from explorer is missing the \"{}\" key", kDelegationResponses));
                }

                // Handle null
                if (!json.GetObject()[kDelegationResponses].IsArray()) {
                    return std::make_shared<BigInt>(BigInt::ZERO);
                }
                const auto &del_val_entries = json.GetObject()[kDelegationResponses].GetArray();
                BigInt total_amt = BigInt::ZERO;
                for (const auto &delegation_entry : del_val_entries) {
                  auto balanceObject =
                      delegation_entry.GetObject()[kBalance].GetObject();
                  if (balanceObject[kDenom].GetString() ==
                      _currency.units[0].code) {
                    total_amt =
                        total_amt +
                        BigInt::fromDecimal(balanceObject[kAmount].GetString());
                  }
                }

                return std::make_shared<BigInt>(total_amt);
            });
}

/// Get total pending rewards
FuturePtr<BigInt> StargateGaiaCosmosLikeBlockchainExplorer::getPendingRewardsBalance(
    const std::string &account) const
{
    const auto endpoint = fmt::format(cosmos::constants::kGrpcRewardsEndpoint, _ns, _version, account);

    const bool jsonParseNumbersAsString = true;

    return _http->GET(endpoint, ACCEPT_HEADER)
        .json(jsonParseNumbersAsString)
        .mapPtr<BigInt>(
            getContext(),
            [this, endpoint](const HttpRequest::JsonResult &result) -> std::shared_ptr<BigInt> {
                auto &json = *std::get<1>(result);

                if (!json.IsObject() || !json.GetObject().HasMember(kTotal) ||
                    !json.GetObject()[kTotal].IsArray()) {
                    throw make_exception(
                        api::ErrorCode::HTTP_ERROR,
                        fmt::format("Failed to get total for {}", endpoint));
                }

                if (!json.GetObject()[kTotal].IsArray() ||
                    json.GetObject()[kTotal].GetArray().Size() <= 0) {
                    return std::make_shared<BigInt>(BigInt::ZERO);
                }

                for (auto& amount: json.GetObject()[kTotal].GetArray()) {
                  if (amount.GetObject()[kDenom].GetString() ==
                      _currency.units[0].code) {
                    std::string floating_amount =
                        amount.GetObject()[kAmount].GetString();
                    floating_amount.erase(floating_amount.find('.'),
                                          std::string::npos);
                    return std::make_shared<BigInt>(floating_amount);
                  }
                }
                return std::make_shared<BigInt>(BigInt::ZERO);
            });
}

/// Get total unbonding balance
FuturePtr<BigInt> StargateGaiaCosmosLikeBlockchainExplorer::getUnbondingBalance(
    const std::string &account) const
{
    const auto endpoint = fmt::format(cosmos::constants::kGrpcUnbondingsEndpoint, _ns, _version, account);

    const bool jsonParseNumbersAsString = true;

    return _http->GET(endpoint, ACCEPT_HEADER)
        .json(jsonParseNumbersAsString)
        .mapPtr<BigInt>(
            getContext(),
            [this, endpoint](const HttpRequest::JsonResult &result) -> std::shared_ptr<BigInt> {
                auto &json = *std::get<1>(result);
                if (!json.HasMember(kUnbondingResponses)) {
                    return std::make_shared<BigInt>(BigInt::ZERO);
                }

                // Handle null
                if (!json.GetObject()[kUnbondingResponses].IsArray()) {
                    return std::make_shared<BigInt>(BigInt::ZERO);
                }
                const auto &del_entries = json.GetObject()[kUnbondingResponses].GetArray();
                BigInt total_amt = BigInt::ZERO;
                for (const auto &del_val_entries : del_entries) {
                    const auto &entries = del_val_entries.GetObject()[kEntries].GetArray();
                    for (const auto &unbondingEntry : entries) {
                      total_amt =
                          total_amt + BigInt::fromDecimal(
                                          unbondingEntry[kBalance].GetString());
                    }
                }

                return std::make_shared<BigInt>(total_amt);
            });
}
/// Get total available (spendable) balance
FuturePtr<BigInt> StargateGaiaCosmosLikeBlockchainExplorer::getSpendableBalance(
    const std::string &account) const
{
    const auto endpoint = fmt::format(cosmos::constants::kGrpcBalancesEndpoint, _ns, _version, account, _currency.units[0].code);

    const bool jsonParseNumbersAsString = true;

    return _http->GET(endpoint, ACCEPT_HEADER)
        .json(jsonParseNumbersAsString)
        .mapPtr<BigInt>(
            getContext(),
            [endpoint](const HttpRequest::JsonResult &result) -> std::shared_ptr<BigInt> {
                auto &json = *std::get<1>(result);
                if (!json.HasMember(kBalance)) {
                    return std::make_shared<BigInt>(BigInt::ZERO);
                }

                const auto &balance = json.GetObject()[kBalance].GetObject();
                BigInt total_amt = BigInt::fromDecimal(balance[kAmount].GetString());
                return std::make_shared<BigInt>(total_amt);
            });
}

// Validators
Future<cosmos::ValidatorList> StargateGaiaCosmosLikeBlockchainExplorer::getActiveValidatorSet() const
{
    const bool parseJsonNumbersAsStrings = true;
    auto basicValidatorList =
        _http->GET(fmt::format("/{}/staking/{}/validators?status=BOND_STATUS_BONDED&pagination.limit=130", _ns, _version))
            .json(parseJsonNumbersAsStrings)
            .map<cosmos::ValidatorList>(getContext(), [](const HttpRequest::JsonResult &response) {
                cosmos::ValidatorList result;
                const auto &document = std::get<1>(response)->GetObject();
                if (!document.HasMember(kValidators)) {
                    throw make_exception(
                        api::ErrorCode::API_ERROR,
                        "The API response from explorer is missing the {} key",
                        kValidators);
                }
                // Handle null
                if (!document[kValidators].IsArray()) {
                    return result;
                }
                const auto &validators = document[kValidators].GetArray();
                for (const auto &node : validators) {
                    cosmos::Validator val;
                    parseValidatorSetEntry(node.GetObject(), val);
                    result.emplace_back(std::move(val));
                }
                return result;
            });

    return basicValidatorList;
}

Future<cosmos::Validator> StargateGaiaCosmosLikeBlockchainExplorer::getValidatorInfo(
    const std::string &valOperAddress) const
{
    // Chain 3 explorer calls to get all the relevant information
    const bool parseJsonNumbersAsStrings = true;
    return _http->GET(fmt::format(kGrpcValidatorInfoEndpoint, _ns, _version, valOperAddress))
        .json(parseJsonNumbersAsStrings)
        .template flatMap<cosmos::Validator>(
            getContext(),
            [this](const HttpRequest::JsonResult &response) -> Future<cosmos::Validator> {
                const auto &document = std::get<1>(response)->GetObject();
                if (!document.HasMember(kValidator)) {
                    throw make_exception(
                        api::ErrorCode::API_ERROR,
                        "The API response from explorer is missing the {} key",
                        kValidator);
                }
                const auto &validatorNode = document[kValidator].GetObject();
                cosmos::Validator val;
                // Stargate explorer does not fill distInfo as it is too costly in HTTP calls
                val.distInfo.selfBondRewards = "n/a";
                val.distInfo.validatorCommission = "n/a";
                parseValidatorSetEntry(validatorNode, val);
                return Future<cosmos::Validator>::successful(val);
            })
        .template flatMap<cosmos::Validator>(
            getContext(),
            [this, parseJsonNumbersAsStrings](
                const cosmos::Validator &inputVal) -> Future<cosmos::Validator> {
                auto retval = cosmos::Validator(inputVal);
                auto consAddr = CosmosLikeExtendedPublicKey::fromBech32(
                                    _currency, retval.consensusPubkey, std::string(""))
                                    ->derive("")
                                    ->toBech32();
                return _http
                    ->GET(fmt::format(kGrpcSignInfoEndpoint, _ns, _version,
                                      consAddr))
                    .json(parseJsonNumbersAsStrings)
                    .template flatMap<cosmos::Validator>(
                        getContext(),
                        [retval](
                            const HttpRequest::JsonResult &response) mutable
                        -> Future<cosmos::Validator> {
                          const auto &document =
                              std::get<1>(response)->GetObject();
                          if (!document.HasMember(kValSignInfos)) {
                            return Future<cosmos::Validator>::successful(
                                retval);
                          }
                          parseSignInfo(document, retval.signInfo);
                          return Future<cosmos::Validator>::successful(retval);
                        });
            });
}

FuturePtr<std::vector<cosmos::Delegation>> StargateGaiaCosmosLikeBlockchainExplorer::getDelegations(
    const std::string &delegatorAddr) const
{
    const auto parseNumbersAsStrings = true;
    const auto ignoreFailStatusCode = true;
    return _http
        ->GET(fmt::format(kGrpcDelegationsEndpoint, _ns, _version, delegatorAddr), ACCEPT_HEADER)
        .json(parseNumbersAsStrings, ignoreFailStatusCode)
        .mapPtr<std::vector<cosmos::Delegation>>(
            getContext(), [this](const HttpRequest::JsonResult &response) {
                // Catching http errors because Cosmos code returns 500 when there is no delegation.
                if (std::get<0>(response)->getStatusCode() >= 300) {
                     return std::make_shared<std::vector<cosmos::Delegation>>();
                }
                const auto &document = std::get<1>(response)->GetObject();
                auto delegations = std::make_shared<std::vector<cosmos::Delegation>>();
                // Handle null
                if (document.HasMember(kGrpcErrorMessage)
                    || !document.HasMember(kDelegationResponses)
                    || !document[kDelegationResponses].IsArray()) {
                    return delegations;
                }
                const auto &results = document[kDelegationResponses].GetArray();
                for (auto &result : results) {
                    cosmos::Delegation delegation;
                    parseDelegation(result, delegation, _currency);
                    delegations->push_back(delegation);
                }
                return delegations;
            });
}

FuturePtr<std::vector<cosmos::Reward>> StargateGaiaCosmosLikeBlockchainExplorer::getPendingRewards(
    const std::string &delegatorAddr) const
{
    const auto parseNumbersAsStrings = true;
    const auto ignoreFailStatusCode = true;
    return _http
        ->GET(fmt::format(kGrpcRewardsEndpoint, _ns, _version, delegatorAddr), ACCEPT_HEADER)
        .json(parseNumbersAsStrings, ignoreFailStatusCode)
        .mapPtr<std::vector<cosmos::Reward>>(
            getContext(), [this](const HttpRequest::JsonResult &response) {
                // Catching http errors because Cosmos code returns 500 when there is no delegation.
                if (std::get<0>(response)->getStatusCode() >= 300) {
                     return std::make_shared<std::vector<cosmos::Reward>>();
                }
                const auto &document = std::get<1>(response)->GetObject();
                auto rewards = std::make_shared<std::vector<cosmos::Reward>>();
                // Handle null
                if (!document[kRewards].IsArray()) {
                    return rewards;
                }
                const auto &results = document[kRewards].GetArray();
                for (auto &result : results) {
                    cosmos::Reward reward;
                    parseReward(result, reward, _currency);
                    rewards->push_back(reward);
                }
                return rewards;
            });
}

namespace {

// JSON helpers to communicate with the explorer

void makeAmount(
    const api::CosmosLikeAmount &amount,
    rapidjson::Value &value,
    rapidjson::Document::AllocatorType &allocator)
{
    auto buffer = rapidjson::Value(rapidjson::kStringType);
    buffer.SetString(amount.amount.c_str(), allocator);
    value.AddMember(cosmos::constants::kAmount, buffer, allocator);
    buffer.SetString(amount.denom.c_str(), allocator);
    value.AddMember(cosmos::constants::kDenom, buffer, allocator);
}

void makeAmountArray(
    const std::vector<api::CosmosLikeAmount> &amounts,
    rapidjson::Value &value,
    rapidjson::Document::AllocatorType &allocator)
{
    for (const auto &amount : amounts) {
        auto amountObject = rapidjson::Value(rapidjson::kObjectType);
        makeAmount(amount, amountObject, allocator);
        value.PushBack(amountObject, allocator);
    }
}

void makeStringValue(
    const std::string &str, rapidjson::Value &value, rapidjson::Document::AllocatorType &allocator)
{
    value.SetString(str.c_str(), allocator);
}

class BaseReq {
   public:
    BaseReq(
        std::string from,
        std::string memo,
        std::string chainId,
        std::string accountNumber,
        std::string accountSequence,
        std::string gas,
        std::string gasAdjustment,
        cosmos::Fee fees,
        bool simulate) :
        from_(std::move(from)),
        memo_(std::move(memo)),
        chainId_(std::move(chainId)),
        accountNumber_(std::move(accountNumber)),
        accountSequence_(std::move(accountSequence)),
        gas_(std::move(gas)),
        gasAdjustment_(std::move(gasAdjustment)),
        fees_(std::move(fees)),
        simulate_(simulate)
    {
    }

    void toJson(rapidjson::Value &value, rapidjson::Document::AllocatorType &allocator) const
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

void makeBaseReq(
    const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
    const std::shared_ptr<api::CosmosLikeMessage> &message,
    std::string gasAdjust,
    rapidjson::Value &value,
    rapidjson::Document::AllocatorType &allocator)
{
    const auto tx = std::dynamic_pointer_cast<CosmosLikeTransactionApi>(transaction);
    const auto msg = std::dynamic_pointer_cast<CosmosLikeMessage>(message);
    // Ensure that to_string result uses '.' as separator
    std::replace(gasAdjust.begin(), gasAdjust.end(), ',', '.');
    const auto baseReq = BaseReq(
        msg->getFromAddress(),
        tx->getMemo(),
        tx->getCurrency().cosmosLikeNetworkParameters.value().ChainId,
        tx->getAccountNumber(),
        tx->getAccountSequence(),
        tx->getTxData().fee.gas.toString(),
        gasAdjust,
        tx->getTxData().fee,
        true);

    baseReq.toJson(value, allocator);
}

struct JsonObject {
    rapidjson::GenericStringRef<char> name;
    rapidjson::Value &value;

    JsonObject(const char *name, rapidjson::Value &value) :
        name(rapidjson::StringRef(name)),
        value(value)
    {
    }
};

template <typename... JsonObj>
std::string makeJsonFrom(rapidjson::Document &document, JsonObj &&... jsonObject)
{
    auto &allocator = document.GetAllocator();

    // TODO: use fold expression when C++17
    // (document.AddMember(std::forward<JsonObj>(jsonObject).name,
    // std::forward<JsonObj>(jsonObject).value, allocator), ...);
    using _t = int[];
    (void)_t{0,
             (document.AddMember(
                  std::forward<JsonObj>(jsonObject).name,
                  std::forward<JsonObj>(jsonObject).value,
                  allocator),
              0)...};

    auto buffer = rapidjson::StringBuffer();
    auto writer = rapidjson::Writer<rapidjson::StringBuffer>(buffer);
    // The Json will start unsorted here.
    // sortJson is defined static in CosmosLikeTransactionApi.cpp (either make it visible or move
    // this to CosmosLikeTransactionApi) sortJson(document);
    document.Accept(writer);

    return buffer.GetString();
}

}  // namespace

Future<BigInt> StargateGaiaCosmosLikeBlockchainExplorer::genericPostRequestForSimulation(
    const std::string &endpoint, const std::string &transaction) const
{
    const auto tx = std::vector<uint8_t>(std::begin(transaction), std::end(transaction));
    const auto headers =
        std::unordered_map<std::string, std::string>{{"Content-Type", "application/json"}};
    return _http->POST(endpoint, tx, headers)
        .json(true, true)
        .map<BigInt>(getContext(), [](const HttpRequest::JsonResult &result) {
          auto &json = *std::get<1>(result);
          const auto statusCode = std::get<0>(result)->getStatusCode();
          if (statusCode < 200 || statusCode >= 400) {
            return BigInt::ZERO;
          }
          const auto estimatedGasLimit =
              json.GetObject()[cosmos::constants::kGasEstimate].GetString();
          return BigInt::fromString(estimatedGasLimit);
        });
}

Future<BigInt> StargateGaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimitForTransfer(
    const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
    const std::shared_ptr<api::CosmosLikeMessage> &message,
    const std::string gasAdjustment) const
{
    auto document = rapidjson::Document();
    document.SetObject();
    auto &allocator = document.GetAllocator();

    auto baseReqValue = rapidjson::Value(rapidjson::kObjectType);
    makeBaseReq(transaction, message, gasAdjustment, baseReqValue, allocator);
    auto baseReq = JsonObject(cosmos::constants::kBaseReq, baseReqValue);

    const auto unwrappedMessage = CosmosLikeMessage::unwrapMsgSend(message);

    auto amountValue = rapidjson::Value(rapidjson::kArrayType);
    makeAmountArray(unwrappedMessage.amount, amountValue, allocator);
    auto amount = JsonObject(cosmos::constants::kAmount, amountValue);

    const auto rawTransaction = makeJsonFrom(document, baseReq, amount);
    const auto endpoint =
        fmt::format(cosmos::constants::kGaiaTransfersEndpoint, unwrappedMessage.toAddress);

    return genericPostRequestForSimulation(endpoint, rawTransaction);
}

Future<BigInt> StargateGaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimitForRewards(
    const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
    const std::shared_ptr<api::CosmosLikeMessage> &message,
    const std::string gasAdjustment) const
{
    auto document = rapidjson::Document();
    document.SetObject();
    auto &allocator = document.GetAllocator();

    auto baseReqValue = rapidjson::Value(rapidjson::kObjectType);
    makeBaseReq(transaction, message, gasAdjustment, baseReqValue, allocator);
    auto baseReq = JsonObject(cosmos::constants::kBaseReq, baseReqValue);

    const auto unwrappedMessage = CosmosLikeMessage::unwrapMsgWithdrawDelegationReward(message);

    const auto rawTransaction = makeJsonFrom(document, baseReq);
    const auto endpoint = fmt::format(
        "{}/{}",
        fmt::format(cosmos::constants::kGaiaRewardsEndpoint, unwrappedMessage.delegatorAddress),
        unwrappedMessage.validatorAddress);

    return genericPostRequestForSimulation(endpoint, rawTransaction);
}

Future<BigInt> StargateGaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimitForDelegations(
    const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
    const std::shared_ptr<api::CosmosLikeMessage> &message,
    const std::string gasAdjustment) const
{
    auto document = rapidjson::Document();
    document.SetObject();
    auto &allocator = document.GetAllocator();

    auto baseReqValue = rapidjson::Value(rapidjson::kObjectType);
    makeBaseReq(transaction, message, gasAdjustment, baseReqValue, allocator);
    auto baseReq = JsonObject(cosmos::constants::kBaseReq, baseReqValue);

    const auto unwrappedMessage = CosmosLikeMessage::unwrapMsgDelegate(message);

    auto delegatorAddressValue = rapidjson::Value(rapidjson::kStringType);
    makeStringValue(unwrappedMessage.delegatorAddress, delegatorAddressValue, allocator);
    const auto delegatorAddress =
        JsonObject(cosmos::constants::kDelegatorAddress, delegatorAddressValue);

    auto validatorAddressValue = rapidjson::Value(rapidjson::kStringType);
    makeStringValue(unwrappedMessage.validatorAddress, validatorAddressValue, allocator);
    const auto validatorAddress =
        JsonObject(cosmos::constants::kValidatorAddress, validatorAddressValue);

    auto amountValue = rapidjson::Value(rapidjson::kObjectType);
    makeAmount(unwrappedMessage.amount, amountValue, allocator);
    const auto amount = JsonObject(cosmos::constants::kAmount, amountValue);

    const auto rawTransaction =
        makeJsonFrom(document, baseReq, delegatorAddress, validatorAddress, amount);
    const auto endpoint =
        fmt::format(cosmos::constants::kGaiaDelegationsEndpoint, unwrappedMessage.delegatorAddress);

    return genericPostRequestForSimulation(endpoint, rawTransaction);
}

Future<BigInt> StargateGaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimitForUnbounding(
    const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
    const std::shared_ptr<api::CosmosLikeMessage> &message,
    const std::string gasAdjustment) const
{
    auto document = rapidjson::Document();
    document.SetObject();
    auto &allocator = document.GetAllocator();

    auto baseReqValue = rapidjson::Value(rapidjson::kObjectType);
    makeBaseReq(transaction, message, gasAdjustment, baseReqValue, allocator);
    auto baseReq = JsonObject(cosmos::constants::kBaseReq, baseReqValue);

    const auto unwrappedMessage = CosmosLikeMessage::unwrapMsgUndelegate(message);

    auto delegatorAddressValue = rapidjson::Value(rapidjson::kStringType);
    makeStringValue(unwrappedMessage.delegatorAddress, delegatorAddressValue, allocator);
    const auto delegatorAddress =
        JsonObject(cosmos::constants::kDelegatorAddress, delegatorAddressValue);

    auto validatorAddressValue = rapidjson::Value(rapidjson::kStringType);
    makeStringValue(unwrappedMessage.validatorAddress, validatorAddressValue, allocator);
    const auto validatorAddress =
        JsonObject(cosmos::constants::kValidatorAddress, validatorAddressValue);

    auto amountValue = rapidjson::Value(rapidjson::kObjectType);
    makeAmount(unwrappedMessage.amount, amountValue, allocator);
    const auto amount = JsonObject(cosmos::constants::kAmount, amountValue);

    const auto rawTransaction =
        makeJsonFrom(document, baseReq, delegatorAddress, validatorAddress, amount);
    const auto endpoint =
        fmt::format(cosmos::constants::kGaiaUnbondingsEndpoint, unwrappedMessage.delegatorAddress);

    return genericPostRequestForSimulation(endpoint, rawTransaction);
}

Future<BigInt> StargateGaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimitForRedelegations(
    const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
    const std::shared_ptr<api::CosmosLikeMessage> &message,
    const std::string gasAdjustment) const
{
    auto document = rapidjson::Document();
    document.SetObject();
    auto &allocator = document.GetAllocator();

    auto baseReqValue = rapidjson::Value(rapidjson::kObjectType);
    makeBaseReq(transaction, message, gasAdjustment, baseReqValue, allocator);
    auto baseReq = JsonObject(cosmos::constants::kBaseReq, baseReqValue);

    const auto unwrappedMessage = CosmosLikeMessage::unwrapMsgBeginRedelegate(message);

    auto delegatorAddressValue = rapidjson::Value(rapidjson::kStringType);
    makeStringValue(unwrappedMessage.delegatorAddress, delegatorAddressValue, allocator);
    const auto delegatorAddress =
        JsonObject(cosmos::constants::kDelegatorAddress, delegatorAddressValue);

    auto validatorSourceAddressValue = rapidjson::Value(rapidjson::kStringType);
    makeStringValue(
        unwrappedMessage.validatorSourceAddress, validatorSourceAddressValue, allocator);
    const auto validatorSourceAddress =
        JsonObject(cosmos::constants::kValidatorSrcAddress, validatorSourceAddressValue);

    auto validatorDestinationAddressValueAddress = rapidjson::Value(rapidjson::kStringType);
    makeStringValue(
        unwrappedMessage.validatorDestinationAddress,
        validatorDestinationAddressValueAddress,
        allocator);
    const auto validatorDestinationAddress = JsonObject(
        cosmos::constants::kValidatorDstAddress, validatorDestinationAddressValueAddress);

    auto amountValue = rapidjson::Value(rapidjson::kObjectType);
    makeAmount(unwrappedMessage.amount, amountValue, allocator);
    const auto amount = JsonObject(cosmos::constants::kAmount, amountValue);

    const auto rawTransaction = makeJsonFrom(
        document,
        baseReq,
        delegatorAddress,
        validatorSourceAddress,
        validatorDestinationAddress,
        amount);
    const auto endpoint = fmt::format(
        cosmos::constants::kGaiaRedelegationsEndpoint, unwrappedMessage.delegatorAddress);

    return genericPostRequestForSimulation(endpoint, rawTransaction);
}

Future<BigInt> StargateGaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimit(
    const std::shared_ptr<api::CosmosLikeTransaction> &transaction,
    const std::shared_ptr<api::CosmosLikeMessage> &message,
    const std::string gasAdjustment) const
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

FuturePtr<BigInt> StargateGaiaCosmosLikeBlockchainExplorer::getEstimatedGasLimit(
    const std::shared_ptr<api::CosmosLikeTransaction> &transaction, const std::string gasAdjustment) const
{
    const auto &messages = transaction->getMessages();
    auto estimations = std::vector<Future<BigInt>>();
    std::transform(
        std::begin(messages),
        std::end(messages),
        std::back_inserter(estimations),
        [&](const std::shared_ptr<api::CosmosLikeMessage> &message) {
            return getEstimatedGasLimit(transaction, message, gasAdjustment);
        });

    /// Because of limitations in the REST API, we cannot estimate
    /// directly the gas needed for a transaction with several messages.
    /// As a workaround, we split the transaction in several transactions
    /// containing only one message and sum the costs of all transactions.
    ///
    /// Also, the gas estimation endpoints do not include the gas costs of
    /// reading and persisting the state.
    return async::sequence(getContext(), estimations)
        .flatMapPtr<BigInt>(getContext(), [](const auto &estimations) {
            const auto result =
                std::accumulate(std::begin(estimations), std::end(estimations), BigInt::ZERO);
            return FuturePtr<BigInt>::successful(std::make_shared<BigInt>(result));
        });

    /* Draft for gRPC implementation
    ** Currently this can't work because the serialize code follows AMINO and not protobuf
  auto fake_tx = std::make_unique<CosmosLikeTransactionApi>(
      *std::dynamic_pointer_cast<CosmosLikeTransactionApi>(transaction));
  if (!fake_tx) {
    return FuturePtr<BigInt>::successful(
        std::make_shared<BigInt>(BigInt::ZERO));
  }
  fake_tx->setDERSignature(std::vector<uint8_t>(40));
  const auto payload = fake_tx->serializeForBroadcast("async");
  std::cout << "Payload :\n\n" << payload << std::endl;
  const auto bytePayload = std::vector<uint8_t>(payload.cbegin(), payload.cend());
  const auto endpoint = fmt::format(cosmos::constants::kGrpcSimulationEndpoint, _ns, _version);
  const auto headers =
        std::unordered_map<std::string, std::string>{{"Content-Type", "application/json"}};
  return _http->POST(endpoint, bytePayload, headers)
      .json()
      .template flatMapPtr<BigInt>(
          _executionContext,
          [](const HttpRequest::JsonResult &result) -> FuturePtr<BigInt> {
            // The content of this payload actually depends on the mode of the
            // transaction block has both check_tx and deliver_tx sync has
            // check_tx async has nothing
            auto json = std::get<1>(result)->GetObject();
            return FuturePtr<BigInt>::successful(
                std::make_shared<BigInt>(BigInt::fromDecimal(
                    json["gas_info"].GetObject()["gas_used"].GetString())));
          });
             */
}

}  // namespace core
}  // namespace ledger
