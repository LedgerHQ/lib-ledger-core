/*
 *
 * RippleLikeBlockchainExplorer
 *
 * Created by El Khalil Bellakrid on 06/01/2019.
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


#ifndef LEDGER_CORE_RIPPLELIKEBLOCKCHAINEXPLORER_H
#define LEDGER_CORE_RIPPLELIKEBLOCKCHAINEXPLORER_H


#include <string>

#include <api/DynamicObject.hpp>
#include <api/ExecutionContext.hpp>
#include <api/RippleLikeNetworkParameters.hpp>
#include <api/RippleLikeMemo.hpp>
#include <async/DedicatedContext.hpp>
#include <collections/DynamicObject.hpp>
#include <math/BigInt.h>
#include <net/HttpClient.hpp>
#include <utils/ConfigurationMatchable.h>
#include <utils/Option.hpp>
#include <wallet/common/Block.h>
#include <wallet/common/explorers/AbstractBlockchainExplorer.h>
#include <wallet/ripple/keychains/RippleLikeKeychain.h>

namespace ledger {
    namespace core {

        struct RippleLikeBlockchainExplorerTransaction {
            std::string hash;
            std::chrono::system_clock::time_point receivedAt;
            BigInt value;
            BigInt fees;
            BigInt sequence;
            std::string receiver;
            std::string sender;
            Option<Block> block;
            uint64_t confirmations;
            Option<uint64_t> destinationTag;
            std::vector<api::RippleLikeMemo> memos;
            int32_t status;

            RippleLikeBlockchainExplorerTransaction(): confirmations(0) {
            }

            RippleLikeBlockchainExplorerTransaction(
                RippleLikeBlockchainExplorerTransaction const& cpy
            ) = default;
        };

        class RippleLikeBlockchainExplorer : public ConfigurationMatchable,
                                             public AbstractBlockchainExplorer<RippleLikeBlockchainExplorerTransaction> {
        public:
            typedef ledger::core::Block Block;
            using Transaction = RippleLikeBlockchainExplorerTransaction;

            RippleLikeBlockchainExplorer(const std::shared_ptr<ledger::core::api::DynamicObject> &configuration,
                                         const std::vector<std::string> &matchableKeys);

            virtual Future<std::shared_ptr<BigInt>>
            getBalance(const std::vector<RippleLikeKeychain::Address> &addresses) = 0;

            virtual Future<std::shared_ptr<BigInt>>
            getSequence(const std::string &address) = 0;

            virtual Future<std::shared_ptr<BigInt>>
            getFees() = 0;

            virtual Future<std::shared_ptr<BigInt>>
            getBaseReserve() = 0;

            virtual Future<std::shared_ptr<BigInt>>
            getLedgerSequence() = 0;
        };
    }
}


#endif //LEDGER_CORE_RIPPLELIKEBLOCKCHAINEXPLORER_H
