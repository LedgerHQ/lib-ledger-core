#pragma once
#include <string>
#include "api/PathResolver.hpp"

namespace ledger {
    namespace core {
        class PathResolver : public ledger::core::api::PathResolver {
        public:
            std::string resolveDatabasePath(const std::string& path) override { return path; };

            std::string resolveLogFilePath(const std::string& path) override { return path; };

            std::string resolvePreferencesPath(const std::string& path) override { return path; };
        };
} }
