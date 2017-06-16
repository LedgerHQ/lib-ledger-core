/*
 *
 * wait
 * ledger-core
 *
 * Created by Pierre Pollastri on 13/06/2017.
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
#ifndef LEDGER_CORE_WAIT_H
#define LEDGER_CORE_WAIT_H

#include <async/Future.hpp>
#include <QEventLoop>
#include "QtThreadPoolExecutionContext.hpp"

template <typename T>
T wait(ledger::core::Future<T> future) {
    auto signaler = std::make_shared<ledger::qt::QtThreadPoolExecutionContext>(1);
    QEventLoop looper;

    future.onComplete(signaler, [&] (const ledger::core::Try<T>& result) {
       looper.quit();
    });
    looper.exec();
    ledger::core::Try<T> result = future.getValue().getValue();
    if (result.isFailure()) {
        throw result.getFailure();
    }
    return result.getValue();
};

#endif //LEDGER_CORE_WAIT_H
