/*
 *
 * RippleLikeTransactionBuilder
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

#include "RippleLikeTransactionBuilder.h"
#include <math/BigInt.h>
#include <api/RippleLikeTransactionCallback.hpp>
#include <wallet/ripple/api_impl/RippleLikeTransactionApi.h>
#include <bytes/BytesReader.h>
#include <wallet/currencies.hpp>

namespace ledger {
    namespace core {

        RippleLikeTransactionBuilder::RippleLikeTransactionBuilder(
                const std::shared_ptr<api::ExecutionContext> &context,
                const api::Currency &currency,
                const std::shared_ptr<RippleLikeBlockchainExplorer> &explorer,
                const std::shared_ptr<spdlog::logger> &logger,
                const RippleLikeTransactionBuildFunction &buildFunction) {
            _context = context;
            _currency = currency;
            _explorer = explorer;
            _build = buildFunction;
            _logger = logger;
            _request.wipe = false;
        }

        RippleLikeTransactionBuilder::RippleLikeTransactionBuilder(const RippleLikeTransactionBuilder &cpy) {
            _currency = cpy._currency;
            _build = cpy._build;
            _request = cpy._request;
            _context = cpy._context;
            _logger = cpy._logger;
        }

        std::shared_ptr<api::RippleLikeTransactionBuilder>
        RippleLikeTransactionBuilder::sendToAddress(const std::shared_ptr<api::Amount> &amount,
                                                    const std::string &address) {
            _request.value = std::make_shared<BigInt>(amount->toString());
            _request.toAddress = address;
            return shared_from_this();
        }

        std::shared_ptr<api::RippleLikeTransactionBuilder>
        RippleLikeTransactionBuilder::wipeToAddress(const std::string &address) {
            _request.toAddress = address;
            _request.wipe = true;
            return shared_from_this();
        }

        std::shared_ptr<api::RippleLikeTransactionBuilder>
        RippleLikeTransactionBuilder::setFees(const std::shared_ptr<api::Amount> & fees) {
            _request.fees = std::make_shared<BigInt>(fees->toString());
            return shared_from_this();
        }

        void RippleLikeTransactionBuilder::build(const std::shared_ptr<api::RippleLikeTransactionCallback> &callback) {
            build().callback(_context, callback);
        }

        Future<std::shared_ptr<api::RippleLikeTransaction>> RippleLikeTransactionBuilder::build() {
            return _build(_request, _explorer);
        }

        std::shared_ptr<api::RippleLikeTransactionBuilder> RippleLikeTransactionBuilder::clone() {
            return std::make_shared<RippleLikeTransactionBuilder>(*this);
        }

        void RippleLikeTransactionBuilder::reset() {
            _request = RippleLikeTransactionBuildRequest();
        }

        std::shared_ptr<api::RippleLikeTransaction>
        api::RippleLikeTransactionBuilder::parseRawUnsignedTransaction(const api::Currency &currency,
                                                                       const std::vector<uint8_t> &rawTransaction) {
            return ::ledger::core::RippleLikeTransactionBuilder::parseRawTransaction(currency, rawTransaction, false);
        }

        std::shared_ptr<api::RippleLikeTransaction>
        api::RippleLikeTransactionBuilder::parseRawSignedTransaction(const api::Currency &currency,
                                                                     const std::vector<uint8_t> &rawTransaction) {
            return ::ledger::core::RippleLikeTransactionBuilder::parseRawTransaction(currency, rawTransaction, true);
        }

        std::shared_ptr<api::RippleLikeTransaction>
        RippleLikeTransactionBuilder::parseRawTransaction(const api::Currency &currency,
                                                          const std::vector<uint8_t> &rawTransaction,
                                                          bool isSigned) {
            auto tx = std::make_shared<RippleLikeTransactionApi>(currencies::RIPPLE);
            BytesReader reader(rawTransaction);

            //1 byte TransactionType Field ID:   Type Code = 1, Field Code = 2
            reader.readNextByte();
            //2 bytes TransactionType ("Payment")
            reader.read(2);

            //1 byte Flags Field ID:   Type Code = 2, Field Code = 2
            reader.readNextByte();
            //4 bytes Flags (tfFullyCanonicalSig ?)
            reader.read(4);

            //1 byte Sequence Field ID:   Type Code = 2, Field Code = 4
            reader.readNextByte();
            //4 bytes Sequence
            auto sequence = reader.read(4);
            auto bigIntSequence = BigInt::fromHex(hex::toString(sequence));
            tx->setSequence(bigIntSequence);

            //2 bytes LastLedgerSequence Field ID:   Type Code = 2, Field Code = 27
            reader.read(2);
            //LastLedgerSequence
            auto ledgerSequence = reader.read(4);
            auto bigIntLedgerSequence = BigInt::fromHex(hex::toString(ledgerSequence));
            tx->setLedgerSequence(bigIntLedgerSequence);

            //Usefull to compute amounts
            auto maxAmountBound = std::vector<uint8_t>({0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00});
            auto bigIntMax = BigInt::fromHex(hex::toString(maxAmountBound));

            //1 byte Amount Field ID:   Type Code = 6, Field Code = 1
            reader.readNextByte();
            //8 bytes Amount (with bitwise OR with 0x4000000000000000)
            auto amountOR = reader.read(8);
            auto bigIntAmountOR = BigInt::fromHex(hex::toString(amountOR));
            tx->setValue(std::make_shared<BigInt>(bigIntAmountOR - bigIntMax));

            //1 byte Fee Field ID:   Type Code = 6, Field Code = 8
            reader.readNextByte();
            //8 bytes Fees (with bitwise OR with 0x4000000000000000)
            auto feesOR = reader.read(8);
            auto bigIntFeesOR = BigInt::fromHex(hex::toString(feesOR));
            tx->setFees(std::make_shared<BigInt>(bigIntFeesOR - bigIntMax));

            //TODO: !!!find out if this is included in raw unsigned tx or not
            //1 byte Signing pubKey Field ID:   Type Code = 7, Field Code = 3 (STI_VL = 7 type)
            reader.readNextByte();
            //Var bytes Signing pubKey (prefix length)
            auto pubKeyLength = reader.readNextVarInt();
            auto signingPubKey = reader.read(pubKeyLength);
            tx->setSigningPubKey(signingPubKey);

            if (isSigned) {
                //1 byte Signature Field ID:   Type Code = 7, Field Code = 4 (STI_VL = 7 type, and TxnSignature = 4)
                reader.readNextByte();
                auto sigLength = reader.readNextVarInt();
                auto sig = reader.read(sigLength);

                //Set signature in Tx
                BytesReader sigReader(sig);
                //DER prefix
                sigReader.readNextByte();
                //Total length
                sigReader.readNextVarInt();

                //Nb of elements for R
                sigReader.readNextByte();
                //R length
                auto rSize = sigReader.readNextVarInt();
                //TODO: verify that we don't truncate leading null byte
                auto rSignature = sigReader.read(rSize);

                //Nb of elements for S
                sigReader.readNextByte();
                //S length
                auto sSize = sigReader.readNextVarInt();
                //TODO: verify that we don't truncate leading null byte
                auto sSignature = sigReader.read(sSize);
                tx->setSignature(rSignature, sSignature);
            }

            //1 byte Account Field ID: Type Code = 8, Field Code = 1 (STI_ACCOUNT = 8 type, and Account = 1)
            reader.readNextByte();
            //20 bytes Acount (hash160 of pubKey without 0x00 prefix)
            auto accountAddressLength = reader.readNextVarInt();
            auto accountAddressHash = reader.read(accountAddressLength);
            auto accountAddress = std::make_shared<RippleLikeAddress>(currencies::RIPPLE, accountAddressHash, std::vector<uint8_t>({0x00}));
            tx->setSender(accountAddress);

            //1 byte Destination Field ID: Type Code = 8, Field Code = 3 (STI_ACCOUNT = 8 type, and Destination = 3)
            reader.readNextByte();
            //20 bytes Destination (hash160 of pubKey without 0x00 prefix)
            auto destAddressLength = reader.readNextVarInt();
            auto destAddressHash = reader.read(destAddressLength);
            auto destAddress = std::make_shared<RippleLikeAddress>(currencies::RIPPLE, destAddressHash, std::vector<uint8_t>({0x00}));
            tx->setReceiver(destAddress);
            return tx;
        }

    }
}