#pragma once

#if defined(__MINGW32__) || defined(__clang__)
    #include <sys/stat.h>
    #include <sys/types.h>
    #include <unistd.h>
#else
    #include <experimental/filesystem>
#endif // !__MINGW32__


#include <random>
#include <boost/lexical_cast.hpp>

namespace ledger {
    namespace core {
        namespace test {
#if defined(__MINGW32__) || defined (__clang__)
            static std::string GetTempDirPath() {
                std::random_device rd;  //Will be used to obtain a seed for the random number engine
                std::mt19937 gen(rd());
                return boost::lexical_cast<std::string>(gen());
            }

            static std::string CreateTempDir() {
                auto path = GetTempDirPath();
#ifdef __MINGW32__
                mkdir(path.c_str());
#else
                mkdir(path.c_str(), 0777);
#endif
                return path;
            }
#else
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
#endif // !__MINGW32__

        }
    }
}
