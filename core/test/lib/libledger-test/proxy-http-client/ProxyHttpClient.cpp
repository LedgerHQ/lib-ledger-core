#include "ProxyHttpClient.hpp"
#include "api/Error.hpp"
#include "api/HttpMethod.hpp"
#include "api/HttpRequest.hpp"
#include <api/HttpReadBodyResult.hpp>
#include <experimental/filesystem>
#include <algorithm>
#include <fstream>
#include <functional>
#include <memory>
#include <mutex>
#include <gtest/gtest.h>

#define HTTP_CACHE_DIR "http_cache"

#if defined(UPDATE_HTTP_CACHE) && defined(ALLOW_HTTP_ACCESS) && !defined(LOAD_HTTP_CACHE)
#pragma message "ProxyHttpClient is configured to update HTTP cache"
#elif !UPDATE_HTTP_CACHE && ALLOW_HTTP_ACCESS && !LOAD_HTTP_CACHE
#pragma message "ProxyHttpClient is configured to run tests agains real web services"
#elif !UPDATE_HTTP_CACHE && !ALLOW_HTTP_ACCESS && LOAD_HTTP_CACHE
#pragma message "ProxyHttpClient is configured to use HTTP cache instead of real web services"
#else
#error "Incorrect configuration of HTTP caching"
#endif

namespace fs = std::experimental::filesystem;

namespace ledger {
    namespace core {
        namespace test {
            namespace impl {
                class TrafficLogger {
                public:
                    TrafficLogger()
                        : _cache_fn(getCacheFileName()) {
                    }

                    ~TrafficLogger() {
                        save();
                    }

                    void add(const std::string& type,
                             const std::string& url,
                             const std::string& request_body,
                             const std::string& response_body) {
                        std::ostringstream oss;
                        oss << type << " " << url << std::endl;
                        oss << encodeBody(request_body);

                        std::lock_guard<std::mutex> guard(_mutex);
                        _data[oss.str()] = encodeBody(response_body);
                    }

                    static std::string getCacheFileName() {
                        const testing::TestInfo* const test_info =
                            testing::UnitTest::GetInstance()->current_test_info();

                        const std::string cache_fn = std::string(HTTP_CACHE_DIR)
                                + std::string("/")
                                + std::string(test_info->test_suite_name())
                                + std::string(".")
                                + std::string(test_info->name());

                        return cache_fn;
                    }

                protected:
                    typedef std::unordered_map<std::string, std::string> map_type_t;

                    std::string encodeBody(const std::string& body) {
                        std::ostringstream oss;
                        if (body.size() == 0) {
                            oss << 0 << std::endl;
                        } else {
                            oss << std::count(body.begin(), body.end(), '\n') + 1 << std::endl;
                            oss << body << std::endl;
                        }
                        return oss.str();
                    }

                    void save() {
                        std::lock_guard<std::mutex> guard(_mutex);

                        if (_data.empty())
                            return;

                        if (!fs::exists(HTTP_CACHE_DIR))
                            fs::create_directory(HTTP_CACHE_DIR);

                        std::ofstream out(_cache_fn);

                        for (const auto& kv : _data)
                            out << kv.first << kv.second << std::endl;
                    }

                    std::mutex _mutex;
                    map_type_t _data;
                    std::string _cache_fn;
                };

                class LoggingHttpRequest : public api::HttpRequest {
                public:
                    typedef std::function<void(const std::string& type,
                                               const std::string& url,
                                               const std::vector<uint8_t>& req_body,
                                               const std::string& resp_body)> callback_t;

                    LoggingHttpRequest(const std::shared_ptr<api::HttpRequest>& request, callback_t fn)
                        : _request(request), _callback(fn) {}

                    virtual ~LoggingHttpRequest() {}

                    virtual api::HttpMethod getMethod() {
                        return _request->getMethod();
                    }

                    virtual std::unordered_map<std::string, std::string> getHeaders() {
                        return _request->getHeaders();
                    }

                    virtual std::vector<uint8_t> getBody() {
                        return _request->getBody();
                    }

                    virtual std::string getUrl() {
                        return _request->getUrl();
                    }

                    virtual void complete(const std::shared_ptr<api::HttpUrlConnection> & response,
                                          const std::experimental::optional<api::Error> & error) {
                        const std::string response_body = readResponseBody(response);
                        _callback(to_string(getMethod()), getUrl(), getBody(), response_body);
                        _request->complete(FakeUrlConnection::fromString(response_body), error);
                    }

                protected:
                    static std::string readResponseBody(const std::shared_ptr<api::HttpUrlConnection> & response) {
                        std::string response_body;
                        while (true) {
                            api::HttpReadBodyResult body = response->readBody();

                            if (body.error)
                                throw std::runtime_error(body.error->message);

                            if (body.data) {
                                const std::vector<uint8_t>& data = *(body.data);
                                if (data.empty())
                                    break;
                                response_body += std::string(data.begin(), data.end());
                                continue;
                            }
                            break;
                        }
                        return response_body;
                    }

                    const std::shared_ptr<api::HttpRequest> _request;
                    callback_t _callback;
                };
            }

            ProxyHttpClient::ProxyHttpClient(std::shared_ptr<api::HttpClient> httpClient)
                : _httpClient(httpClient)
            {
#ifdef UPDATE_HTTP_CACHE
                _logger = std::make_shared<impl::TrafficLogger>();
#endif

#ifdef LOAD_HTTP_CACHE
                if (!_logger)
                    loadCache(impl::TrafficLogger::getCacheFileName());
#endif
            }

            void ProxyHttpClient::execute(const std::shared_ptr<api::HttpRequest>& request) {
                auto vector_uint8_to_string = [](const std::vector<uint8_t>& v) {
                    return std::string(v.begin(), v.end());
                };
                auto it = _cache.find(request->getUrl() + vector_uint8_to_string(request->getBody()));
                if (it != _cache.end() && !_logger) {
                    std::cout << "get response from cache : " << request->getUrl() << std::endl;
                    request->complete(it->second, std::experimental::nullopt);
                    return;
                }

#ifndef ALLOW_HTTP_ACCESS
                std::cout << "response not found in cache : " << request->getUrl() << std::endl;
                throw std::runtime_error("HTTP access isn't allowed.");
#endif
                if (_logger) {
                    std::shared_ptr<impl::LoggingHttpRequest> temp
                            = std::make_shared<impl::LoggingHttpRequest>(request,
                                                                    [this, vector_uint8_to_string](const std::string& type,
                                                                                                   const std::string& url,
                                                                                                   const std::vector<uint8_t>& req_body,
                                                                                                   const std::string& resp_body){
                        _logger->add(type, url, vector_uint8_to_string(req_body), resp_body);
                    });

#ifdef ALLOW_HTTP_ACCESS
                    _httpClient->execute(temp);
#endif
                } else {
#ifdef ALLOW_HTTP_ACCESS
                    _httpClient->execute(request);
#endif
                }
            }

            void ProxyHttpClient::loadCache(const std::string& file_name) {
                // Cache file format:
                // (GET|POST) URL
                // request length in lines
                // request body
                // response length in lines
                // response body
                // empty line
                auto read_body = [](std::ifstream& file) {
                    std::string s, line;
                    size_t n = 0;

                    file >> n;
                    while (n > 0 && std::getline(file, line)) {
                        if (s.empty() && line.empty())
                            continue;
                        if (!s.empty())
                            s += "\n";
                        s += line;
                        n--;
                    }

                    return s;
                };

                std::ifstream input_file(file_name);
                if (input_file.is_open()) {
                    std::string line;
                    std::string type, url;
                    while (input_file >> type >> url) {
                        const std::string request_body = read_body(input_file);
                        const std::string response_body = read_body(input_file);

                        url += request_body;
                        addCache(url, response_body);
                    }
                }
            }

            void ProxyHttpClient::addCache(const std::string& url, const std::string& body) {
                _cache.emplace(url, FakeUrlConnection::fromString(body));
            }
        }
    }
}
