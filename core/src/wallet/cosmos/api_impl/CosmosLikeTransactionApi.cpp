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
        {}

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

            Document document;
            document.SetObject();

            Document::AllocatorType& allocator = document.GetAllocator();

            Value vString(rapidjson::kStringType);
            vString.SetString(_accountNumber.c_str(), static_cast<rapidjson::SizeType>(_accountNumber.length()), allocator);
            document.AddMember(kAccountNumber, vString, allocator);

            auto chainID = networks::getCosmosLikeNetworkParameters(_currency.name).ChainId;
            vString.SetString(chainID.c_str(), static_cast<rapidjson::SizeType>(chainID.length()), allocator);
            document.AddMember(kChainId, vString, allocator);

            auto getAmountObject = [&] (const std::string &denom, const std::string &amount) {
                Value amountObject(kObjectType);
                Value vStringLocal(rapidjson::kStringType);
                vStringLocal.SetString(amount.c_str(), static_cast<rapidjson::SizeType>(amount.length()), allocator);
                amountObject.AddMember(kAmount, vStringLocal, allocator);
                vStringLocal.SetString(denom.c_str(), static_cast<rapidjson::SizeType>(denom.length()), allocator);
                amountObject.AddMember(kDenom, vStringLocal, allocator);
                return amountObject;
            };

            // Fee object
            auto feeAmountObj = getAmountObject(_txData.fee.amount[0].denom, _txData.fee.amount[0].amount);
            // Technically the feeArray can contain all fee.amount[i] ;
            // But Cosmoshub only accepts uatom as a fee denom so the
            // array is always length 1 for the time being
            Value feeArray(kArrayType);
            feeArray.PushBack(feeAmountObj, allocator);
            Value feeAmountObject(kObjectType);
            feeAmountObject.AddMember(kAmount, feeArray, allocator);
            auto gas = _txData.fee.gas.toString();
            vString.SetString(gas.c_str(), static_cast<rapidjson::SizeType>(gas.length()), allocator);
            feeAmountObject.AddMember(kGas, vString, allocator);
            document.AddMember(kFee, feeAmountObject, allocator);

            vString.SetString(_txData.memo.c_str(), static_cast<rapidjson::SizeType>(_txData.memo.length()), allocator);
            document.AddMember(kMemo, vString, allocator);

            Value msgArray(kArrayType);
            for (auto msg: _txData.messages) {
                msgArray.PushBack(std::make_shared<CosmosLikeMessage>(msg)->toJson(allocator), allocator);
            }
            document.AddMember(kMessages, msgArray, allocator);

            // Add signatures
            if (!_sSignature.empty() && !_rSignature.empty()) {
                Value sigArray(kArrayType);

                Value pubKeyObject(kObjectType);
                // TODO store it somewhere
                std::string pubKeyType = "tendermint/PubKeySecp256k1";
                vString.SetString(pubKeyType.c_str(), static_cast<rapidjson::SizeType>(pubKeyType.length()), allocator);
                pubKeyObject.AddMember(kType, vString, allocator);

                auto pubKeyValue = cereal::base64::encode(_signingPubKey.data(), _signingPubKey.size());
                vString.SetString(pubKeyValue.c_str(), static_cast<rapidjson::SizeType>(pubKeyValue.length()), allocator);
                pubKeyObject.AddMember(kValue, vString, allocator);

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

                // Set pub key
                Value sigObject(kObjectType);
                sigObject.AddMember(kPubKey, pubKeyObject, allocator);
                // Set signature
                auto strSignature = cereal::base64::encode(signature.data(), signature.size());
                vString.SetString(strSignature.c_str(), static_cast<rapidjson::SizeType>(strSignature.length()), allocator);
                sigObject.AddMember(kSignature, vString, allocator);

                sigArray.PushBack(sigObject, allocator);
                document.AddMember(kSignature, sigArray, allocator);
            }

            vString.SetString(_accountSequence.c_str(), static_cast<rapidjson::SizeType>(_accountSequence.length()), allocator);
            document.AddMember(kSequence, vString, allocator);

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

    }
}
