/*
 * AlgorandConstants
 *
 * Created by Hakim Aammar on 14/04/2020.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2020 Ledger
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

#ifndef LEDGER_CORE_ALGORANDCONSTANTS_H
#define LEDGER_CORE_ALGORANDCONSTANTS_H

#include <algorand/api/AlgorandTransactionType.hpp>


namespace ledger {
namespace core {
namespace algorand {
namespace constants {

    using TxType = ledger::core::api::AlgorandTransactionType;

    // Constant values
    static constexpr uint64_t COIN_ID = 283;

    // Explorer endpoints
    static constexpr char purestakeAccountEndpoint[] = "/account/{}";
    static constexpr char purestakeAccountTransactionsEndpoint[] = "/account/{}/transactions";
    static constexpr char purestakeTransactionEndpoint[] = "/transaction/{}";
    static constexpr char purestakeTransactionsEndpoint[] = "/transactions";
    static constexpr char purestakeTransactionsParamsEndpoint[] = "/transactions/params";

    // Transaction keys
    static constexpr char txPaymentType[] = "pay";
    static constexpr char txKeyRegistrationType[] = "keyreg";
    static constexpr char txAssetConfigType[] = "acfg";
    static constexpr char txAssetTransferType[] = "axfer";
    static constexpr char txAssetFreezeType[] = "afrz";

    static constexpr char txPayment[] = "payment";
    static constexpr char txKeyRegistration[] = "keyreg";
    static constexpr char txAssetConfig[] = "curcfg";
    static constexpr char txAssetTransfer[] = "curxfer";
    static constexpr char txAssetFreeze[] = "curfrz";

    static constexpr const char* txTypeToChars(TxType type) {
        switch (type) {
            case TxType::PAYMENT:
                return constants::txPaymentType;
            case TxType::KEY_REGISTRATION:
                return constants::txKeyRegistrationType;
            case TxType::ASSET_CONFIGURATION:
                return constants::txAssetConfigType;
            case TxType::ASSET_TRANSFER:
                return constants::txAssetTransferType;
            case TxType::ASSET_FREEZE:
                return constants::txAssetFreezeType;
            case TxType::UNSUPPORTED:
            default:
                return "";
        }
    }

    static constexpr TxType charsToMsgType(const char* string) {
        if (string == constants::txPaymentType) {
            return TxType::PAYMENT;
        }
        if (string == constants::txKeyRegistrationType) {
            return TxType::KEY_REGISTRATION;
        }
        if (string == constants::txAssetConfigType) {
            return TxType::ASSET_CONFIGURATION;
        }
        if (string == constants::txAssetTransferType) {
            return TxType::ASSET_TRANSFER;
        }
        if (string == constants::txAssetFreezeType) {
            return TxType::ASSET_FREEZE;
        }
        return TxType::UNSUPPORTED;
    }

} // namespace constants
} // namespace algorand
} // namespace core
} // namespace ledger

#endif // LEDGER_CORE_ALGORANDCONSTANTS_H
