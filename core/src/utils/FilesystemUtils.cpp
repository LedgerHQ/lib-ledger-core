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

using namespace std;
namespace fs = std::experimental::filesystem::v1;

bool ledger::core::FilesystemUtils::isExecutable(const std::string& path){
    fs::path filePath{path};
#ifdef _WIN32
    auto extension=filePath.extension().string();
    return (extension==".exe") || (extension==".bat") || (extension==".com")
#else
    return (status(filePath).permissions() & fs::perms::owner_exec)!=fs::perms::none;
#endif
}

void ledger::core::FilesystemUtils::clearFs(const std::string& path) {
    fs::path filePath{path};
    for (const auto & file : fs::recursive_directory_iterator(path))
    {
        if (!fs::is_directory(file.path())) {
            if (!FilesystemUtils::isExecutable(file.path().string())){
                fs::remove(file.path());
            }
        }
    }
}
