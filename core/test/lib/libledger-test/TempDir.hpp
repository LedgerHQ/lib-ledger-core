#pragma once

#include <experimental/filesystem>
#include <random>
#include <boost/lexical_cast.hpp>

namespace ledger {
    namespace core {
        namespace test {
            static std::string GetTempDirPath() {
                std::random_device rd;  //Will be used to obtain a seed for the random number engine
                std::mt19937 gen(rd());
                auto path = std::experimental::filesystem::temp_directory_path();
                path.append(boost::lexical_cast<std::string>(gen()));
                return path.string();
            }

            static std::string CreateTempDir() {
                auto path = GetTempDirPath();
                std::experimental::filesystem::create_directory(path);
                return path;
            }
        }
    }
}
