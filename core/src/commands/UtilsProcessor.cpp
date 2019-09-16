#include "UtilsProcessor.h"
#include "messages/utils/commands.pb.h"
#include "async/Promise.hpp"
#include "math/CryptoUtils.h"

namespace ledger {
namespace core {
    using namespace message::utils;

    CreateXpubFromPointsResponse UtilsCommandProcessor::processRequest(const CreateXpubFromPointsRequest& req) {
        CreateXpubFromPointsResponse resp;
        resp.set_xpub();
        return resp;
    }


    Future<std::string> UtilsCommandProcessor::processRequest(const std::string& request) {
        UtilsRequest req;
        if (!req.ParseFromString(request)) {
            throw std::runtime_error("Can't parse UtilsRequest");
        }
        switch (req.request_case())
        {
        case UtilsRequest::RequestCase::kXpubFromPoints: {
            return Future<std::string>::successful(processRequest(req.xpub_from_points()).SerializeAsString());
        }
        default:
            throw std::runtime_error("Unknown UtilsRequestType");;
        }
    }
}
}