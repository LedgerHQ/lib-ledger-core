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

#include <src/api/HttpClient.hpp>
#include <src/api/HttpUrlConnection.hpp>
#include <src/api/HttpRequest.hpp>
#include <src/api/HttpReadBodyResult.hpp>
#include <src/api/HttpMethod.hpp>
#include <QtNetwork>
#include <QObject>

namespace ledger {
    namespace qt {
        class QtHttpClient : public QObject, public core::api::HttpClient {
            Q_OBJECT
        public:
            QtHttpClient() : _manager(this) {};
            void execute(const std::shared_ptr<core::api::HttpRequest> &request) override;

        private:
            QNetworkAccessManager _manager;
        };
    }
}

#endif //LEDGER_CORE_QTHTTPCLIENT_HPP
