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
#include <api/TimePeriod.hpp>
#include <bytes/RLP/RLPStringEncoder.h>
#include <utils/hex.h>
#include <math/BigInt.h>
#include <api/Address.hpp>
#include <api/ErrorCode.hpp>
#include <utils/Exception.hpp>
#include "erc20Tokens.h"
#include <wallet/common/OperationQuery.h>
#include <wallet/common/BalanceHistory.hpp>
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

        FuturePtr<api::BigInt> ERC20LikeAccount::getBalance() {
            auto parentAccount = _account.lock();
            if (!parentAccount) {
                throw make_exception(api::ErrorCode::NULL_POINTER, "Could not lock parent account.");
            }
            return parentAccount->getERC20Balance(_token.contractAddress);
        }

        void ERC20LikeAccount::getBalance(const std::shared_ptr<api::BigIntCallback> & callback) {
            getBalance().callback(getContext(), callback);
        }

        std::vector<std::shared_ptr<api::BigInt>> ERC20LikeAccount::getBalanceHistoryFor(
            const std::chrono::system_clock::time_point& startDate,
            const std::chrono::system_clock::time_point& endDate,
            api::TimePeriod precision
        ) {
            auto operations = getOperations();
            // manually sort by date
            std::sort(
                operations.begin(),
                operations.end(),
                [](
                    const std::shared_ptr<api::ERC20LikeOperation>& a,
                    const std::shared_ptr<api::ERC20LikeOperation>& b
                ) {
                    return a->getTime() < b->getTime();
                }
            );

            // a small type used to pick implementations used by agnostic::getBalanceHistoryFor.
            struct OperationStrategy {
                static inline std::chrono::system_clock::time_point date(std::shared_ptr<api::ERC20LikeOperation>& op) {
                    return op->getTime();
                }

                static inline std::shared_ptr<api::BigInt> value_constructor(const BigInt& v) {
                    return std::make_shared<api::BigIntImpl>(v);
                }

                static inline void update_balance(std::shared_ptr<api::ERC20LikeOperation>& op, BigInt& sum) {
                    auto value = BigInt(op->getValue()->toString(10));

                    switch (op->getOperationType()) {
                        case api::OperationType::RECEIVE:
                            sum = sum + value;
                            break;

                        case api::OperationType::SEND:
                            sum = sum - value;
                            break;
                        default:
                            break;
                    }
                }
            };

            return agnostic::getBalanceHistoryFor<OperationStrategy, BigInt, api::BigInt>(
                startDate,
                endDate,
                precision,
                operations.cbegin(),
                operations.cend(),
                BigInt()
            );
        }

        BigInt ERC20LikeAccount::accumulateBalanceWithOperation(
            const BigInt& balance,
            api::ERC20LikeOperation& op
        ) {
            auto ty = op.getOperationType();
            auto value = BigInt(op.getValue()->toString(10));

            switch (ty) {
                case api::OperationType::RECEIVE:
                    return balance + value;

                case api::OperationType::SEND:
                    return balance - value;
                default:
                    return balance;
            }
        }

        std::vector<std::shared_ptr<api::ERC20LikeOperation>>
        ERC20LikeAccount::getOperations() {
            auto localAccount = _account.lock();
            if (!localAccount) {
                throw make_exception(api::ErrorCode::NULL_POINTER, "Account was released.");
            }
            soci::session sql (localAccount->getWallet()->getDatabase()->getPool());
            soci::rowset<soci::row> rows = (sql.prepare << "SELECT op.uid, op.ethereum_operation_uid, op.account_uid,"
                    " op.type, op.hash, op.nonce, op.value,"
                    " op.date, op.sender, op.receiver, op.input_data,"
                    " op.gas_price, op.gas_limit, op.gas_used, op.status, op.block_height"
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
                auto blockHeight = row.get<long long>(15);
                op->setBlockHeight(blockHeight);
                result.emplace_back(op);
            }
            return result;
        }

        Future<std::vector<uint8_t>> ERC20LikeAccount::getTransferToAddressData(const std::shared_ptr<api::BigInt> &amount,
                                                                                const std::string & address) {
            if (! api::Address::isValid(address, _parentCurrency)) {
                throw Exception(api::ErrorCode::INVALID_EIP55_FORMAT, "Invalid address : Invalid EIP55 format");
            }
            return getBalance().map<std::vector<uint8_t>>(getContext(), [amount, address] (const std::shared_ptr<api::BigInt> &balance) {
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
            });
        }

        void ERC20LikeAccount::getTransferToAddressData(const std::shared_ptr<api::BigInt> &amount,
                                                        const std::string &address,
                                                        const std::shared_ptr<api::BinaryCallback> &data) {
            auto context = _account.lock()->getContext();
            getTransferToAddressData(amount, address).callback(context, data);
        }

        void ERC20LikeAccount::putOperation(soci::session &sql, const std::shared_ptr<ERC20LikeOperation> &operation, bool newOperation) {
            auto status = operation->getStatus();
            auto erc20OpUid = operation->getOperationUid();
            auto gasUsed = operation->getUsedGas()->toString(16);
            if (newOperation) {
                auto ethOpUid = operation->getETHOperationUid();
                auto hash = operation->getHash();
                auto receiver = operation->getReceiver();
                auto sender = operation->getSender();
                auto data = hex::toString(operation->getData());
                auto operationType = api::to_string(operation->getOperationType());
                auto nonce = operation->getNonce()->toString(16);
                auto value = operation->getValue()->toString(16);
                auto time = operation->getTime();
                auto gasPrice = operation->getGasPrice()->toString(16);
                auto gasLimit = operation->getGasLimit()->toString(16);
                auto blockHeight = operation->getBlockHeight().value_or(0);
                sql << "INSERT INTO erc20_operations VALUES("
                        ":uid, :eth_op_uid, :accout_uid, :op_type, :hash, :nonce, :value, :date, :sender,"
                        ":receiver, :data, :gas_price, :gas_limit, :gas_used, :status, :block_height"
                        ")"
                        , use(erc20OpUid), use(ethOpUid)
                        , use(_accountUid), use(operationType), use(hash)
                        , use(nonce), use(value), use(time)
                        , use(sender), use(receiver), use(data)
                        , use(gasPrice), use(gasLimit), use(gasUsed)
                        , use(status), use(blockHeight);
            } else {
                // Update
                sql << "UPDATE erc20_operations SET status = :code , gas_used = :gas WHERE uid = :uid"
                        , use(status)
                        , use(gasUsed)
                        , use(erc20OpUid);
            }
        }

        std::shared_ptr<api::OperationQuery> ERC20LikeAccount::queryOperations() {
            auto localAccount = _account.lock();
            if (!localAccount) {
                throw make_exception(api::ErrorCode::NULL_POINTER, "Account was released.");
            }
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

        std::shared_ptr<api::ExecutionContext> ERC20LikeAccount::getContext() {
            auto parentAccount = _account.lock();
            if (!parentAccount) {
                throw make_exception(api::ErrorCode::NULL_POINTER, "Could not lock parent account.");
            }
            return parentAccount->getContext();
        }

    }
}
