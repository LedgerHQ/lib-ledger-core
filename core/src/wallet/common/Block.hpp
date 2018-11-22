#pragma once

#include <string>
#include <chrono>

namespace ledger {
    namespace core {
        struct Block {
            std::string hash;
            uint32_t height;
            std::chrono::system_clock::time_point createdAt;
        };
    }
}
