/*
 *
 * QtHttpClient
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/04/2017.
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

#pragma once

#include <QtNetwork>
#include <QNetworkReply>
#include <QObject>
#include <functional>

#include <core/api/HttpClient.hpp>
#include <core/api/HttpUrlConnection.hpp>
#include <core/api/HttpRequest.hpp>
#include <core/api/HttpReadBodyResult.hpp>
#include <core/api/HttpMethod.hpp>

#include "../async/QtThreadDispatcher.hpp"

namespace ledger {
    namespace qt {
        class QtHttpClient : public QObject, public core::api::HttpClient {
            Q_OBJECT
        public:
            QtHttpClient(const std::shared_ptr<core::api::ExecutionContext>& context) : _context(context), _manager(this) {};
            void execute(const std::shared_ptr<core::api::HttpRequest> &request) override;

        private:
            QNetworkAccessManager _manager;
            std::shared_ptr<core::api::ExecutionContext> _context;
        };
    }
}
