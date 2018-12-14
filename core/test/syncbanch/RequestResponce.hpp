#pragma once
#include <asio.hpp>
#include <api/HttpUrlConnection.hpp>
#include <network/uri.hpp>

namespace ledger {
    namespace core {
        namespace api {
            class HttpRequest;
        }
    }
}

struct HttpUrlConnection : public ledger::core::api::HttpUrlConnection {
    int32_t statusCode;
    std::string statusText;
    std::unordered_map<std::string, std::string> headers;
    std::vector<uint8_t> body;

    int32_t getStatusCode() override;
    std::string getStatusText() override;
    std::unordered_map<std::string, std::string> getHeaders() override;
    ledger::core::api::HttpReadBodyResult readBody() override;
};


class RequestResponse : public std::enable_shared_from_this<RequestResponse>
{
public:
    RequestResponse(
        asio::io_context& io_context,
        const std::shared_ptr<ledger::core::api::HttpRequest>& request);
    void execute();
private:
    void handle_resolve(const asio::error_code& err, const asio::ip::tcp::resolver::results_type& endpoints);
    void handle_connect(const asio::error_code& err);
    void handle_write_request(const asio::error_code& err);
    void handle_read_status_line(const asio::error_code& err);
    void handle_read_headers(const asio::error_code& err);
    void handle_read_content(const asio::error_code& err);

    asio::ip::tcp::resolver resolver_;
    asio::ip::tcp::socket socket_;
    asio::streambuf request_;
    asio::streambuf response_;
    std::shared_ptr<ledger::core::api::HttpRequest> apiRequest_;
    std::shared_ptr<HttpUrlConnection> urlConnection;
    network::uri uri;
};
