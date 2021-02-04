/*
 *
 * FilesystemUtils.cpp
 * ledger-core
 *
 * Created by Huiqi ZHENG on 25/09/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2017 Ledger
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

#include "FilesystemUtils.hpp"
#include <iostream>
#include <experimental/filesystem>
#include <string>
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h>
#endif
#ifdef __linux__
    #include <unistd.h>
#endif
#ifdef __APPLE__
    #include <mach-o/dyld.h>
#endif

using namespace std;
namespace fs = std::experimental::filesystem::v1;

//refer to https://stackoverflow.com/questions/1528298/get-path-of-executable to get the executable path with C++
string FilesystemUtils::getExecutablePath(){
#ifdef _WIN32
    WCHAR buf[1024];
    GetModuleFileNameW(NULL, buf, 1024);
    wstring ws(buf);
    string res(ws.begin(),ws.end());
#else
    unsigned int path_max=1024;
    char buf[path_max];
    #ifdef __linux__
        int rslt = readlink("/proc/self/exe", buf, path_max - 1);
        buf[rslt] = '\0';
    #endif
    #ifdef __APPLE__
        _NSGetExecutablePath(buf, &path_max);
    #endif
    string res(buf);
#endif
  return res;
}

string FilesystemUtils::getExecutableDir(){
    auto path = getExecutablePath();
#ifdef _WIN32
    auto last_slash_idx = path.rfind('\\');
#else
    auto last_slash_idx = path.rfind('/');
#endif
    if (string::npos != last_slash_idx) {
        return path.substr(0, last_slash_idx);
    }
    else {
        return path;
    }
}

bool FilesystemUtils::isExecutableOrLib(const std::string& path){
    fs::path filePath { path };
#ifdef _WIN32
    auto extension=filePath.extension().string();
    return (extension==".exe") || (extension==".bat") || (extension==".com") || (extension == ".lib") || (extension == ".dll") || (extension == ".pdb") || (extension == ".ilk");
#else
    auto permissions=status(filePath).permissions();
    return (permissions & (fs::perms::owner_exec | fs::perms::group_exec | fs::perms::others_exec))!=fs::perms::none;
#endif
}

void FilesystemUtils::clearFs(const std::string& path) {
    error_code ec;
    for (const auto & file : fs::recursive_directory_iterator(path))
    {
        if (!fs::is_directory(file.path())) {
            if (!FilesystemUtils::isExecutableOrLib(file.path().string())){
                fs::remove(file.path(), ec);
            }
        }
    }
}
