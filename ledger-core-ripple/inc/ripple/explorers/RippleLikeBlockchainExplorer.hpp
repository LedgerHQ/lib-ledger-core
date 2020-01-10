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


#pragma once

#include <string>

#include <core/api/DynamicObject.hpp>
#include <core/api/ExecutionContext.hpp>
#include <core/async/DedicatedContext.hpp>
#include <core/collections/DynamicObject.hpp>
#include <core/math/BigInt.hpp>
#include <core/net/HttpClient.hpp>
#include <core/utils/ConfigurationMatchable.hpp>
#include <core/utils/Option.hpp>
#include <core/api/Block.hpp>
#include <core/explorers/AbstractBlockchainExplorer.hpp>

#include <ripple/api/RippleLikeNetworkParameters.hpp>
#include <ripple/api/RippleLikeMemo.hpp>
#include <ripple/keychains/RippleLikeKeychain.hpp>

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
            Option<api::Block> block;
            uint64_t confirmations;
            Option<uint64_t> destinationTag;

            std::vector<api::RippleLikeMemo> memos;

            RippleLikeBlockchainExplorerTransaction() {
                confirmations = 0;
            }

            RippleLikeBlockchainExplorerTransaction(const RippleLikeBlockchainExplorerTransaction &cpy) {
                this->block = cpy.block;
                this->hash = cpy.hash;
                this->receivedAt = cpy.receivedAt;
                this->confirmations = cpy.confirmations;
                this->fees = cpy.fees;
                this->sequence = cpy.sequence;
                this->receiver = cpy.receiver;
                this->sender = cpy.sender;
                this->value = cpy.value;
                this->destinationTag = cpy.destinationTag;
                this->memos = cpy.memos;
            }

        };

        class RippleLikeBlockchainExplorer : public ConfigurationMatchable,
                                             public AbstractBlockchainExplorer<RippleLikeBlockchainExplorerTransaction> {
        public:
            typedef api::Block Block;

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
