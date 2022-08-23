/**
 *
 * ledger-core
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2022 Ledger
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

#include <cstdlib>
#include <string>

inline std::string getPostgresUrl() {
    const char *host = getenv("POSTGRES_HOST");
    std::string host_var(host ? host : "localhost");

    const char *port = getenv("POSTGRES_PORT");
    std::string port_var(port ? port : "5432");

    const char *dbname = getenv("POSTGRES_DB");
    std::string dbname_var(dbname ? dbname : "test_db");

    const char *user = getenv("POSTGRES_USER");
    std::string user_var(user ? user : "");

    const char *password = getenv("POSTGRES_PASSWORD");
    std::string password_var(password ? password : "");

    if (!user_var.empty()) {
        if (!password_var.empty()) {
            user_var.append(":").append(password_var);
        }
        user_var.append("@");
    }

    return std::string{"postgres://"} + user_var + host_var + ":" + port_var + "/" + dbname_var;
}