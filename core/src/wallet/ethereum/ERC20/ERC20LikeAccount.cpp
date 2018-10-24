/*
 *
 * ERC20LikeAccount
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


#include "ERC20LikeAccount.h"
#include <math/BigInt.h>
#include <api_impl/BigIntImpl.hpp>
#include <api/Amount.hpp>
#include <api/OperationType.hpp>
#include <bytes/RLP/RLPStringEncoder.h>
#include <utils/hex.h>
#include <math/BigInt.h>
#include <api/Address.hpp>
namespace ledger {
    namespace core {
        ERC20LikeAccount::ERC20LikeAccount(const api::ERC20Token &erc20Token,
                                           const std::string &accountAddress,
                                           const api::Currency &parentCurrency):
                                            _token(erc20Token),
                                            _accountAddress(accountAddress),
                                            _parentCurrency(parentCurrency) {

        }

        api::ERC20Token ERC20LikeAccount::getToken() {
            return _token;
        }

        std::string ERC20LikeAccount::getAddress() {
            return _accountAddress;
        }

        std::shared_ptr<api::BigInt> ERC20LikeAccount::getBalance() {
            auto result = BigInt::ZERO;
            for (auto &op : _operations) {
                if (op->getOperationType() == api::OperationType::RECEIVE) {
                    result = result + BigInt(op->getValue()->toString());
                } else if (op->getOperationType() == api::OperationType::SEND){
                    result = result - BigInt(op->getValue()->toString());
                }

            }
            return std::make_shared<api::BigIntImpl>(result);
        }

        std::vector<std::shared_ptr<api::ERC20LikeOperation>>
        ERC20LikeAccount::getOperations() {
            return _operations;
        }

        std::vector<uint8_t> ERC20LikeAccount::getTransferToAddressData(const std::shared_ptr<api::Amount> & amount,
                                                                        const std::string & address) {
            if (! api::Address::isValid(address, _parentCurrency)) {
                throw Exception(api::ErrorCode::INVALID_EIP55_FORMAT, "Invalid address : Invalid EIP55 format");
            }

            RLPStringEncoder transferEncoder("transfer");
            transferEncoder.append(hex::toByteArray(address.substr(2,address.size() - 2)));
            BigInt bigAmount(amount->toString());
            transferEncoder.append(hex::toByteArray(bigAmount.toHexString()));
            return transferEncoder.encode();
        }

        void ERC20LikeAccount::putOperation(const std::shared_ptr<api::ERC20LikeOperation> &operation) {
            _operations.push_back(operation);
        }


    }
}