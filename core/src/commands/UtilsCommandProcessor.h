#pragma once
#include <string>
#include <memory>
#include "messages/utils/commands.pb.h"
#include "async/Future.hpp"

namespace ledger {
    namespace core {
        class UtilsCommandProcessor {
        public:
            Future<std::string> processRequest(const std::string& message);
        private:
            message::utils::CreateXpubFromPointsResponse processRequest(const message::utils::CreateXpubFromPointsRequest& req);
        };
    }
}