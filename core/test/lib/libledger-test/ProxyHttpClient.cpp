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

using namespace std;
namespace fs = experimental::filesystem;

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

                    void add(const string& type,
                             const string& url,
                             const string& request_body,
                             const string& response_body) {
                        ostringstream oss;
                        oss << type << " " << url << endl;
                        oss << encodeBody(request_body);

                        lock_guard<mutex> guard(_mutex);
                        _data[oss.str()] = encodeBody(response_body);
                    }

                    static string getCacheFileName() {
                        const testing::TestInfo* const test_info =
                            testing::UnitTest::GetInstance()->current_test_info();

                        const string cache_fn = string(HTTP_CACHE_DIR)
                                + string("/")
                                + string(test_info->test_suite_name())
                                + string(".")
                                + string(test_info->name());

                        return cache_fn;
                    }

                protected:
                    typedef unordered_map<string, string> map_type_t;

                    string encodeBody(const string& body) {
                        ostringstream oss;
                        if (body.size() == 0) {
                            oss << 0 << endl;
                        } else {
                            oss << count(body.begin(), body.end(), '\n') + 1 << endl;
                            oss << body << endl;
                        }
                        return oss.str();
                    }

                    void save() {
                        lock_guard<mutex> guard(_mutex);

                        if (_data.empty())
                            return;

                        if (!fs::exists(HTTP_CACHE_DIR))
                            fs::create_directory(HTTP_CACHE_DIR);

                        ofstream out(_cache_fn);

                        for (const auto& kv : _data)
                            out << kv.first << kv.second << endl;
                    }

                    mutex _mutex;
                    map_type_t _data;
                    string _cache_fn;
                };

                class LoggingHttpRequest : public api::HttpRequest {
                public:
                    typedef function<void(const string& type,
                                          const string& url,
                                          const vector<uint8_t>& req_body,
                                          const string& resp_body)> callback_t;

                    LoggingHttpRequest(const shared_ptr<api::HttpRequest>& request, callback_t fn)
                        : _request(request), _callback(fn) {}

                    virtual ~LoggingHttpRequest() {}

                    virtual api::HttpMethod getMethod() {
                        return _request->getMethod();
                    }

                    virtual unordered_map<string, string> getHeaders() {
                        return _request->getHeaders();
                    }

                    virtual vector<uint8_t> getBody() {
                        return _request->getBody();
                    }

                    virtual string getUrl() {
                        return _request->getUrl();
                    }

                    virtual void complete(const shared_ptr<api::HttpUrlConnection> & response, const experimental::optional<api::Error> & error) {
                        const string response_body = readResponseBody(response);
                        _callback(to_string(getMethod()), getUrl(), getBody(), response_body);
                        _request->complete(FakeUrlConnection::fromString(response_body), error);
                    }

                protected:
                    static string readResponseBody(const shared_ptr<api::HttpUrlConnection> & response) {
                        string response_body;
                        while (true) {
                            api::HttpReadBodyResult body = response->readBody();

                            if (body.error)
                                throw runtime_error(body.error->message);

                            if (body.data) {
                                const vector<uint8_t>& data = *(body.data);
                                if (data.empty())
                                    break;
                                response_body += string((char*)data.data(), data.size());
                                continue;
                            }
                            break;
                        }
                        return response_body;
                    }

                    const shared_ptr<api::HttpRequest> _request;
                    callback_t _callback;
                };
            }

            ProxyHttpClient::ProxyHttpClient(shared_ptr<api::HttpClient> httpClient)
                : _httpClient(httpClient)
            {
#ifdef UPDATE_HTTP_CACHE
                _logger = make_shared<impl::TrafficLogger>();
#endif

#ifdef LOAD_HTTP_CACHE
                if (!_logger) {
                    loadCache(impl::TrafficLogger::getCacheFileName());
                }
#endif
            }

            void ProxyHttpClient::execute(const shared_ptr<api::HttpRequest>& request) {
                auto vector_uint8_to_string = [](const vector<uint8_t>& v) {
                    return string((const char*)v.data(), v.size());
                };
                auto it = _cache.find(request->getUrl() + vector_uint8_to_string(request->getBody()));
                if (it != _cache.end() && !_logger) {
                    cout << "get response from cache : " << request->getUrl() << endl;
                    request->complete(it->second, experimental::nullopt);
                    return;
                }

#ifndef ALLOW_HTTP_ACCESS
                throw runtime_error("HTTP access isn't allowed.");
#endif
                if (_logger) {
                    shared_ptr<impl::LoggingHttpRequest> temp
                            = make_shared<impl::LoggingHttpRequest>(request,
                                                                    [this, vector_uint8_to_string](const string& type,
                                                                                                   const string& url,
                                                                                                   const vector<uint8_t>& req_body,
                                                                                                   const string& resp_body){
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

            void ProxyHttpClient::loadCache(const string& file_name) {
                // Cache file format:
                // (GET|POST) URL
                // request length in lines
                // request body
                // response length in lines
                // response body
                // empty line
                auto read_body = [](ifstream& file) {
                    string s, line;
                    size_t n = 0;

                    file >> n;
                    while (n > 0 && getline(file, line)) {
                        if (s.empty() && line.empty())
                            continue;
                        if (!s.empty())
                            s += "\n";
                        s += line;
                        n--;
                    }

                    return s;
                };

                ifstream input_file(file_name);
                if (input_file.is_open()) {
                    string line;
                    string type, url;
                    while (input_file >> type >> url) {
                        const string request_body = read_body(input_file);
                        const string response_body = read_body(input_file);

                        url += request_body;
                        addCache(url, response_body);
                    }
                }
            }

            void ProxyHttpClient::addCache(const string& url, const string& body) {
                _cache.emplace(url, FakeUrlConnection::fromString(body));
            }
        }
    }
}
