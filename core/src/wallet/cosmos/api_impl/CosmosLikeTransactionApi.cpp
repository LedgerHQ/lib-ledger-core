/*
 *
 * CosmosLikeTransactionApi
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


#include <wallet/cosmos/api_impl/CosmosLikeTransactionApi.hpp>

#include <fmt/format.h>
#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>
#include <cereal/external/base64.hpp>

#include <api/Amount.hpp>
#include <wallet/common/AbstractAccount.hpp>
#include <wallet/common/AbstractWallet.hpp>
#include <bytes/BytesWriter.h>
#include <bytes/BytesReader.h>
#include <utils/hex.h>
#include <math/BigInt.h>

#include <wallet/cosmos/CosmosLikeCurrencies.hpp>
#include <cosmos/CosmosLikeAddress.hpp>
#include <api/CosmosLikeMsgType.hpp>
#include <wallet/cosmos/CosmosLikeMessage.hpp>
#include <wallet/cosmos/CosmosLikeConstants.hpp>
#include <wallet/cosmos/CosmosNetworks.hpp>

using namespace rapidjson;

// Used to sort lexicographically the keys in dictionnary as per
// https://github.com/cosmos/ledger-cosmos-app/blob/master/docs/TXSPEC.md
struct NameComparator {
    bool operator()(const Value::Member &lhs, const Value::Member &rhs) const {
        return (strcmp(lhs.name.GetString(), rhs.name.GetString()) < 0);
    }
};

// Recursively sort the dictionnaries in the given json
static void sortJson(Value &val) {
    // This code assumes that
    // only "objects" ({key: Value}) and "arrays" ([Value]) can have nested
    // dictionnaries to sort
    // Therefore the base case is reaching a "primitive" type as inner Value
    if (val.IsObject()) {
        std::sort(val.MemberBegin(), val.MemberEnd(), NameComparator());
        for (auto subValue = val.MemberBegin(); subValue != val.MemberEnd(); subValue++) {
            sortJson(subValue->value);
        }
    } else if (val.IsArray()) {
        for (auto subValue = val.Begin(); subValue != val.End(); subValue++) {
            sortJson(*subValue);
        }
    }
}

namespace ledger {
    namespace core {

        CosmosLikeTransactionApi::CosmosLikeTransactionApi(const cosmos::Transaction& txData) :
            _txData(txData)
        {
            _currency = currencies::ATOM;
        }

        CosmosLikeTransactionApi::CosmosLikeTransactionApi(const std::shared_ptr<OperationApi>& baseOp):
        CosmosLikeTransactionApi(baseOp->getBackend().cosmosTransaction.getValue().tx){
            _currency = baseOp->getCurrency();
        }

        std::string CosmosLikeTransactionApi::getMemo() const {
            return _txData.memo;
        }

        std::vector<std::shared_ptr<api::CosmosLikeMessage>> CosmosLikeTransactionApi::getMessages() const {
            auto result = std::vector<std::shared_ptr<api::CosmosLikeMessage>>();
            std::transform(_txData.messages.begin(), _txData.messages.end(), std::back_inserter(result),
                           [](const auto& message) -> std::shared_ptr<CosmosLikeMessage> {
                               return std::make_shared<CosmosLikeMessage>(message);
                           }
            );
            return result;
        }

        CosmosLikeTransactionApi & CosmosLikeTransactionApi::setMessages(const std::vector<std::shared_ptr<api::CosmosLikeMessage>> & cmessages) {
            auto result = std::vector<cosmos::Message>();
            for (auto& message : cmessages) {
                auto concrete_message = std::dynamic_pointer_cast<CosmosLikeMessage>(message);
                if (!concrete_message) {
                    throw Exception(
                        api::ErrorCode::INVALID_ARGUMENT,
                        fmt::format("Unknown backend message"));
                }
                result.push_back(concrete_message->getRawData());
            }
            _txData.messages = result;
            return *this;
        }

        std::string CosmosLikeTransactionApi::getHash() const {
            return _txData.hash;
        }

        std::shared_ptr<api::Amount> CosmosLikeTransactionApi::getFee() const {
            return std::make_shared<Amount>(_currency, 0, BigInt(_txData.fee.amount[0].amount));
        }

       std::chrono::system_clock::time_point CosmosLikeTransactionApi::getDate() const {
            return _txData.timestamp;
        }

        std::vector<uint8_t> CosmosLikeTransactionApi::getSigningPubKey() const {
            return _signingPubKey;
        }

        std::shared_ptr<api::Amount> CosmosLikeTransactionApi::getGas() const {
            return std::make_shared<Amount>(_currency, 0, _txData.fee.gas);
        }

        void CosmosLikeTransactionApi::setSignature(const std::vector<uint8_t> &rSignature,
                                                    const std::vector<uint8_t> &sSignature) {
            _rSignature = rSignature;
            _sSignature = sSignature;
        }

        void CosmosLikeTransactionApi::setDERSignature(const std::vector<uint8_t> &signature) {
            BytesReader reader(signature);
            //DER prefix
            reader.readNextByte();
            //Total length
            reader.readNextVarInt();
            //Nb of elements for R
            reader.readNextByte();
            //R length
            auto rSize = reader.readNextVarInt();
            if (rSize > 0 && reader.peek() == 0x00) {
                reader.readNextByte();
                _rSignature = reader.read(rSize - 1);
            } else {
                _rSignature = reader.read(rSize);
            }
            //Nb of elements for S
            reader.readNextByte();
            //S length
            auto sSize = reader.readNextVarInt();
            if (sSize > 0 && reader.peek() == 0x00) {
                reader.readNextByte();
                _sSignature = reader.read(sSize - 1);
            } else {
                _sSignature = reader.read(sSize);
            }
        }

		void CosmosLikeTransactionApi::setRawData(const cosmos::Transaction &txData) {
			_txData = txData;
		}

        const cosmos::Transaction & CosmosLikeTransactionApi::getRawData() const {
            return _txData;
        }

        std::string CosmosLikeTransactionApi::serialize() {

            using namespace cosmos::constants;
            Value vString(kStringType);

            Document document;
            document.SetObject();
            Document::AllocatorType& allocator = document.GetAllocator();

            Value txObject(kObjectType);
            {
                Value msgArray(kArrayType);
                for (const auto& msg: _txData.messages) {
                    msgArray.PushBack(std::make_shared<CosmosLikeMessage>(msg)->toJson(allocator), allocator);
                }
                txObject.AddMember(kMessage, msgArray, allocator);

                // Add fees
                Value feeObject(kObjectType);
                Value feeArray(kArrayType);
                {
                    auto gas = _txData.fee.gas.toString();
                    vString.SetString(gas.c_str(), static_cast<SizeType>(gas.length()), allocator);
                    feeObject.AddMember(kGas, vString, allocator);

                    Value feeAmountArray(kArrayType);
                    auto getAmountObject = [&] (const std::string &denom, const std::string &amount) {
                        Value amountObject(kObjectType);
                        vString.SetString(amount.c_str(), static_cast<SizeType>(amount.length()), allocator);
                        amountObject.AddMember(kAmount, vString, allocator);
                        vString.SetString(denom.c_str(), static_cast<SizeType>(denom.length()), allocator);
                        amountObject.AddMember(kDenom, vString, allocator);
                        return amountObject;
                    };

                    // Technically the feeArray can contain all fee.amount[i] ;
                    // But Cosmoshub only accepts uatom as a fee denom so the
                    // array is always length 1 for the time being
                    auto feeAmountObj = getAmountObject(_txData.fee.amount[0].denom, _txData.fee.amount[0].amount);
                    feeArray.PushBack(feeAmountObj, allocator);
                    feeObject.AddMember(kAmount, feeArray, allocator);
                }
                txObject.AddMember(kFee, feeObject, allocator);

                // Add signatures
                if (!_sSignature.empty() && !_rSignature.empty()) {
                    Value sigArray(kArrayType);
                    Value sigObject(kObjectType);

                    { // Set pub key
                        Value pubKeyObject(kObjectType);
                        // TODO store it somewhere
                        std::string pubKeyType = "tendermint/PubKeySecp256k1";
                        vString.SetString(pubKeyType.c_str(), static_cast<SizeType>(pubKeyType.length()), allocator);
                        pubKeyObject.AddMember(kType, vString, allocator);

                        auto pubKeyValue = cereal::base64::encode(_signingPubKey.data(), _signingPubKey.size());
                        vString.SetString(pubKeyValue.c_str(), static_cast<SizeType>(pubKeyValue.length()), allocator);
                        pubKeyObject.AddMember(kValue, vString, allocator);
                        sigObject.AddMember(kPubKey, pubKeyObject, allocator);
                    }

                    { // Set signature
                        auto pad = [] (const std::vector<uint8_t> &input) {
                            auto output = input;
                            while(output.size() < 32) {
                                output.emplace(output.begin(), 0x00);
                            }
                            return output;
                        };
                        auto signature = vector::concat(pad(_rSignature), pad(_sSignature));
                        if (signature.size() != 64) {
                            throw Exception(api::ErrorCode::INVALID_ARGUMENT, "Invalid signature when serializing transaction");
                        }
                        auto strSignature = cereal::base64::encode(signature.data(), signature.size());
                        vString.SetString(strSignature.c_str(), static_cast<SizeType>(strSignature.length()), allocator);
                        sigObject.AddMember(kSignature, vString, allocator);
                    }

                    sigArray.PushBack(sigObject, allocator);
                    txObject.AddMember(kSignatures, sigArray, allocator);
                }

                vString.SetString(_txData.memo.c_str(), static_cast<SizeType>(_txData.memo.length()), allocator);
                txObject.AddMember(kMemo, vString, allocator);
            }
            document.AddMember(kTx, txObject, allocator);

            // Set mode
            // FIXME What mode do we want? (sync|async|block)
            vString.SetString("async", static_cast<SizeType>(5), allocator);
            document.AddMember(kMode, vString, allocator);

            StringBuffer buffer;
            Writer<StringBuffer> writer(buffer);
            sortJson(document);
            document.Accept(writer);
            return buffer.GetString();
        }

        CosmosLikeTransactionApi &CosmosLikeTransactionApi::setCurrency(const api::Currency& currency) {
            _currency = currency;
            return *this;
        }

        CosmosLikeTransactionApi &CosmosLikeTransactionApi::setSigningPubKey(const std::vector<uint8_t> &pubKey) {
            _signingPubKey = pubKey;
            return *this;
        }

        CosmosLikeTransactionApi &CosmosLikeTransactionApi::setHash(const std::string &rhs_hash) {
            _txData.hash = rhs_hash;
            return *this;
        }

        CosmosLikeTransactionApi &CosmosLikeTransactionApi::setGas(const std::shared_ptr<BigInt> &rhs_gas) {
            if (!rhs_gas) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "CosmosLikeTransactionApi::setGas: Invalid gas");
            }
            _txData.fee.gas = *rhs_gas;
            return *this;
        }

        CosmosLikeTransactionApi &CosmosLikeTransactionApi::setFee(const std::shared_ptr<BigInt> &rhs_fee) {
            if (!rhs_fee) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT,
                                     "CosmosLikeTransactionApi::setGasPrice: Invalid fee");
            }
            // Assumes uatom
            if (_txData.fee.amount.size() > 0) {
                _txData.fee.amount[0].amount = rhs_fee->toString();
                _txData.fee.amount[0].denom = _currency.units.front().name;
            } else {
                _txData.fee.amount.emplace_back(rhs_fee->toString(), _currency.units.front().name);
            }

            return *this;
        }

        CosmosLikeTransactionApi &CosmosLikeTransactionApi::setSequence(const std::string &sequence) {
            _accountSequence = sequence;
            return *this;
        }

        CosmosLikeTransactionApi &CosmosLikeTransactionApi::setMemo(const std::string &rhs_memo) {
            _txData.memo = rhs_memo;
            return *this;
        }

        CosmosLikeTransactionApi &CosmosLikeTransactionApi::setAccountNumber(const std::string &accountNumber) {
            _accountNumber = accountNumber;
            return *this;
        }

        std::string CosmosLikeTransactionApi::serializeTransfer() const {
            auto document = rapidjson::Document();
            document.SetObject();
            auto& allocator = document.GetAllocator();

            auto baseReq = rapidjson::Value(rapidjson::kObjectType);
            for (const auto &message : _txData.messages) {
                const auto tmpMessage = std::make_shared<CosmosLikeMessage>(message);
                if (!(tmpMessage->getMessageType() == api::CosmosLikeMsgType::MSGSEND)) continue;

                auto buffer = rapidjson::Value(rapidjson::kStringType);
                auto makeStringValue = [&allocator](const std::string &str) {
                    auto res = rapidjson::Value(rapidjson::kStringType);
                    res.SetString(str.c_str(), static_cast<rapidjson::SizeType>(str.size()), allocator);
                    return res;
                };

                const auto msgSend = CosmosLikeMessage::unwrapMsgSend(tmpMessage);

                buffer = makeStringValue(msgSend.fromAddress);
                baseReq.AddMember(cosmos::constants::kFrom, buffer, allocator);

                buffer = makeStringValue(_txData.memo);
                baseReq.AddMember(cosmos::constants::kMemo, buffer, allocator);

                buffer = makeStringValue(networks::getCosmosLikeNetworkParameters(_currency.name).ChainId);
                baseReq.AddMember(cosmos::constants::kChainId, buffer, allocator);

                buffer = makeStringValue(_accountNumber);
                baseReq.AddMember(cosmos::constants::kAccountNumber, buffer, allocator);

                buffer = makeStringValue(_accountSequence);
                baseReq.AddMember(cosmos::constants::kSequence, buffer, allocator);

                buffer = makeStringValue(_txData.fee.gas.toString());
                baseReq.AddMember(cosmos::constants::kGas, buffer, allocator);

                // todo : remove duplicated code
                auto getAmountObject = [&] (const std::string &denom, const std::string &amount) {
                    auto tmp = rapidjson::Value(rapidjson::kStringType);
                    auto amountObject = rapidjson::Value(rapidjson::kObjectType);
                    tmp.SetString(amount.c_str(), static_cast<rapidjson::SizeType>(amount.length()), allocator);
                    amountObject.AddMember(cosmos::constants::kAmount, tmp, allocator);
                    tmp.SetString(denom.c_str(), static_cast<rapidjson::SizeType>(denom.length()), allocator);
                    amountObject.AddMember(cosmos::constants::kDenom, tmp, allocator);
                    return amountObject;
                };

                auto fees = rapidjson::Value(rapidjson::kArrayType);
                auto feeAmount = getAmountObject(_txData.fee.amount[0].denom, _txData.fee.amount[0].amount);
                fees.PushBack(feeAmount, allocator);
                baseReq.AddMember(cosmos::constants::kFees, fees, allocator);

                auto simulate = rapidjson::Value(rapidjson::kTrueType);
                baseReq.AddMember(cosmos::constants::kSimulate, simulate, allocator);

                document.AddMember(cosmos::constants::kBaseReq, baseReq, allocator);


                auto amount = rapidjson::Value(rapidjson::kArrayType);
                auto amountObj = getAmountObject(msgSend.amount[0].denom, msgSend.amount[0].amount);
                amount.PushBack(amountObj, allocator);
                document.AddMember(cosmos::constants::kAmount, amount, allocator);

                break;
            }

            auto buffer = rapidjson::StringBuffer();
            auto writer = rapidjson::Writer<rapidjson::StringBuffer>(buffer);
            sortJson(document);
            document.Accept(writer);
            return buffer.GetString();
        }

    }
}
