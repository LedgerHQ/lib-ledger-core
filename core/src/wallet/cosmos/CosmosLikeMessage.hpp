/*
 *
 * CosmosLikeMessage
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

#pragma once

#include <rapidjson/document.h>

#include <collections/DynamicObject.hpp>

#include <wallet/cosmos/cosmos.hpp>
#include <wallet/common/api_impl/OperationApi.h>

#include <api/CosmosLikeAmount.hpp>
#include <api/CosmosLikeContent.hpp>
#include <api/CosmosLikeVoteOption.hpp>
#include <api/CosmosLikeMsgSend.hpp>
#include <api/CosmosLikeMsgDelegate.hpp>
#include <api/CosmosLikeMsgUndelegate.hpp>
#include <api/CosmosLikeMsgBeginRedelegate.hpp>
#include <api/CosmosLikeMsgSubmitProposal.hpp>
#include <api/CosmosLikeMsgVote.hpp>
#include <api/CosmosLikeMsgDeposit.hpp>
#include <api/CosmosLikeMsgWithdrawDelegationReward.hpp>
#include <api/CosmosLikeMessage.hpp>
#include <api/CosmosLikeMsgType.hpp>

namespace ledger {
	namespace core {
		class CosmosLikeMessage : public api::CosmosLikeMessage {
			friend api::CosmosLikeMessage;

		public:

        	CosmosLikeMessage(const cosmos::Message& msgData);
        	CosmosLikeMessage(const std::shared_ptr<OperationApi>& baseOp);

			virtual api::CosmosLikeMsgType getMessageType() const override;
			virtual std::string getRawMessageType() const override;

			rapidjson::Value toJson(rapidjson::Document::AllocatorType& allocator) const;

            void setRawData(const cosmos::Message &msgData);
			const cosmos::Message& getRawData() const;
            const std::string& getFromAddress() const;

		private:

			cosmos::Message _msgData;

		};
	}
}
