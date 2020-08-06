#include "HttpClientOnFakeExplorer.hpp"
#include <unordered_map>
#include <boost/algorithm/string.hpp>
#include "api/HttpRequest.hpp"
#include "api/HttpReadBodyResult.hpp"
#include "utils/optional.hpp"
#include "api/Error.hpp"

namespace ledger {
    namespace core {
        namespace test {
            std::string createTrunsactionBulkJson(std::vector<std::string>& transactions) {
                return "{\"truncated\":false, \"txs\" : [" + boost::algorithm::join(transactions, ",") + "]}";
            }

            std::unordered_map<std::string, std::string> parseParameters(const std::string& parameters) {
                std::unordered_map<std::string, std::string> result;
                std::vector<std::string> splited;
                boost::split(splited, parameters, [](char c) { return c == '&'; });
                for (auto& param : splited) {
                    if (param.empty())
                        continue;
                    std::vector<std::string> key_value;
                    boost::split(key_value, param, [](char c) { return c == '='; });
                    if (key_value.size() == 1) {
                        result[key_value[0]] = "";
                    }
                    else {
                        result[key_value[0]] = key_value[1];
                    }
                }
                return result;
            }

            HttpClientOnFakeExplorer::HttpClientOnFakeExplorer(std::shared_ptr<ExplorerStorage> explorer) : _explorer(explorer) {
            };

            void HttpClientOnFakeExplorer::execute(const std::shared_ptr<api::HttpRequest>& request) {
                std::string url = request->getUrl();
                // TODO: replace this code when we will have "standart" url parsing 
                std::vector<std::string> result;
                boost::split(result, url, [](char c) {return c == '?'; });
                std::unordered_map<std::string, std::string> parameters;
                if (result.size() > 1)
                    parameters = parseParameters(result[1]);
                std::vector<std::string> pathComponents;
                boost::split(pathComponents, result[0], [](char c) {return c == '/'; });
                auto it = std::find(pathComponents.begin(),pathComponents.end(), "addresses");
                if (it != pathComponents.end()) {
                    auto addressesIt = it + 1;
                    if (addressesIt == pathComponents.end()) {
                        request->complete(std::shared_ptr<api::HttpUrlConnection>(), api::Error(api::ErrorCode::API_ERROR, ""));
                        return;
                    }
                    std::vector<std::string> addresses;
                    boost::split(addresses, *addressesIt, [](char c) {return c == ','; });
                    std::string blockHash = "";
                    auto blockHashIt = parameters.find("blockHash");
                    if (blockHashIt != parameters.end()) {
                        blockHash = blockHashIt->second;
                    }
                    auto transactions = _explorer->getTransactions(addresses, blockHash);
                    request->complete(FakeUrlConnection::fromString(createTrunsactionBulkJson(transactions)), std::experimental::optional<api::Error>());
                    return;
                }
                it = std::find(pathComponents.begin(), pathComponents.end(), "current");
                if (it != pathComponents.end()) {
                    std::string lastBlock = _explorer->getLastBlock();
                    if (lastBlock.empty()) {
                        request->complete(std::shared_ptr<api::HttpUrlConnection>(), api::Error(api::ErrorCode::BLOCK_NOT_FOUND, "Block not found"));
                        return;
                    }
                    request->complete(FakeUrlConnection::fromString(lastBlock), std::experimental::optional<api::Error>());
                    return;
                }
                it = std::find(pathComponents.begin(), pathComponents.end(), "syncToken");
                if (it != pathComponents.end()) {
                    request->complete(FakeUrlConnection::fromString("{\"token\":\"PLEASE-LET-ME-IN\"}"), std::experimental::optional<api::Error>());
                    return;
                }
            }

        }
    }
}