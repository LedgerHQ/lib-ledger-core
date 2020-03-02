/*
 *
 * CosmosLikeOperation
 *
 * Created by El Khalil Bellakrid on  14/06/2019.
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


#include <cosmos/api_impl/CosmosLikeOperation.hpp>

#include <core/api/ErrorCode.hpp>
#include <core/utils/Exception.hpp>
#include <core/operation/OperationDatabaseHelper.hpp>

#include <cosmos/api_impl/CosmosLikeTransactionApi.hpp>
#include <cosmos/CosmosLikeMessage.hpp>

namespace ledger {
    namespace core {

        CosmosLikeOperation::CosmosLikeOperation(ledger::core::cosmos::Transaction const& tx,
                                                 ledger::core::cosmos::Message const& msg) :
            //Operation(account), // TODO Need this?
            _txApi(std::make_shared<CosmosLikeTransactionApi>(tx)),
            _msgApi(std::make_shared<CosmosLikeMessage>(msg))
        {
            /* TODO Complete missing info for _txApi ?
                api::Currency _currency;
                std::string _accountNumber;
                std::string _accountSequence;
                std::vector<uint8_t> _rSignature;
                std::vector<uint8_t> _sSignature;
                std::vector<uint8_t> _signingPubKey;
            */
        }

        void CosmosLikeOperation::setTransactionData(ledger::core::cosmos::Transaction const& tx) {
            if (_txApi == nullptr) {
                _txApi = std::make_shared<CosmosLikeTransactionApi>(tx);
            }
            std::static_pointer_cast<CosmosLikeTransactionApi>(_txApi)->setRawData(tx);
        }

        void CosmosLikeOperation::setMessageData(ledger::core::cosmos::Message const& msg) {
            if (_msgApi == nullptr) {
                _msgApi = std::make_shared<CosmosLikeMessage>(msg);
            }
            std::static_pointer_cast<CosmosLikeMessage>(_msgApi)->setRawData(msg);
        }

		std::shared_ptr<api::CosmosLikeTransaction> CosmosLikeOperation::getTransaction() {
            return _txApi;
        }

		std::shared_ptr<api::CosmosLikeMessage> CosmosLikeOperation::getMessage() {
			return _msgApi;
		}

        void CosmosLikeOperation::refreshUid(const std::string &msgIndex) {
            auto final = fmt::format("{}+{}+{}", _txApi->getHash(), msgIndex, api::to_string(_msgApi->getMessageType()));

            uid = OperationDatabaseHelper::createUid(accountUid, final, getOperationType());
        }

    }
}
