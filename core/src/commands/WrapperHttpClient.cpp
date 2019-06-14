#include "WrapperHttpClient.hpp"
#include "services.pb.h"
#include "api/HttpRequest.hpp"
#include "api/HttpMethod.hpp"
#include "api/HttpUrlConnection.hpp"
#include "api/Error.hpp"
#include "api/HttpReadBodyResult.hpp"
#include "utils/optional.hpp"

namespace ledger {
    namespace core {
        using namespace message;

        struct HttpUrlConnection : public api::HttpUrlConnection {
            int32_t statusCode;
            std::string statusText;
            std::unordered_map<std::string, std::string> headers;
            std::vector<uint8_t> body;

            int32_t getStatusCode() { return statusCode; };
            std::string getStatusText() { return statusText; };
            std::unordered_map<std::string, std::string> getHeaders() { return headers; };
            api::HttpReadBodyResult readBody() {
                auto b = body;
                body = std::vector<uint8_t>();
                return api::HttpReadBodyResult(std::experimental::optional<ledger::core::api::Error>(), b);
            };
        };


        WrapperHttpClient::WrapperHttpClient(const WrapperHttpClient::OnRequest& onRequest)
            : _onRequest(onRequest) {
        };

        void WrapperHttpClient::execute(const std::shared_ptr<api::HttpRequest>& request) {
            // create HttpRequest message and send to client
            HttpRequest httpReq;
            std::string method = api::to_string(request->getMethod());
            if (method == "DEL") {
                method = "DELETE";
            }
            httpReq.set_method(method);
            httpReq.set_url(request->getUrl());
            std::vector<uint8_t> body = request->getBody();
            auto& headers = *httpReq.mutable_headers();
            for (auto& p : request->getHeaders()) {
                headers[p.first] = p.second;
            }
            httpReq.set_body(std::string((char*)body.data(), body.size()));
            ServiceRequest serviceReq;
            serviceReq.set_type(ServiceRequestType::HTTP_REQ);
            std::string serialized;
            if (!httpReq.SerializeToString(&serialized)) {
                throw std::runtime_error("Can't serialize http request");
            }
            serviceReq.set_request_body(serialized);
            std::vector<uint8_t> buff(serviceReq.ByteSize());
            if (!serviceReq.SerializeToArray(&buff[0], buff.size())) {
                throw std::runtime_error("Can't serialize service request");
            }
            _onRequest(std::move(buff), [request](std::vector<uint8_t>&& responseData) {
                auto urlConnection = std::make_shared<HttpUrlConnection>();
                ServiceResponse resp;
                if (!resp.ParseFromArray(&responseData[0], responseData.size())) {
                    request->complete(urlConnection, api::Error(api::ErrorCode::RUNTIME_ERROR, "Can't parse service response"));
                    return;
                }
                if (resp.error() != "") {
                    throw std::runtime_error("Error during service request: " + resp.error());
                }
                HttpResponse httpResponse;
                if (!httpResponse.ParseFromString(resp.response_body())) {
                    request->complete(urlConnection, api::Error(api::ErrorCode::RUNTIME_ERROR, "Can't parse http response"));
                    return;
                }
                urlConnection->statusCode = httpResponse.code();
                std::string body = httpResponse.body();
                urlConnection->body.assign(&body[0], &body[0] + body.size());
                request->complete(urlConnection, std::experimental::optional<api::Error>());
                });
        };
    }
}


