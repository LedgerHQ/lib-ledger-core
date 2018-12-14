#include "RequestResponce.hpp"
#include <api/HttpReadBodyResult.hpp>
#include <api/HttpRequest.hpp>
#include <network/uri.hpp>

using namespace asio::ip;
using namespace std;
using namespace ledger::core;

int32_t HttpUrlConnection::getStatusCode() {
    return statusCode;
}

std::string HttpUrlConnection::getStatusText() {
    return statusText;
}

std::unordered_map<std::string, std::string> HttpUrlConnection::getHeaders() {
    return headers;
}

ledger::core::api::HttpReadBodyResult HttpUrlConnection::readBody() {
    auto b = body;
    body = std::vector<uint8_t>();
    return api::HttpReadBodyResult(std::experimental::optional<ledger::core::api::Error>(), b);
}

RequestResponse::RequestResponse(
    asio::io_context& io_context,
    const std::shared_ptr<ledger::core::api::HttpRequest> &request)
    : resolver_(io_context)
    , socket_(io_context)
    , apiRequest_(request)
    , uri(request->getUrl())
{

    urlConnection = std::make_shared<HttpUrlConnection>();
    std::ostream request_stream(&request_);
    auto hdrs = request->getHeaders();
    
    request_stream << "GET " << uri.path().to_string() + "?" + uri.query().to_string() + uri.fragment().to_string() << " HTTP/1.0\r\n";
    request_stream << "Host: " << uri.authority().to_string() << "\r\n";
    request_stream << "Accept: */*\r\n";
    request_stream << "User-Agent: Mozilla/5.0 (Windows NT 10.0; Win64; x64) AppleWebKit/537.36 (KHTML, like Gecko) Chrome/70.0.3538.110 Safari/537.36\r\n";
    request_stream << "Connection: close\r\n\r\n";
}

void RequestResponse::execute() {
        
    resolver_.async_resolve(
        uri.authority().to_string(),
        "http",
        [self = shared_from_this()](const asio::error_code& err, const tcp::resolver::results_type& endpoints) { self->handle_resolve(err, endpoints);
    });
}

void RequestResponse::handle_resolve(const asio::error_code& err,
    const tcp::resolver::results_type& endpoints)
{
    if (!err)
    {
        // Attempt a connection to each endpoint in the list until we
        // successfully establish a connection.

        asio::async_connect(socket_, endpoints,
            [self = shared_from_this()](const asio::error_code& err, const tcp::endpoint& endpoint)
        {
            self->handle_connect(err); 
        });
    }
    else
    {
        apiRequest_->complete(urlConnection, api::Error(api::ErrorCode::UNABLE_TO_CONNECT_TO_HOST, err.message()));
    }
}

void RequestResponse::handle_connect(const asio::error_code& err)
{
    if (!err)
    {
        // The connection was successful. Send the request.
        asio::async_write(socket_, request_,
            [self = shared_from_this()](const asio::error_code& err, std::size_t bytes_transferred) {self->handle_write_request(err); });
    }
    else
    {
        apiRequest_->complete(urlConnection, api::Error(api::ErrorCode::UNABLE_TO_CONNECT_TO_HOST, err.message()));
    }
}

void RequestResponse::handle_write_request(const asio::error_code& err)
{
    if (!err)
    {
        // Read the response status line. The response_ streambuf will
        // automatically grow to accommodate the entire line. The growth may be
        // limited by passing a maximum size to the streambuf constructor.

        asio::async_read_until(socket_, response_, "\r\n",
            [self = shared_from_this()](const asio::error_code& err,
                std::size_t bytes_transferred) {self->handle_read_status_line(err); });

    }
    else
    {
        apiRequest_->complete(urlConnection, api::Error(api::ErrorCode::NO_INTERNET_CONNECTIVITY, err.message()));
    }
}

void RequestResponse::handle_read_status_line(const asio::error_code& err)
{
    if (!err)
    {
        // Check that response is OK.
        std::istream response_stream(&response_);
        std::string http_version;
        response_stream >> http_version;
        unsigned int status_code;
        response_stream >> status_code;
        std::string status_message;
        std::getline(response_stream, status_message);
        if (!response_stream || http_version.substr(0, 5) != "HTTP/")
        {
            apiRequest_->complete(urlConnection, api::Error(api::ErrorCode::HTTP_ERROR, "Server send not a HTTP response"));
            return;
        }
        urlConnection->statusCode = status_code;
        urlConnection->statusText = status_message;
        switch (status_code) {
        case 302:
        {
            std::string location;
            std::getline(response_stream, location);
            std::cout << location << std::endl;
            std::getline(response_stream, location);
            std::cout << location << std::endl;
            break;
        }
        case 200:
        {
            // Read the response headers, which are terminated by a blank line.
            asio::async_read_until(socket_, response_, "\r\n\r\n",
                [self = shared_from_this()](const asio::error_code& err,
                    std::size_t bytes_transferred) { self->handle_read_headers(err); });
            break;
        }
        default:
        {
            apiRequest_->complete(urlConnection, api::Error(api::ErrorCode::HTTP_ERROR, status_message));
            return;
        }
        };
    }
    else
    {
        apiRequest_->complete(urlConnection, api::Error(api::ErrorCode::NO_INTERNET_CONNECTIVITY, err.message()));
    }
}

void RequestResponse::handle_read_headers(const asio::error_code& err)
{
    if (!err)
    {
        // Process the response headers.
        std::istream response_stream(&response_);
        std::string header;
        while (std::getline(response_stream, header) && header != "\r")
        {
            //TODO: fix this, make a split at :
            urlConnection->headers[header] = header;
        }

        // Start reading remaining data until EOF.
            
        asio::async_read(socket_, response_,
        asio::transfer_at_least(1),
            [self = shared_from_this()](const asio::error_code& err,
                std::size_t bytes_transferred) {self->handle_read_content(err); });

    }
    else
    {
        apiRequest_->complete(urlConnection, api::Error(api::ErrorCode::NO_INTERNET_CONNECTIVITY, err.message()));
    }
}

void RequestResponse::handle_read_content(const asio::error_code& err)
{
    if (!err)
    {
        // Continue reading remaining data until EOF.
        asio::async_read(
            socket_, 
            response_,
            asio::transfer_at_least(1),
            [self = shared_from_this()](const asio::error_code& err,
                std::size_t bytes_transferred) {self->handle_read_content(err); }
        );

    }
    else if (err == asio::error::eof)
    {
        urlConnection->body = std::vector<uint8_t>{ asio::buffers_begin(response_.data()), asio::buffers_end(response_.data()) };
        apiRequest_->complete(urlConnection, std::experimental::optional<api::Error>());
    }
    else
    {
        apiRequest_->complete(urlConnection, api::Error(api::ErrorCode::NO_INTERNET_CONNECTIVITY, err.message()));
    }
}
