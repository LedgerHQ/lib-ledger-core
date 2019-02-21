/*
 *
 * RotatingEncryptableSink
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
#ifndef LEDGER_CORE_ROTATINGENCRYPTABLESINK_HPP
#define LEDGER_CORE_ROTATINGENCRYPTABLESINK_HPP

#include "spdlog/spdlog.h"
#include "spdlog/sinks/sink.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "api/ExecutionContext.hpp"
#include "api/PathResolver.hpp"
#include <memory>
#include "utils/optional.hpp"
#include <mutex>

namespace ledger {
    namespace core {
        /**
         * Based on spdlog::sinks::rotating_file_sink
         */
        class RotatingEncryptableSink : public spdlog::sinks::base_sink<std::mutex>, public std::enable_shared_from_this<RotatingEncryptableSink> {
        public:
            RotatingEncryptableSink(
                    const std::shared_ptr<api::ExecutionContext> &context,
                    const std::shared_ptr<api::PathResolver> &resolver,
                    const std::string &name,
                    std::size_t maxSize,
                    std::size_t maxFiles
            );
            virtual void sink_it_(const spdlog::details::log_msg &msg) override;
            virtual void flush_() override;

        protected:
            void _sink_it(std::shared_ptr<fmt::memory_buffer> msg);

        private:
            static spdlog::filename_t calc_filename(
                    std::shared_ptr<api::PathResolver> resolver,
                    const spdlog::filename_t& filename, std::size_t index, const spdlog::filename_t& extension);

            void _rotate();

#if defined(_WIN32) || defined(_WIN64)
            static void ToWide(const std::string &input, std::wstring &output);
            static void ToNarrow(const std::wstring &input, std::string &output);
#endif

            std::shared_ptr<api::ExecutionContext> _context;
            std::weak_ptr<api::PathResolver> _resolver;
            std::string _name;
            spdlog::filename_t _base_filename;
            spdlog::filename_t _extension;
            std::size_t _max_size;
            std::size_t _max_files;
            std::size_t _current_size;
            spdlog::details::file_helper _file_helper;
        };
    }
}


#endif //LEDGER_CORE_ROTATINGENCRYPTABLESINK_HPP
