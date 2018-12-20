#pragma once

#ifndef __MINGW32__
    #include <experimental/filesystem>
#else
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
#endif // !__MINGW32__


#include <random>
#include <boost/lexical_cast.hpp>

namespace ledger {
    namespace core {
        namespace test {
#ifndef __MINGW32__
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
#else
            static std::string GetTempDirPath() {
                std::random_device rd;  //Will be used to obtain a seed for the random number engine
                std::mt19937 gen(rd());
                return boost::lexical_cast<std::string>(gen());
            }

            static std::string CreateTempDir() {
                auto path = GetTempDirPath();
                mkdir(path.c_str());
                return path;
            }
#endif // !__MINGW32__

        }
    }
}
