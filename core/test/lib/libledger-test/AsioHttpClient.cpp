/*
 *
 * AsioHttpClient
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/03/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#include "AsioHttpClient.hpp"
#include "NativeThreadDispatcher.hpp"
#include <asio.hpp>
#include <ledger/core/net/URI.hpp>

using namespace ledger::core;

AsioHttpClient::AsioHttpClient(const std::shared_ptr<ledger::core::api::ExecutionContext> &context)  : _sslContext(asio::ssl::context::tlsv12) {
    _context = context;
    asio::error_code err;
    _io_service.run();
    _sslContext.set_default_verify_paths(err);
}

void AsioHttpClient::execute(const std::shared_ptr<ledger::core::api::HttpRequest> &request) {
    struct Query {
        asio::streambuf request;
        asio::streambuf response;
        asio::ip::tcp::resolver resolver;
        asio::ssl::stream<asio::ip::tcp::socket> socket;

        std::shared_ptr<ledger::core::api::HttpRequest> apiRequest;

        Query(asio::ssl::context& context, asio::io_service& io_service, const std::shared_ptr<ledger::core::api::HttpRequest> &request)
        : resolver(io_service), socket(io_service, context), apiRequest(request) {

        }
    };
    auto q = std::make_shared<Query>(_sslContext, _io_service, request);

    _context->execute(make_runnable([q] () {
        URI uri(q->apiRequest->getUrl());
        std::ostream request_stream(&q->request);

        std::cout << "GET " << uri.getPath().str() << uri.getQuery().str() << uri.getFragment().str() << " HTTP/1.1\r\n";
        std::cout << "Host: " << uri.getDomain() << "\r\n";
        std::cout << "User-Agent: asio-http-client\r\n";
        std::cout << "Accept: */*\r\n\r\n";

        request_stream << "GET " << uri.getPath().str() << uri.getQuery().str() << uri.getFragment().str() << " HTTP/1.1\r\n";
        request_stream << "Host: " << uri.getDomain() << "\r\n";
        request_stream << "Accept: */*\r\n";
        request_stream << "User-Agent: asio-http-client\r\n\r\n";

        // Start an asynchronous resolve to translate the server and service names
        // into a list of endpoints.
        asio::ip::tcp::resolver::query query(uri.getDomain().str(), uri.getScheme().str());
        asio::error_code err;
        auto resolveResult = q->resolver.resolve(query, err);
        if (err) {
            q->apiRequest->complete(nullptr, api::Error(api::ErrorCode::UNABLE_TO_RESOLVE_HOST, err.message()));
            return ;
        }
        q->socket.set_verify_mode(asio::ssl::verify_peer);
        q->socket.set_verify_callback([] (bool preverified, asio::ssl::verify_context& ctx) -> bool {
            char subject_name[256];
            X509* cert = X509_STORE_CTX_get_current_cert(ctx.native_handle());
            X509_NAME_oneline(X509_get_subject_name(cert), subject_name, 256);

            return true;

        });
        SSL_set_tlsext_host_name(q->socket.native_handle(), "sni79463.cloudflaressl.com");
        asio::connect(q->socket.lowest_layer(), resolveResult, err);
        if (err) {
            q->apiRequest->complete(nullptr, api::Error(api::ErrorCode::UNABLE_TO_CONNECT_TO_HOST, err.message()));
            return ;
        }
        q->socket.handshake(asio::ssl::stream_base::client, err);
        if (err) {
            q->apiRequest->complete(nullptr, api::Error(api::ErrorCode::SSL_ERROR, err.message()));
            return ;
        }
        asio::write(q->socket, q->request, err);
        if (err) {
            q->apiRequest->complete(nullptr, api::Error(api::ErrorCode::HTTP_ERROR, err.message()));
            return ;
        }
        asio::read_until(q->socket, q->response, "\r\n", err);
        if (err) {
            q->apiRequest->complete(nullptr, api::Error(api::ErrorCode::HTTP_ERROR, err.message()));
            return ;
        }
        unsigned int status_code;
        {
            // Check that response is OK.
            std::istream response_stream(&q->response);
            std::string http_version;
            response_stream >> http_version;

            response_stream >> status_code;
            std::string status_message;
            std::getline(response_stream, status_message);
            if (!response_stream || http_version.substr(0, 5) != "HTTP/") {
                q->apiRequest->complete(nullptr, api::Error(api::ErrorCode::HTTP_ERROR, "Invalid HTTP response"));
                return;
            }

            // Read the response headers, which are terminated by a blank line.
            asio::read_until(q->socket, q->response, "\r\n\r\n", err);
            if (err) {
                q->apiRequest->complete(nullptr, api::Error(api::ErrorCode::HTTP_ERROR, err.message()));
                return;
            }
        }
        {
            // Process the response headers.
            std::istream response_stream(&q->response);
            std::string header;
            while (std::getline(response_stream, header) && header != "\r")
                std::cout << header << "\n";
            std::cout << "\n";

            // Write whatever content we already have to output.
            if (q->response.size() > 0)
                std::cout << &q->response;


            std::cout.flush();
            // Start reading remaining data until EOF.
            while (!err && err != asio::error::eof) {
                asio::read(q->socket, q->response, asio::transfer_at_least(1));
                if (!err) {
                    std::cout << &q->response;
                }
            }

        }
    }));
}
