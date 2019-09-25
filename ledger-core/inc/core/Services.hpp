/*
 *
 * Services
 * ledger-core
 *
 * Created by Dimitri Sabadie on 2019/08/20.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2019 Ledger
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

#include <memory>
#include <unordered_map>
#include <spdlog/spdlog.h>

#include <core/api/ErrorCode.hpp>
#include <core/api/EventBus.hpp>
#include <core/api/PathResolver.hpp>
#include <core/api/RandomNumberGenerator.hpp>
#include <core/api/ThreadDispatcher.hpp>
#include <core/async/DedicatedContext.hpp>
#include <core/collections/DynamicObject.hpp>
#include <core/events/EventPublisher.hpp>
#include <core/database/DatabaseSessionPool.hpp>
#include <core/net/HttpClient.hpp>
#include <core/net/WebSocketClient.hpp>
#include <core/preferences/Preferences.hpp>

namespace ledger {
    namespace core {
        /// The Services ledger-core object.
        ///
        /// That object is used to store all meta data and objects related to ledger-core, such as:
        ///
        ///   - Configuration password.
        ///   - Dynamic configuration.
        ///   - User preferences.
        ///   - HTTP and websocket clients.
        ///   - Path resolver.
        ///   - Etc. etc.
        ///
        /// It’s typical that those objects don’t have to live more than once in memory so it’s better to
        /// gather them (legacy code was using the WalletPool for that).
        struct Services : DedicatedContext, std::enable_shared_from_this<Services> {
            Services() = delete;
            Services(
                const std::string &name,
                const std::string &password,
                const std::shared_ptr<api::HttpClient> &httpClient,
                const std::shared_ptr<api::WebSocketClient> &webSocketClient,
                const std::shared_ptr<api::PathResolver> &pathResolver,
                const std::shared_ptr<api::LogPrinter> &logPrinter,
                const std::shared_ptr<api::ThreadDispatcher> &dispatcher,
                const std::shared_ptr<api::RandomNumberGenerator> &rng,
                const std::shared_ptr<api::DatabaseBackend> &backend,
                const std::shared_ptr<api::DynamicObject> &configuration
            );
            ~Services() = default;

            /// Create a new meta configuration.
            ///
            /// This function is semantically exactly the same as the parametered Services constructor.
            static std::shared_ptr<Services> newInstance(
                const std::string &name,
                const std::string &password,
                const std::shared_ptr<api::HttpClient> &httpClient,
                const std::shared_ptr<api::WebSocketClient> &webSocketClient,
                const std::shared_ptr<api::PathResolver> &pathResolver,
                const std::shared_ptr<api::LogPrinter> &logPrinter,
                const std::shared_ptr<api::ThreadDispatcher> &dispatcher,
                const std::shared_ptr<api::RandomNumberGenerator>& rng,
                const std::shared_ptr<api::DatabaseBackend> &backend,
                const std::shared_ptr<api::DynamicObject>& configuration
            );

            // Password management
            Future<api::ErrorCode> changePassword(
                const std::string& oldPassword,
                const std::string& newPassword
            );

            std::shared_ptr<HttpClient> getHttpClient(const std::string& baseUrl);
            std::shared_ptr<WebSocketClient> getWebSocketClient() const;
            std::shared_ptr<Preferences> getExternalPreferences() const;
            std::shared_ptr<Preferences> getInternalPreferences() const;
            std::shared_ptr<api::PathResolver> getPathResolver() const;
            std::shared_ptr<api::RandomNumberGenerator> rng() const;
            std::shared_ptr<api::ThreadDispatcher> getDispatcher() const;
            std::shared_ptr<spdlog::logger> logger() const;
            std::shared_ptr<DatabaseSessionPool> getDatabaseSessionPool() const;
            std::shared_ptr<DynamicObject> getConfiguration() const;
            std::shared_ptr<api::EventBus> getEventBus() const;
            const std::string& getName() const;
            const std::string getPassword() const;

        private:
            // General
            std::string _poolName;
            std::string _password;
            std::shared_ptr<DynamicObject> _configuration;

            // File system management
            std::shared_ptr<api::PathResolver> _pathResolver;

            // HTTP management
            std::shared_ptr<api::HttpClient> _httpEngine;
            std::unordered_map<std::string, std::weak_ptr<HttpClient>> _httpClients;

            // WS management
            std::shared_ptr<WebSocketClient> _wsClient;

            // Preferences management
            std::shared_ptr<PreferencesBackend> _externalPreferencesBackend;
            std::shared_ptr<PreferencesBackend> _internalPreferencesBackend;

            // Database management
            std::shared_ptr<DatabaseSessionPool> _database;

            // Logger
            std::shared_ptr<spdlog::logger> _logger;
            std::shared_ptr<api::LogPrinter> _logPrinter;

            // Threading management
            std::shared_ptr<api::ThreadDispatcher> _threadDispatcher;

            // RNG management
            std::shared_ptr<api::RandomNumberGenerator> _rng;

            // Event publisher
            std::shared_ptr<EventPublisher> _publisher;
        };
    }
}
