/*
 *
 * QtThreadDispatcher
 * ledger-core
 *
 * Created by Pierre Pollastri on 19/04/2017.
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
#ifndef LEDGER_CORE_QTTHREADDISPATCHER_HPP
#define LEDGER_CORE_QTTHREADDISPATCHER_HPP

#include <QObject>
#include <QMap>
#include <QMutex>
#include <src/api/ThreadDispatcher.hpp>
#include <src/api/Runnable.hpp>
#include <QCoreApplication>
#include <src/api/ExecutionContext.hpp>

namespace ledger {
    namespace qt {
        class QtThreadDispatcher : public core::api::ThreadDispatcher {
        public:
            QtThreadDispatcher() : QtThreadDispatcher(nullptr) {};
            QtThreadDispatcher(std::shared_ptr<QCoreApplication> app);
            std::shared_ptr<core::api::ExecutionContext> getSerialExecutionContext(const std::string &name) override;
            std::shared_ptr<core::api::ExecutionContext> getThreadPoolExecutionContext(const std::string &name) override;
            std::shared_ptr<core::api::ExecutionContext> getMainExecutionContext() override;
            std::shared_ptr<core::api::Lock> newLock() override;
            void setMaximumThreadPoolThreads(int num);
            void stop();
            int waitUntilStopped();

        private:
            QMutex _mutex;
            int _maxThreads;
            QMap<std::string, std::shared_ptr<core::api::ExecutionContext>> _contexts;
        };

        std::shared_ptr<ledger::core::api::Runnable> make_runnable(std::function<void()> func);
    }
}


#endif //LEDGER_CORE_QTTHREADDISPATCHER_HPP
