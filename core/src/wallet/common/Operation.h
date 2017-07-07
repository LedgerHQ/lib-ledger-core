
/*
 *
 * Operation
 * ledger-core
 *
 * Created by Pierre Pollastri on 07/06/2017.
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
#ifndef LEDGER_CORE_OPERATION_H
#define LEDGER_CORE_OPERATION_H

#include <string>
#include <api/WalletType.hpp>
#include <chrono>
#include <vector>
#include <math/BigInt.h>
#include <utils/Option.hpp>
#include "TrustIndicator.h"
#include <memory>
#include <wallet/bitcoin/explorers/BitcoinLikeBlockchainExplorer.hpp>
#include <api/OperationType.hpp>
#include <api/Operation.hpp>

namespace ledger {
    namespace core {
        struct Operation {
            std::string uid;
            std::string accountUid;
            std::string walletUid;
            api::WalletType walletType;
            std::chrono::system_clock::time_point date;
            std::vector<std::string> senders;
            std::vector<std::string> recipients;
            BigInt amount;
            Option<BigInt> fees;
            Option<uint64_t> blockHeight;
            std::string currencyName;
            api::OperationType type;
            std::shared_ptr<TrustIndicator> trust;
            Option<BitcoinLikeBlockchainExplorer::Transaction> bitcoinTransaction;

            Operation() {};
            void refreshUid();
        private:

        };
    }

}


#endif //LEDGER_CORE_OPERATION_H
