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
    static constexpr const char* fee = "fee";
    static constexpr const char* fv = "fv";
    static constexpr const char* gen = "gen";
    static constexpr const char* gh = "gh";
    static constexpr const char* grp = "grp";
    static constexpr const char* lv = "lv";
    static constexpr const char* lx = "lx";
    static constexpr const char* note = "note";
    static constexpr const char* snd = "snd";
    static constexpr const char* type = "type";

    // Key Registration Transaction
    static constexpr const char* nonpart = "nonpart";
    static constexpr const char* selkey = "selkey";
    static constexpr const char* votefst = "votefst";
    static constexpr const char* votekd = "votekd";
    static constexpr const char* votekey = "votekey";
    static constexpr const char* votelst = "votelst";

    // Payment Transaction
    static constexpr const char* amt = "amt";
    static constexpr const char* close = "close";
    static constexpr const char* rcv = "rcv";

    // Asset configuration Transaction
    static constexpr const char* apar = "apar";
    static constexpr const char* caid = "caid";
    // Asset parameters
    static constexpr const char* am = "am";
    static constexpr const char* an = "an";
    static constexpr const char* au = "au";
    static constexpr const char* c = "c";
    static constexpr const char* dc = "dc";
    static constexpr const char* df = "df";
    static constexpr const char* f = "f";
    static constexpr const char* m = "m";
    static constexpr const char* r = "r";
    static constexpr const char* t = "t";
    static constexpr const char* un = "un";

    // Asset Transfer Transaction
    static constexpr const char* aamt = "aamt";
    static constexpr const char* aclose = "aclose";
    static constexpr const char* arcv = "arcv";
    static constexpr const char* asnd = "asnd";
    static constexpr const char* xaid = "xaid";

    // Asset Freeze Transaction
    static constexpr const char* afrz = "afrz";
    static constexpr const char* fadd = "fadd";
    static constexpr const char* faid = "faid";

    // Signed Transaction
    static constexpr const char* lsig = "lsig";
    static constexpr const char* msig = "msig";
    static constexpr const char* sig = "sig";
    static constexpr const char* txn = "txn";

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

    namespace constants = ledger::core::algorand::constants;

    inline uint32_t countFieldsInAssetParams(const AssetParams& fields)
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
                makeKeyValue(constants::am, params.metaDataHash),
                makeKeyValue(constants::an, params.assetName),
                makeKeyValue(constants::au, params.url),
                makeKeyValue(constants::c, params.clawbackAddr),
                makeKeyValue(constants::dc, params.decimals),
                makeKeyValue(constants::df, params.defaultFrozen),
                makeKeyValue(constants::f, params.freezeAddr),
                makeKeyValue(constants::m, params.managerAddr),
                makeKeyValue(constants::r, params.reserveAddr),
                makeKeyValue(constants::t, params.total),
                makeKeyValue(constants::un, params.unitName));
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

    inline uint32_t countFieldsInDetails(const Transaction::Details& details)
    {
        return boost::apply_visitor(TransactionDetailsFieldsCounter(), details);
    }

    inline uint32_t countFieldsInHeader(const Transaction::Header& header)
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

    inline uint32_t countFieldsInTransaction(const Transaction& tx)
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
                    makeKeyValue(constants::fee, header.fee),
                    makeKeyValue(constants::fv, header.firstValid),
                    makeKeyValue(constants::gen, header.genesisId),
                    makeKeyValue(constants::gh, header.genesisHash),
                    makeKeyValue(constants::grp, header.group),
                    makeKeyValue(constants::lv, header.lastValid),
                    makeKeyValue(constants::lx, header.lease),
                    makeKeyValue(constants::nonpart, fields.nonParticipation),
                    makeKeyValue(constants::note, header.note),
                    makeKeyValue(constants::selkey, fields.selectionPk),
                    makeKeyValue(constants::snd, header.sender),
                    makeKeyValue(constants::type, header.type),
                    makeKeyValue(constants::votefst, fields.voteFirst),
                    makeKeyValue(constants::votekd, fields.voteKeyDilution),
                    makeKeyValue(constants::votekey, fields.votePk),
                    makeKeyValue(constants::votelst, fields.voteLast));
        }

        packer<Stream>& operator()(const PaymentTxnFields& fields) const
        {
            using namespace ledger::core::algorand;

            return packKeyValues(out,
                    makeKeyValue(constants::amt, fields.amount),
                    makeKeyValue(constants::close, fields.closeAddr),
                    makeKeyValue(constants::fee, header.fee),
                    makeKeyValue(constants::fv, header.firstValid),
                    makeKeyValue(constants::gen, header.genesisId),
                    makeKeyValue(constants::gh, header.genesisHash),
                    makeKeyValue(constants::grp, header.group),
                    makeKeyValue(constants::lv, header.lastValid),
                    makeKeyValue(constants::lx, header.lease),
                    makeKeyValue(constants::note, header.note),
                    makeKeyValue(constants::rcv, fields.receiverAddr),
                    makeKeyValue(constants::snd, header.sender),
                    makeKeyValue(constants::type, header.type));
        }

        packer<Stream>& operator()(const AssetConfigTxnFields& fields) const
        {
            using namespace ledger::core::algorand;

            return packKeyValues(out,
                    makeKeyValue(constants::apar, fields.assetParams),
                    makeKeyValue(constants::caid, fields.assetId),
                    makeKeyValue(constants::fee, header.fee),
                    makeKeyValue(constants::fv, header.firstValid),
                    makeKeyValue(constants::gen, header.genesisId),
                    makeKeyValue(constants::gh, header.genesisHash),
                    makeKeyValue(constants::grp, header.group),
                    makeKeyValue(constants::lv, header.lastValid),
                    makeKeyValue(constants::lx, header.lease),
                    makeKeyValue(constants::note, header.note),
                    makeKeyValue(constants::snd, header.sender),
                    makeKeyValue(constants::type, header.type));
        }

        packer<Stream>& operator()(const AssetTransferTxnFields& fields) const
        {
            using namespace ledger::core::algorand;

            return packKeyValues(out,
                    makeKeyValue(constants::aamt, fields.assetAmount),
                    makeKeyValue(constants::aclose, fields.assetCloseTo),
                    makeKeyValue(constants::arcv, fields.assetReceiver),
                    makeKeyValue(constants::asnd, fields.assetSender),
                    makeKeyValue(constants::fee, header.fee),
                    makeKeyValue(constants::fv, header.firstValid),
                    makeKeyValue(constants::gen, header.genesisId),
                    makeKeyValue(constants::gh, header.genesisHash),
                    makeKeyValue(constants::grp, header.group),
                    makeKeyValue(constants::lv, header.lastValid),
                    makeKeyValue(constants::lx, header.lease),
                    makeKeyValue(constants::note, header.note),
                    makeKeyValue(constants::snd, header.sender),
                    makeKeyValue(constants::type, header.type),
                    makeKeyValue(constants::xaid, fields.assetId));
        }

        packer<Stream>& operator()(const AssetFreezeTxnFields& fields) const
        {
            using namespace ledger::core::algorand;

            return packKeyValues(out,
                    makeKeyValue(constants::afrz, fields.assetFrozen),
                    makeKeyValue(constants::fadd, fields.frozenAddress),
                    makeKeyValue(constants::faid, fields.assetId),
                    makeKeyValue(constants::fee, header.fee),
                    makeKeyValue(constants::fv, header.firstValid),
                    makeKeyValue(constants::gen, header.genesisId),
                    makeKeyValue(constants::gh, header.genesisHash),
                    makeKeyValue(constants::grp, header.group),
                    makeKeyValue(constants::lv, header.lastValid),
                    makeKeyValue(constants::lx, header.lease),
                    makeKeyValue(constants::note, header.note),
                    makeKeyValue(constants::snd, header.sender),
                    makeKeyValue(constants::type, header.type));
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

    inline uint32_t countFieldsInSignedTransaction(const SignedTransaction& stxn)
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
                makeKeyValue(constants::sig, stxn.getSig()),
                makeKeyValue(constants::txn, stxn.getTxn()));
    }

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

    template<typename Stream>
    inline void concatTxnAndSig(Stream& s,
                                const std::vector<uint8_t>& txn,
                                const std::vector<uint8_t>& sig)
    {
        packer<Stream> p(s);
        p.pack_map(2u);
        p.pack("sig");
        p.pack(sig);
        p.pack("txn");
        p.pack_bin_body(
                reinterpret_cast<const char*>(txn.data()),
                txn.size()
        );
    }

} // namespace msgpack

