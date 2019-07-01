#pragma once
#include <string>
#include "api/PathResolver.hpp"

namespace ledger {
    namespace core {
        class PathResolver : public ledger::core::api::PathResolver {
        public:
            std::string resolveDatabasePath(const std::string& path) override { return _workingDir + "/" + path; };

            std::string resolveLogFilePath(const std::string& path) override { return _workingDir + "/" + path; };

            std::string resolvePreferencesPath(const std::string& path) override { return _workingDir + "/preference/" + path; };

            void setWorkingDir(const std::string& workingDir) {
                _workingDir = workingDir;
            }
        private:
            std::string _workingDir;
        };
} }
