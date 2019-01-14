/*
 *
 * migrations
 * ledger-core
 *
 * Created by Pierre Pollastri on 25/04/2017.
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
#ifndef LEDGER_CORE_MIGRATIONS_HPP
#define LEDGER_CORE_MIGRATIONS_HPP

#include <soci.h>
#include <iostream>
#include <src/utils/Exception.hpp>

namespace ledger {
    namespace core {
        template <int migrationNumber>
        void migrate(soci::session& sql) {
            std::cerr << "No specified migration for version " << migrationNumber << std::endl;
            throw make_exception(api::ErrorCode::RUNTIME_ERROR, "No specified migration for version {}", migrationNumber);
        }

        template <int version>
        bool migrate(soci::session& sql, int currentVersion) {
            bool previousResult = migrate<version - 1>(sql, currentVersion);
            if (currentVersion < version) {
                migrate<version>(sql);
                sql << "UPDATE __database_meta__ SET version = :version", soci::use(version);
                return true;
            }

            return previousResult;
        };

        template <> bool migrate<-1>(soci::session& sql, int currentVersion);

        // Migrations
        template <> void migrate<0>(soci::session& sql);
        template <> void migrate<1>(soci::session& sql);
        template <> void migrate<2>(soci::session& sql);
        template <> void migrate<3>(soci::session& sql);
        template <> void migrate<4>(soci::session& sql);
        template <> void migrate<5>(soci::session& sql);
    }
}


#endif //LEDGER_CORE_MIGRATIONS_HPP
