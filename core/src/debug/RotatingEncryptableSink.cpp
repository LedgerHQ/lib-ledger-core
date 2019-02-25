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
#include "fmt/format.h"
#include "RotatingEncryptableSink.hpp"
#include "utils/LambdaRunnable.hpp"
#include "../api/ErrorCode.hpp"
#include "../utils/Exception.hpp"

namespace ledger {
    namespace core {
        RotatingEncryptableSink::RotatingEncryptableSink(const std::shared_ptr<api::ExecutionContext> &context,
                                                         const std::shared_ptr<api::PathResolver> &resolver,
                                                         const std::string &name,
                                                         std::size_t maxSize,
                                                         std::size_t maxFiles) {
            _context = context;
            _resolver = resolver;
            _name = name;
#if defined(_WIN32) || defined(_WIN64)
            ToWide(name, _base_filename);
            ToWide("log", _extension);
#else
            _base_filename = name;
            _extension = "log";
#endif
            _max_size = maxSize;
            _max_files = maxFiles;
            _file_helper.open(calc_filename(resolver, _base_filename, 0, _extension));
            _current_size = _file_helper.size(); //expensive. called only once
            set_level(spdlog::level::trace);
        }

        void RotatingEncryptableSink::sink_it_(const spdlog::details::log_msg &msg) {
            auto context = _context;
            std::shared_ptr<fmt::memory_buffer> buffer = std::make_shared<fmt::memory_buffer>();
            formatter_->format(msg, *buffer);
            auto self = shared_from_this();
            context->execute(make_runnable([self, buffer] () {
                self->_sink_it(buffer);
            }));
        }

        void RotatingEncryptableSink::flush_() {
            auto context = _context;
            context->execute(make_runnable([this] () {
               _file_helper.flush();
            }));
        }

        void RotatingEncryptableSink::_sink_it(std::shared_ptr<fmt::memory_buffer> msg) {
            // TODO: implement encryption
            _current_size += msg->size();
            if (_current_size > _max_size)
            {
                _rotate();
                _current_size = msg->size();
            }
            _file_helper.write(*msg);
        }

        spdlog::filename_t RotatingEncryptableSink::calc_filename(std::shared_ptr<api::PathResolver> resolver,
                                                                  const spdlog::filename_t &filename, std::size_t index,
                                                                  const spdlog::filename_t &extension) {
            auto mangledFilename = fmt::format(SPDLOG_FILENAME_T("{}.{}"), filename, extension);
            if (index) {
                mangledFilename = fmt::format(SPDLOG_FILENAME_T("{}.{}.{}"), filename, index, extension);
            }

#if defined(_WIN32) || defined(_WIN64)
            std::string narrowFilename;
            ToNarrow(mangledFilename, narrowFilename);
            auto narrowFilePath = resolver->resolveLogFilePath(narrowFilename);
            std::wstring wideFilePath;
            ToWide(narrowFilePath, wideFilePath);
            return wideFilePath;
#else
            return resolver->resolveLogFilePath(mangledFilename);
#endif
        }

#if defined(_WIN32) || defined(_WIN64)
        void RotatingEncryptableSink::ToWide(const std::string &input, std::wstring &output) {
            wchar_t buffer[MAX_PATH];
	        MultiByteToWideChar(CP_UTF8, 0, input.c_str(), -1, buffer, MAX_PATH);
	        output = buffer;
        }

        void RotatingEncryptableSink::ToNarrow(const std::wstring &input, std::string &output) {
            char buffer[MAX_PATH];
	        WideCharToMultiByte(CP_UTF8, 0, input.c_str(), -1, buffer, MAX_PATH, NULL, NULL);
	        output = buffer;
        }
#endif

        void RotatingEncryptableSink::_rotate() {
            auto resolver = _resolver.lock();
            if (!resolver) {
                throw make_exception(api::ErrorCode::NULL_POINTER, "Resolver was released.");
            }
            using spdlog::details::os::filename_to_str;
            _file_helper.close();
            for (auto i = _max_files; i > 0; --i)
            {
                auto src = calc_filename(resolver, _base_filename, i - 1, _extension);
                auto target = calc_filename(resolver, _base_filename, i, _extension);

                if (spdlog::details::file_helper::file_exists(target))
                {
                    if (spdlog::details::os::remove(target) != 0)
                    {
                        throw spdlog::spdlog_ex("rotating_file_sink: failed removing " + spdlog::details::os::filename_to_str(target), errno);
                    }
                }
                if (spdlog::details::file_helper::file_exists(src) && spdlog::details::os::rename(src, target))
                {
                    throw spdlog::spdlog_ex("rotating_file_sink: failed renaming " + spdlog::details::os::filename_to_str(src) + " to " + spdlog::details::os::filename_to_str(target), errno);
                }
            }
            _file_helper.reopen(true);
        }
    }
}
