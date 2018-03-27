/*
 *
 * QtMainExecutionContext
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
#include "QtMainExecutionContext.hpp"
#include "QtThreadDispatcher.hpp"
#include "QMetaType"

void ledger::qt::QtMainExecutionContext::execute(const std::shared_ptr<ledger::core::api::Runnable> &runnable) {
    emit postRunnable(runnable);
}

void ledger::qt::QtMainExecutionContext::delay(const std::shared_ptr<ledger::core::api::Runnable> &runnable,
                                               int64_t millis) {
    auto self = this;
    QTimer::singleShot((int) millis, this, [=] () -> void {
        self->performExecute(runnable);
    });
}

ledger::qt::QtMainExecutionContext::QtMainExecutionContext(const std::shared_ptr<QCoreApplication> &app) {
    _app = app;
    qRegisterMetaType<std::shared_ptr<core::api::Runnable>>();
    qRegisterMetaType<std::shared_ptr<core::api::Runnable>>("std::shared_ptr<core::api::Runnable>");
    connect(this, &QtMainExecutionContext::postRunnable, this, &QtMainExecutionContext::performExecute);
}

ledger::qt::QtMainExecutionContext::QtMainExecutionContext() : QtMainExecutionContext(0, NULL) {

}

ledger::qt::QtMainExecutionContext::QtMainExecutionContext(int argc, char **argv)
: QtMainExecutionContext(std::make_shared<QCoreApplication>(argc, argv)) {
    _isRunning = true;
}

int ledger::qt::QtMainExecutionContext::run() {
    if (!_isRunning.load())
        return 0;
    return _app->exec();
}

void ledger::qt::QtMainExecutionContext::exit() {
    _isRunning = false;
    auto app = _app;
    execute(make_runnable([app] {
        app->exit(0);
    }));
}

void ledger::qt::QtMainExecutionContext::performExecute(std::shared_ptr<ledger::core::api::Runnable> runnable) {
    runnable->run();
}
