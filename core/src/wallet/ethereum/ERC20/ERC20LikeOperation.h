/*
 *
 * ERC20LikeTransaction
 *
 * Created by El Khalil Bellakrid on 26/08/2018.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2018 Ledger
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


#ifndef LEDGER_CORE_ERC20LIKETRANSACTION_H
#define LEDGER_CORE_ERC20LIKETRANSACTION_H

#include <api/ERC20LikeOperation.hpp>
#include <api/ERC20Token.hpp>
#include <api/OperationType.hpp>
#include <wallet/common/Operation.h>
namespace ledger {
    namespace core {

        class ERC20LikeOperation : public api::ERC20LikeOperation {
        public:
            ERC20LikeOperation(const std::string &accountAddress,
                               const std::string &operationUid,
                               const Operation &operation,
                               const api::ERC20Token &token,
                               const api::Currency &currency);

            ERC20LikeOperation() {};
            
            std::string getHash() override;
            std::shared_ptr<api::BigInt> getNonce() override;
            std::shared_ptr<api::BigInt> getGasPrice() override;
            std::shared_ptr<api::BigInt> getGasLimit() override;
            std::shared_ptr<api::BigInt> getUsedGas() override;
            std::string getSender() override;
            std::string getReceiver() override;
            std::shared_ptr<api::BigInt> getValue() override ;
            std::vector<uint8_t> getData() override ;
            std::chrono::system_clock::time_point getTime() override;
            api::OperationType getOperationType() override;
            std::string getOperationUid();
            int32_t getStatus() override ;
        private:
            api::ERC20Token _token;
            std::string _uid;
            std::string _hash;
            std::shared_ptr<api::BigInt> _nonce;
            std::shared_ptr<api::BigInt> _gasPrice;
            std::shared_ptr<api::BigInt> _gasLimit;
            std::shared_ptr<api::BigInt> _gasUsed;
            std::string _sender;
            std::string _receiver;
            std::shared_ptr<api::BigInt> _value;
            std::vector<uint8_t> _data;
            std::chrono::system_clock::time_point _time;
            api::OperationType _operationType;
            int32_t _status;
        };

    }
}


#endif //LEDGER_CORE_ERC20LIKETRANSACTION_H
