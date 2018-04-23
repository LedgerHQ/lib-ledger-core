/*
 *
 * NativePathResolver
 * ledger-core
 *
 * Created by Pierre Pollastri on 22/11/2016.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#ifndef LEDGER_CORE_NATIVEPATHRESOLVER_HPP
#define LEDGER_CORE_NATIVEPATHRESOLVER_HPP

#include <src/api/PathResolver.hpp>
#include <vector>
#include <utils/Option.hpp>

class NativePathResolver : public ledger::core::api::PathResolver {
public:
    NativePathResolver();
    explicit NativePathResolver(const ledger::core::Option<std::string>& rootDirPath);
    virtual std::string resolveDatabasePath(const std::string &path) override;

    virtual std::string resolveLogFilePath(const std::string &path) override;

    virtual std::string resolvePreferencesPath(const std::string &path) override;
    void clean();
private:
    std::vector<std::string> _createdPaths;
    ledger::core::Option<std::string> _rootDirPath;
};


#endif //LEDGER_CORE_NATIVEPATHRESOLVER_HPP
