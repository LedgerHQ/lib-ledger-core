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
#ifndef LEDGER_CORE_QTHTTPCLIENT_HPP
#define LEDGER_CORE_QTHTTPCLIENT_HPP

#include <functional>

#include <api/HttpClient.hpp>
#include <api/HttpUrlConnection.hpp>
#include <api/HttpRequest.hpp>
#include <api/HttpReadBodyResult.hpp>
#include <api/HttpMethod.hpp>
#include <QtNetwork>
#include <QNetworkReply>
#include <QObject>
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

#endif //LEDGER_CORE_QTHTTPCLIENT_HPP
