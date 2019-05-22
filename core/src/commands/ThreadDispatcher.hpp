#pragma once
#include "api/ThreadDispatcher.hpp"
#include "api/Lock.hpp"
#include <mutex>

namespace ledger { namespace core {
    class Lock :public ledger::core::api::Lock {
    public:
        virtual ~Lock() {}

        /**
         *Acquire lock by thread calling this method,
         *If Lock already acquired by another thread, execution of calling thread should be blocked
         *until the other thread call the unlock method
         */
        void lock() override {
            _mutex.lock();
        }

        /**
         *Try to acquire lock
         *If Lock already aquired by another thread, method returns false for calling thread
         *without blocking its execution
         *@return bool, return true if Lock acquire by calling thread, false otherwise
         */
        bool tryLock() override {
            return _mutex.try_lock();
        }

        /**Release Lock ownership by calling thread */
        void unlock() override {
            _mutex.unlock();
        }
    private:
        std::mutex _mutex;
    };


    class ThreadDispatcher :public ledger::core::api::ThreadDispatcher {
    public:
        ThreadDispatcher(std::shared_ptr<ledger::core::api::ExecutionContext> context) : _context(context) {};

        std::shared_ptr<ledger::core::api::ExecutionContext> getSerialExecutionContext(const std::string& name) override {
            return _context;
        }

        /**
         *Get an execution context where tasks are executed in parallel thanks to a thread pool
         *where a system of inter-thread communication was designed
         *@param name, string, name of execution context to retrieve
         *@return ExecutionContext object
         */
        std::shared_ptr<ledger::core::api::ExecutionContext> getThreadPoolExecutionContext(const std::string& name) override {
            return _context;
        }

        /**
         *Get main execution context (generally where tasks that should never get blocked are executed)
         *@return ExecutionContext object
         */
        std::shared_ptr<ledger::core::api::ExecutionContext> getMainExecutionContext() override {
            return _context;
        }

        /**
         *Get lock to handle multithreading
         *@return Lock object
         */
        std::shared_ptr<ledger::core::api::Lock> newLock() override {
            return std::make_shared<Lock>();
        }
    private:
        std::shared_ptr<ledger::core::api::ExecutionContext> _context;
    };
} }