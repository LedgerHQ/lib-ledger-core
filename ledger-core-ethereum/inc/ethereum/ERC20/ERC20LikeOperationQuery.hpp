/*
 *
 * ERC20LikeAccount
 *
 * Created by Alexis Le Provost on 08/10/2019.
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

#include <soci.h>

#include <core/operation/OperationQuery.hpp>
#include <ethereum/ERC20/ERC20LikeOperation.hpp>

namespace ledger {
    namespace core {
        class ERC20LikeOperationQuery : public OperationQuery<ERC20LikeOperation> {
        public:
            ERC20LikeOperationQuery(
                const std::shared_ptr<api::QueryFilter>& headFilter,
                const std::shared_ptr<DatabaseSessionPool>& pool,
                const std::shared_ptr<api::ExecutionContext>& context,
                const std::shared_ptr<api::ExecutionContext>& mainContext);

        protected:
            soci::rowset<soci::row> performExecute(soci::session &sql) override;

            void inflateCompleteTransaction(
                soci::session &sql,
                const std::string &accountUid,
                ERC20LikeOperation& operation
            ) override;

            std::shared_ptr<ERC20LikeOperation> createOperation(
                std::shared_ptr<AbstractAccount> &account
            ) override;
        };
    }
}
