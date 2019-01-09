/*
 *
 * RippleLikeTransactionApi
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


#include "RippleLikeTransactionApi.h"
#include <wallet/common/Amount.h>
#include <wallet/common/AbstractAccount.hpp>
#include <wallet/common/AbstractWallet.hpp>
#include <ripple/RippleLikeAddress.h>
#include <bytes/BytesWriter.h>
#include <bytes/BytesReader.h>
#include <bytes/RLP/RLPListEncoder.h>
#include <bytes/RLP/RLPStringEncoder.h>
#include <utils/hex.h>

namespace ledger {
    namespace core {

        RippleLikeTransactionApi::RippleLikeTransactionApi(const api::Currency& currency) {
            _currency = currency;
        }

        RippleLikeTransactionApi::RippleLikeTransactionApi(const std::shared_ptr<OperationApi>& operation) {
            auto& tx = operation->getBackend().rippleTransaction.getValue();
            _time = tx.receivedAt;

            if (tx.block.nonEmpty()) {
                _block = std::make_shared<RippleLikeBlockApi>(tx.block.getValue());
            } else {
                _block = nullptr;
            }

            _hash = tx.hash;

            _currency = operation->getAccount()->getWallet()->getCurrency();

            _fees = std::make_shared<Amount>(_currency, 0, tx.fees);
            _value = std::make_shared<Amount>(_currency, 0, tx.value);

            _receiver = RippleLikeAddress::fromBase58(tx.receiver, _currency);
            _sender = RippleLikeAddress::fromBase58(tx.sender, _currency);

        }

        std::string RippleLikeTransactionApi::getHash() {
            return _hash;
        }

        std::shared_ptr<api::Amount> RippleLikeTransactionApi::getFees() {
            return _fees;
        }

        std::shared_ptr<api::RippleLikeAddress> RippleLikeTransactionApi::getReceiver() {
            return _receiver;
        }

        std::shared_ptr<api::RippleLikeAddress> RippleLikeTransactionApi::getSender() {
            return _sender;
        }

        std::shared_ptr<api::Amount> RippleLikeTransactionApi::getValue() {
            return _value;
        }

        std::chrono::system_clock::time_point RippleLikeTransactionApi::getDate() {
            return _time;
        }

        std::shared_ptr<api::RippleLikeBlock> RippleLikeTransactionApi::getBlock() {
            return _block;
        }

        void RippleLikeTransactionApi::setSignature(const std::vector<uint8_t> & vSignature, const std::vector<uint8_t> & rSignature, const std::vector<uint8_t> & sSignature) {
            _vSignature = vSignature;
            _rSignature = rSignature;
            _sSignature = sSignature;
        }

        void RippleLikeTransactionApi::setDERSignature(const std::vector<uint8_t> & signature) {
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

        std::vector<uint8_t> RippleLikeTransactionApi::serialize() {
            BytesWriter writer;
            return writer.toByteArray();
        }

        RippleLikeTransactionApi & RippleLikeTransactionApi::setFees(const std::shared_ptr<BigInt>& fees) {
            if (!fees) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "RippleLikeTransactionApi::setFees: Invalid Fees");
            }
            _fees = std::make_shared<Amount>(_currency, 0, *fees);
            return *this;
        }

        RippleLikeTransactionApi & RippleLikeTransactionApi::setValue(const std::shared_ptr<BigInt>& value) {
            if (!value) {
                throw make_exception(api::ErrorCode::INVALID_ARGUMENT, "RippleLikeTransactionApi::setValue: Invalid Value");
            }

            _value = std::make_shared<Amount>(_currency, 0, *value);
            return *this;
        }

        RippleLikeTransactionApi & RippleLikeTransactionApi::setReceiver(const std::string &receiver) {
            _receiver = RippleLikeAddress::fromBase58(receiver, _currency);
            return *this;
        }


    }
}