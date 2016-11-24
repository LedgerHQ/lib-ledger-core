/*
 *
 * Logger
 * ledger-core
 *
 * Created by Pierre Pollastri on 24/11/2016.
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
#ifndef LEDGER_CORE_LOGGER_HPP
#define LEDGER_CORE_LOGGER_HPP

#include "../api/PathResolver.hpp"
#include "../api/ExecutionContext.hpp"
#include "../api/LogPrinter.hpp"
#include "../utils/optional.hpp"
#include "../api/Logger.hpp"
#include <memory>
#include <spdlog/spdlog.h>

namespace ledger {

 namespace core {
     class Logger {
     public:
         Logger(const std::string& name,
                const std::shared_ptr<api::PathResolver>& resolver,
                const std::shared_ptr<api::ExecutionContext>& loggerContext,
                const std::shared_ptr<api::LogPrinter>& printer,
                const std::experimental::optional<std::string> &password
         );
         void d();
         std::shared_ptr<api::Logger> getApiLogger();
     private:
         std::string _name;
         std::weak_ptr<api::PathResolver> _resolver;
         std::weak_ptr<api::ExecutionContext> _context;
         std::weak_ptr<api::LogPrinter> _printer;
         std::experimental::optional<std::string> _password;
         std::shared_ptr<spdlog::logger> _logger;
     };
 }
}


#endif //LEDGER_CORE_LOGGER_HPP
