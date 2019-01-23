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
#include "erc20Tokens.h"
#include <wallet/common/OperationQuery.h>
#include <soci.h>
#include <database/soci-date.h>
#include <database/query/ConditionQueryFilter.h>
using namespace soci;

namespace ledger {
    namespace core {
        ERC20LikeAccount::ERC20LikeAccount(const std::string &accountUid,
                                           const api::ERC20Token &erc20Token,
                                           const std::string &accountAddress,
                                           const api::Currency &parentCurrency,
                                           const std::shared_ptr<EthereumLikeAccount> &parentAccount):
                                            _accountUid(accountUid),
                                            _token(erc20Token),
                                            _accountAddress(accountAddress),
                                            _parentCurrency(parentCurrency),
                                            _account(parentAccount)

        {}

        api::ERC20Token ERC20LikeAccount::getToken() {
            return _token;
        }

        std::string ERC20LikeAccount::getAddress() {

            return _accountAddress;
        }

        std::shared_ptr<api::BigInt> ERC20LikeAccount::getBalance() {
            auto result = BigInt::ZERO;
            auto operations = getOperations();
            for (auto &op : operations) {
                if (op->getOperationType() == api::OperationType::RECEIVE) {
                    result = result + BigInt(op->getValue()->toString(10));
                } else if (op->getOperationType() == api::OperationType::SEND){
                    result = result - BigInt(op->getValue()->toString(10));
                }

            }
            return std::make_shared<api::BigIntImpl>(result);
        }

        std::vector<std::shared_ptr<api::ERC20LikeOperation>>
        ERC20LikeAccount::getOperations() {
            auto localAccount = _account.lock();
            soci::session sql (localAccount->getWallet()->getDatabase()->getPool());
            soci::rowset<soci::row> rows = (sql.prepare << "SELECT op.uid, op.ethereum_operation_uid, op.account_uid,"
                    " op.type, op.hash, op.nonce, op.value,"
                    " op.date, op.sender, op.receiver, op.input_data,"
                    " op.gas_price, op.gas_limit, op.gas_used, op.status"
                    " FROM erc20_operations AS op"
                    " WHERE op.account_uid = :account_uid", soci::use(_accountUid));
            std::vector<std::shared_ptr<api::ERC20LikeOperation>> result;
            for (auto& row : rows) {
                auto op = std::make_shared<ERC20LikeOperation>();
                op->setOperationUid(row.get<std::string>(0));
                op->setETHOperationUid(row.get<std::string>(1));
                op->setOperationType(api::from_string<api::OperationType>(row.get<std::string>(3)));
                op->setHash(row.get<std::string>(4));
                op->setNonce(BigInt::fromHex(row.get<std::string>(5)));
                op->setValue(BigInt::fromHex(row.get<std::string>(6)));
                op->setTime(DateUtils::fromJSON(row.get<std::string>(7)));
                op->setSender(row.get<std::string>(8));
                op->setReceiver(row.get<std::string>(9));
                if (row.get_indicator(10) != soci::i_null) {
                    auto data = row.get<std::string>(10);
                    auto dataArray = hex::toByteArray(data);
                    op->setData(dataArray);
                }

                op->setGasPrice(BigInt::fromHex(row.get<std::string>(11)));
                op->setGasLimit(BigInt::fromHex(row.get<std::string>(12)));
                op->setUsedGas(BigInt::fromHex(row.get<std::string>(13)));
                auto status = row.get<int32_t>(14);
                op->setStatus(status);
                result.emplace_back(op);
            }
            return result;
        }

        std::vector<uint8_t> ERC20LikeAccount::getTransferToAddressData(const std::shared_ptr<api::BigInt> &amount,
                                                                        const std::string & address) {
            if (! api::Address::isValid(address, _parentCurrency)) {
                throw Exception(api::ErrorCode::INVALID_EIP55_FORMAT, "Invalid address : Invalid EIP55 format");
            }
            auto balance = getBalance();
            if ( amount->compare(balance) > 0) {
                throw Exception(api::ErrorCode::NOT_ENOUGH_FUNDS, "Cannot gather enough funds.");
            }

            BytesWriter writer;
            writer.writeByteArray(hex::toByteArray(erc20Tokens::ERC20MethodsID.at("transfer")));

            auto toUint256Format = [=](const std::string &hexInput) -> std::string {
                if (hexInput.size() > 64) {
                    throw Exception(api::ErrorCode::INVALID_ARGUMENT, "Invalid argument passed to toUint256Format");
                }
                auto hexOutput = hexInput;
                while (hexOutput.size() != 64) {
                    hexOutput = "00" + hexOutput;
                }

                return hexOutput;
            };

            writer.writeByteArray(hex::toByteArray(toUint256Format(address.substr(2,address.size() - 2))));

            BigInt bigAmount(amount->toString(10));
            writer.writeByteArray(hex::toByteArray(toUint256Format(bigAmount.toHexString())));

            return writer.toByteArray();
        }

        void ERC20LikeAccount::putOperation(soci::session &sql, const std::shared_ptr<ERC20LikeOperation> &operation, bool newOperation) {
            if (newOperation) {
                auto erc20OpUid = operation->getOperationUid();
                auto ethOpUid = operation->getETHOperationUid();
                auto hash = operation->getHash();
                auto receiver = operation->getReceiver();
                auto sender = operation->getSender();
                auto data = hex::toString(operation->getData());
                auto status = operation->getStatus();
                sql << "INSERT INTO erc20_operations VALUES("
                        ":uid, :eth_op_uid, :accout_uid, :op_type, :hash, :nonce, :value, :date, :sender,"
                        ":receiver, :data, :gas_price, :gas_limit, :gas_used, :status"
                        ")"
                        , use(erc20OpUid), use(ethOpUid)
                        , use(_accountUid), use(api::to_string(operation->getOperationType())), use(hash)
                        , use(operation->getNonce()->toString(16)), use(operation->getValue()->toString(16)), use(operation->getTime())
                        , use(sender), use(receiver), use(data)
                        , use(operation->getGasPrice()->toString(16)), use(operation->getGasLimit()->toString(16)), use(operation->getUsedGas()->toString(16))
                        , use(status);
            }
        }

        std::shared_ptr<api::OperationQuery> ERC20LikeAccount::queryOperations() {
            auto localAccount = _account.lock();
            auto accountUid = localAccount->getAccountUid();
            auto filter = std::make_shared<ConditionQueryFilter<std::string>>("account_uid", "=", _accountUid, "e");
            auto query = std::make_shared<ERC20OperationQuery>(
                    filter,
                    localAccount->getWallet()->getDatabase(),
                    localAccount->getWallet()->getContext(),
                    localAccount->getWallet()->getMainExecutionContext()
            );
            query->registerAccount(localAccount);
            return query;
        }

    }
}