#pragma once
#include "api/LogPrinter.hpp"
#include "api/ExecutionContext.hpp"
#include <iostream>

namespace ledger {
    namespace core {
        class LogPrinter : public ledger::core::api::LogPrinter {
        public:
            LogPrinter(std::shared_ptr<ledger::core::api::ExecutionContext> context) : _context(context) {

            }
            void printError(const std::string& message) override {
                std::cout << "ERROR: " << message << std::endl;
            }

            /**
             * Print useful information messages.
             * @param message, string
             */
            virtual void printInfo(const std::string& message) override {
                std::cout << "INFO: " << message << std::endl;
            }

            /**
             * Print debug messages.
             * @param message string
             */
            virtual void printDebug(const std::string& message) override {
                std::cout << "DEBUG: " << message << std::endl;
            }

            /**
             * Print warning messages.
             * @param message, string
             */
            virtual void printWarning(const std::string& message) override {
                std::cout << "WARN: " << message << std::endl;
            }

            /**
             * Print messages from APDU comand interpretation loop.
             * @param message, string
             */
            virtual void printApdu(const std::string& message) override {
                std::cout << "APDU: " << message << std::endl;
            }

            /**
             * Print critical errors causing a core dump or error from which recovery is impossible.
             * @param message, string
             */
            virtual void printCriticalError(const std::string& message) override {
                std::cout << "CRIT: " << message << std::endl;
            }

            /**
             * Get context in which printer is executed (print).
             * @return ExecutionContext object
             */
            std::shared_ptr<ledger::core::api::ExecutionContext> getContext() override {
                return _context;
            }
        private:
            std::shared_ptr<ledger::core::api::ExecutionContext> _context;
        };
    }
}
