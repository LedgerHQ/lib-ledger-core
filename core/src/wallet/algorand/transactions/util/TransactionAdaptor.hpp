/*
 * TransactionAdaptor
 *
 * Created by Rémi Barjon on 04/05/2020.
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

#pragma once

#include "AddressAdaptor.hpp"
#include "B64StringAdaptor.hpp"
#include "MsgpackHelpers.hpp"

#include "../../AlgorandAddress.hpp"
#include "../../utils/B64String.hpp"
#include "../../model/transactions/AlgorandAsset.hpp"
#include "../../model/transactions/AlgorandAssetParams.hpp"
#include "../../model/transactions/AlgorandKeyreg.hpp"
#include "../../model/transactions/AlgorandPayment.hpp"
#include "../../model/transactions/AlgorandSignedTransaction.hpp"
#include "../../model/transactions/AlgorandTransaction.hpp"

#include <utils/Option.hpp>

#include <boost/variant.hpp>

#include <msgpack.hpp>

#include <cstdint>
#include <string>

namespace ledger {
namespace core {
namespace algorand {
namespace constants {

    // Header
    static constexpr char fee[] = "fee";
    static constexpr char fv[] = "fv";
    static constexpr char gen[] = "gen";
    static constexpr char gh[] = "gh";
    static constexpr char grp[] = "grp";
    static constexpr char lv[] = "lv";
    static constexpr char lx[] = "lx";
    static constexpr char note[] = "note";
    static constexpr char snd[] = "snd";
    static constexpr char type[] = "type";

    // Key Registration Transaction
    static constexpr char nonpart[] = "nonpart";
    static constexpr char selkey[] = "selkey";
    static constexpr char votefst[] = "votefst";
    static constexpr char votekd[] = "votekd";
    static constexpr char votekey[] = "votekey";
    static constexpr char votelst[] = "votelst";

    // Payment Transaction
    static constexpr char amt[] = "amt";
    static constexpr char close[] = "close";
    static constexpr char rcv[] = "rcv";

    // Asset configuration Transaction
    static constexpr char apar[] = "apar";
    static constexpr char caid[] = "caid";
    // Asset parameters
    static constexpr char am[] = "am";
    static constexpr char an[] = "an";
    static constexpr char au[] = "au";
    static constexpr char c[] = "c";
    static constexpr char dc[] = "dc";
    static constexpr char df[] = "df";
    static constexpr char f[] = "f";
    static constexpr char m[] = "m";
    static constexpr char r[] = "r";
    static constexpr char t[] = "t";
    static constexpr char un[] = "un";

    // Asset Transfer Transaction
    static constexpr char aamt[] = "aamt";
    static constexpr char aclose[] = "aclose";
    static constexpr char arcv[] = "arcv";
    static constexpr char asnd[] = "asnd";
    static constexpr char xaid[] = "xaid";

    // Asset Freeze Transaction
    static constexpr char afrz[] = "afrz";
    static constexpr char fadd[] = "fadd";
    static constexpr char faid[] = "faid";

    // Signed Transaction
    static constexpr char lsig[] = "lsig";
    static constexpr char msig[] = "msig";
    static constexpr char sig[] = "sig";
    static constexpr char txn[] = "txn";

} // namespace constants
} // namespace algorand
} // namespace core
} // namespace ledger

namespace msgpack {
MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS) {
namespace adaptor {

    using ledger::core::algorand::model::AssetConfigTxnFields;
    using ledger::core::algorand::model::AssetFreezeTxnFields;
    using ledger::core::algorand::model::AssetTransferTxnFields;
    using ledger::core::algorand::model::KeyRegTxnFields;
    using ledger::core::algorand::model::PaymentTxnFields;
    using ledger::core::algorand::model::SignedTransaction;
    using ledger::core::algorand::model::Transaction;
    using ledger::core::algorand::model::AssetParams;

    namespace {

        namespace constants = ledger::core::algorand::constants;

        uint32_t countFieldsInAssetParams(const AssetParams& fields)
        {
            return countValidValues(
                    fields.metaDataHash,
                    fields.assetName,
                    fields.url,
                    fields.clawbackAddr,
                    fields.decimals,
                    fields.defaultFrozen,
                    fields.freezeAddr,
                    fields.managerAddr,
                    fields.reserveAddr,
                    fields.total,
                    fields.unitName);
        }

        template<typename Stream>
        packer<Stream>& packAssetParams(packer<Stream>& o, const AssetParams& params)
        {
            using namespace ledger::core::algorand;

            return packKeyValues(o,
                    KeyValue<Option<std::vector<uint8_t>>>(constants::am, params.metaDataHash),
                    KeyValue<Option<std::string>>(constants::an, params.assetName),
                    KeyValue<Option<std::string>>(constants::au, params.url),
                    KeyValue<Option<Address>>(constants::c, params.clawbackAddr),
                    KeyValue<Option<uint32_t>>(constants::dc, params.decimals),
                    KeyValue<Option<bool>>(constants::df, params.defaultFrozen),
                    KeyValue<Option<Address>>(constants::f, params.freezeAddr),
                    KeyValue<Option<Address>>(constants::m, params.managerAddr),
                    KeyValue<Option<Address>>(constants::r, params.reserveAddr),
                    KeyValue<Option<uint64_t>>(constants::t, params.total),
                    KeyValue<Option<std::string>>(constants::un, params.unitName));
        }

        class TransactionDetailsFieldsCounter : public boost::static_visitor<uint32_t>
        {
        public:
            uint32_t operator()(const KeyRegTxnFields& fields) const
            {
                return countValidValues(
                        fields.nonParticipation,
                        fields.selectionPk,
                        fields.voteFirst,
                        fields.voteKeyDilution,
                        fields.votePk,
                        fields.voteLast);
            }

            uint32_t operator()(const PaymentTxnFields& fields) const
            {
                return countValidValues(
                        fields.amount,
                        fields.closeAddr,
                        fields.receiverAddr);
            }

            uint32_t operator()(const AssetConfigTxnFields& fields) const
            {
                return countValidValues(
                        fields.assetParams,
                        fields.assetId);
            }

            uint32_t operator()(const AssetTransferTxnFields& fields) const
            {
                return countValidValues(
                        fields.assetAmount,
                        fields.assetCloseTo,
                        fields.assetReceiver,
                        fields.assetSender,
                        fields.assetId);
            }

            uint32_t operator()(const AssetFreezeTxnFields& fields) const
            {
                return countValidValues(
                        fields.assetFrozen,
                        fields.frozenAddress,
                        fields.assetId);
            }
        };

        uint32_t countFieldsInDetails(const Transaction::Details& details)
        {
            return boost::apply_visitor(TransactionDetailsFieldsCounter(), details);
        }

        uint32_t countFieldsInHeader(const Transaction::Header& header)
        {
            return countValidValues(
                    header.fee,
                    header.firstValid,
                    header.genesisId,
                    header.genesisHash,
                    header.group,
                    header.lastValid,
                    header.lease,
                    header.note,
                    header.sender,
                    header.type);
        }

        uint32_t countFieldsInTransaction(const Transaction& tx)
        {
            return countFieldsInHeader(tx.header) + countFieldsInDetails(tx.details);
        }

        template<typename Stream>
        class TransactionPacker : public boost::static_visitor<packer<Stream>&>
        {
        public:
            TransactionPacker(packer<Stream>& out, const Transaction::Header& header)
                : header(header), out(out)
            {}

            packer<Stream>& operator()(const KeyRegTxnFields& fields) const
            {
                using namespace ledger::core::algorand;

                return packKeyValues(out,
                        KeyValue<uint64_t>(constants::fee, header.fee),
                        KeyValue<uint64_t>(constants::fv, header.firstValid),
                        KeyValue<Option<std::string>>(constants::gen, header.genesisId),
                        KeyValue<B64String>(constants::gh, header.genesisHash),
                        KeyValue<Option<std::vector<uint8_t>>>(constants::grp, header.group),
                        KeyValue<uint64_t>(constants::lv, header.lastValid),
                        KeyValue<Option<std::vector<uint8_t>>>(constants::lx, header.lease),
                        KeyValue<Option<bool>>(constants::nonpart, fields.nonParticipation),
                        KeyValue<Option<std::vector<uint8_t>>>(constants::note, header.note),
                        KeyValue<std::string>(constants::selkey, fields.selectionPk),
                        KeyValue<Address>(constants::snd, header.sender),
                        KeyValue<std::string>(constants::type, header.type),
                        KeyValue<uint64_t>(constants::votefst, fields.voteFirst),
                        KeyValue<uint64_t>(constants::votekd, fields.voteKeyDilution),
                        KeyValue<std::string>(constants::votekey, fields.votePk),
                        KeyValue<uint64_t>(constants::votelst, fields.voteLast));
            }

            packer<Stream>& operator()(const PaymentTxnFields& fields) const
            {
                using namespace ledger::core::algorand;

                return packKeyValues(out,
                        KeyValue<uint64_t>(constants::amt, fields.amount),
                        KeyValue<Option<Address>>(constants::close, fields.closeAddr),
                        KeyValue<uint64_t>(constants::fee, header.fee),
                        KeyValue<uint64_t>(constants::fv, header.firstValid),
                        KeyValue<Option<std::string>>(constants::gen, header.genesisId),
                        KeyValue<B64String>(constants::gh, header.genesisHash),
                        KeyValue<Option<std::vector<uint8_t>>>(constants::grp, header.group),
                        KeyValue<uint64_t>(constants::lv, header.lastValid),
                        KeyValue<Option<std::vector<uint8_t>>>(constants::lx, header.lease),
                        KeyValue<Option<std::vector<uint8_t>>>(constants::note, header.note),
                        KeyValue<Address>(constants::rcv, fields.receiverAddr),
                        KeyValue<Address>(constants::snd, header.sender),
                        KeyValue<std::string>(constants::type, header.type));
            }

            packer<Stream>& operator()(const AssetConfigTxnFields& fields) const
            {
                using namespace ledger::core::algorand;

                return packKeyValues(out,
                        KeyValue<Option<AssetParams>>(constants::apar, fields.assetParams),
                        KeyValue<Option<uint64_t>>(constants::caid, fields.assetId),
                        KeyValue<uint64_t>(constants::fee, header.fee),
                        KeyValue<uint64_t>(constants::fv, header.firstValid),
                        KeyValue<Option<std::string>>(constants::gen, header.genesisId),
                        KeyValue<B64String>(constants::gh, header.genesisHash),
                        KeyValue<Option<std::vector<uint8_t>>>(constants::grp, header.group),
                        KeyValue<uint64_t>(constants::lv, header.lastValid),
                        KeyValue<Option<std::vector<uint8_t>>>(constants::lx, header.lease),
                        KeyValue<Option<std::vector<uint8_t>>>(constants::note, header.note),
                        KeyValue<Address>(constants::snd, header.sender),
                        KeyValue<std::string>(constants::type, header.type));
            }

            packer<Stream>& operator()(const AssetTransferTxnFields& fields) const
            {
                using namespace ledger::core::algorand;

                return packKeyValues(out,
                        KeyValue<Option<uint64_t>>(constants::aamt, fields.assetAmount),
                        KeyValue<Option<Address>>(constants::aclose, fields.assetCloseTo),
                        KeyValue<Address>(constants::arcv, fields.assetReceiver),
                        KeyValue<Option<Address>>(constants::asnd, fields.assetSender),
                        KeyValue<uint64_t>(constants::fee, header.fee),
                        KeyValue<uint64_t>(constants::fv, header.firstValid),
                        KeyValue<Option<std::string>>(constants::gen, header.genesisId),
                        KeyValue<B64String>(constants::gh, header.genesisHash),
                        KeyValue<Option<std::vector<uint8_t>>>(constants::grp, header.group),
                        KeyValue<uint64_t>(constants::lv, header.lastValid),
                        KeyValue<Option<std::vector<uint8_t>>>(constants::lx, header.lease),
                        KeyValue<Option<std::vector<uint8_t>>>(constants::note, header.note),
                        KeyValue<Address>(constants::snd, header.sender),
                        KeyValue<std::string>(constants::type, header.type),
                        KeyValue<uint64_t>(constants::xaid, fields.assetId));
            }

            packer<Stream>& operator()(const AssetFreezeTxnFields& fields) const
            {
                using namespace ledger::core::algorand;

                return packKeyValues(out,
                        KeyValue<bool>(constants::afrz, fields.assetFrozen),
                        KeyValue<Address>(constants::fadd, fields.frozenAddress),
                        KeyValue<uint64_t>(constants::faid, fields.assetId),
                        KeyValue<uint64_t>(constants::fee, header.fee),
                        KeyValue<uint64_t>(constants::fv, header.firstValid),
                        KeyValue<Option<std::string>>(constants::gen, header.genesisId),
                        KeyValue<B64String>(constants::gh, header.genesisHash),
                        KeyValue<Option<std::vector<uint8_t>>>(constants::grp, header.group),
                        KeyValue<uint64_t>(constants::lv, header.lastValid),
                        KeyValue<Option<std::vector<uint8_t>>>(constants::lx, header.lease),
                        KeyValue<Option<std::vector<uint8_t>>>(constants::note, header.note),
                        KeyValue<Address>(constants::snd, header.sender),
                        KeyValue<std::string>(constants::type, header.type));
            }

        private:
            const Transaction::Header& header;
            packer<Stream>& out;
        };

        template<typename Stream>
        packer<Stream>& packTransaction(packer<Stream>& o, const Transaction& txn)
        {
            return boost::apply_visitor(
                    TransactionPacker<Stream>(o, txn.header),
                    txn.details);
        }

        uint32_t countFieldsInSignedTransaction(const SignedTransaction& stxn)
        {
            return countValidValues(
                    stxn.getSig(),
                    stxn.getTxn());
        }

        template<typename Stream>
        packer<Stream>& packSignedTransaction(packer<Stream>& o, const SignedTransaction& stxn)
        {
            using namespace ledger::core::algorand;

            return packKeyValues(o,
                    KeyValue<Option<std::vector<uint8_t>>>(constants::sig, stxn.getSig()),
                    KeyValue<Transaction>(constants::txn, stxn.getTxn()));
        }

    } // namespace

    template<>
    struct pack<AssetParams>
    {
        template<typename Stream>
        packer<Stream>& operator()(packer<Stream>& o,
                                   const AssetParams& fields) const
        {
            const auto numberOfFields = countFieldsInAssetParams(fields);
            o.pack_map(numberOfFields);
            return packAssetParams(o, fields);
        }

    };

    template<>
    struct pack<Transaction>
    {
        template<typename Stream>
        packer<Stream>& operator()(packer<Stream>& o,
                                   const Transaction& txn) const
        {
            const auto numberOfFields = countFieldsInTransaction(txn);
            o.pack_map(numberOfFields);
            return packTransaction(o, txn);
        }
    };

    template<>
    struct pack<SignedTransaction>
    {
        template<typename Stream>
        packer<Stream>& operator()(packer<Stream>& o,
                                   const SignedTransaction& stxn) const
        {
            const auto numberOfFields = countFieldsInSignedTransaction(stxn);
            o.pack_map(numberOfFields);
            return packSignedTransaction(o, stxn);
        }
    };

} // namespace adaptor
} // MSGPACK_API_VERSION_NAMESPACE(MSGPACK_DEFAULT_API_NS)
} // namespace msgpack

