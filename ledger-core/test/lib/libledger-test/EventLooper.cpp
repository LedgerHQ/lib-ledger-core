/*
 *
 * EventLooper
 * ledger-core
 *
 * Created by Pierre Pollastri on 21/11/2016.
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
#include "EventLooper.hpp"
#include <iostream>
#include <chrono>
#include <thread>

static const long RUN_LOOP_FREQUENCY_MS = 1;
using namespace std::chrono;

static long getTimeMillis() {
    milliseconds ms = duration_cast< milliseconds >(
            system_clock::now().time_since_epoch()
    );
    return ms.count();
}

void EventLooper::run() {
    _mutex.lock();
    _isRunning = true;
    _mutex.unlock();
    while (isRunning()) {
        auto start = getTimeMillis();
        _mutex.lock();
        std::vector<std::pair<Event *, long>> events = _events;
        _events =  std::vector<std::pair<Event *, long>>();
        for (auto& eventPair : events) {
            if (eventPair.second <= start) {
                eventPair.first->run();
                delete eventPair.first;
            } else {
                _events.push_back(eventPair);
            }
        }
        _mutex.unlock();
        auto end = getTimeMillis();
        std::this_thread::sleep_for(std::chrono::milliseconds((RUN_LOOP_FREQUENCY_MS - (end - start))));
    }
}

void EventLooper::push_back(Event *event) {
   push_back(event, 0);
}

void EventLooper::stop() {
    _mutex.lock();
    _isRunning = false;
    _mutex.unlock();
}

void EventLooper::push_back(Event *event, long delay) {
    _mutex.lock();
    long dueTime = delay + getTimeMillis();
    _events.push_back(std::pair<Event *, long>(event, dueTime));
    _mutex.unlock();
}

bool EventLooper::isRunning() {
    _mutex.lock();
    auto isRunning = _isRunning;
    _mutex.unlock();
    return isRunning;
}

void LambdaEvent::run() {
    _run();
}

LambdaEvent::LambdaEvent(std::function<void()> lambda) {
    _run = lambda;
}
