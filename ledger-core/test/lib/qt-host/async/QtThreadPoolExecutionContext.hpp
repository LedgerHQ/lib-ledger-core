/*
 *
 * QtThreadPoolExecutionContext
 * ledger-core
 *
 * Created by Pierre Pollastri on 20/04/2017.
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

#include <QtConcurrent>
#include <QThreadPool>
#include <QTimer>

#include <core/api/ExecutionContext.hpp>
#include <core/api/Runnable.hpp>

namespace ledger {
    namespace qt {
        class QtThreadPoolExecutionContext : public QObject, public core::api::ExecutionContext {
            Q_OBJECT
        public:
            QtThreadPoolExecutionContext(int maxThreads, const std::shared_ptr<core::api::ExecutionContext>& main);
            void execute(const std::shared_ptr<core::api::Runnable> &runnable) override;
            void delay(const std::shared_ptr<core::api::Runnable> &runnable, int64_t millis) override;

        private slots:
            void performDelayedCall(std::shared_ptr<ledger::core::api::Runnable> runnable);

        private:
            QThreadPool _pool;
            std::shared_ptr<core::api::ExecutionContext> _main;
        };
    }
}
