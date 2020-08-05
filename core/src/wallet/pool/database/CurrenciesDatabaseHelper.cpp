/*
 *
 * CurrenciesDatabaseHelper
 * ledger-core
 *
 * Created by Pierre Pollastri on 15/05/2017.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
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
#include "CurrenciesDatabaseHelper.hpp"
#include <api/BitcoinLikeNetworkParameters.hpp>
#include <api/EthereumLikeNetworkParameters.hpp>
#include <utils/hex.h>
#include <api/enum_from_string.hpp>
#include <api/Currency.hpp>
#include <api/BitcoinLikeFeePolicy.hpp>
#include <collections/strings.hpp>
#include <database/soci-number.h>

using namespace soci;

bool ledger::core::CurrenciesDatabaseHelper::insertCurrency(soci::session &sql,
                                                            const ledger::core::api::Currency &currency) {
    int count;
    sql << "SELECT COUNT(*) FROM currencies WHERE name = :name",
    soci::use(currency.name),
    soci::into(count);
    bool inserted = false;
    if (count == 0) {
        auto currencyType = api::to_string(currency.walletType);
        // Insert currency
        sql << "INSERT INTO currencies VALUES(:name, :type, :bip44, :uri)",
            use(currency.name),
            use(currencyType),
            use(currency.bip44CoinType),
            use(currency.paymentUriScheme);

        // Insert network parameters
        switch (currency.walletType) {
            case api::WalletType::BITCOIN: {
                const auto &params = currency.bitcoinLikeNetworkParameters.value();

                std::stringstream additionalBIPs;
                std::string separator(";");
                strings::join(params.AdditionalBIPs, additionalBIPs, separator);
                auto BIPs = additionalBIPs.str();
                auto hexP2PKHVersion = hex::toString(params.P2PKHVersion);
                auto hexP2SHVersion = hex::toString(params.P2SHVersion);
                auto hexXPUBVersion = hex::toString(params.XPUBVersion);
                auto feePolicy = api::to_string(params.FeePolicy);
                auto sigHash = hex::toString(params.SigHash);
                auto useTimestampedTransaction = static_cast<int>(params.UsesTimestampedTransaction);
                sql
                << "INSERT INTO bitcoin_currencies VALUES(:name, :identifier, :p2pkh, :p2sh, :xpub, :dust, :fee_policy, :prefix, :use_timestamped_transaction, :delay, :sigHashType, :additionalBIPs)",
                use(currency.name),
                use(params.Identifier),
                use(hexP2PKHVersion),
                use(hexP2SHVersion),
                use(hexXPUBVersion),
                use(params.DustAmount),
                use(feePolicy),
                use(params.MessagePrefix),
                use(useTimestampedTransaction),
                use(params.TimestampDelay),
                use(sigHash),
                use(BIPs);
                break;
            }
            case api::WalletType::COSMOS: {
                auto &params = currency.cosmosLikeNetworkParameters.value();

                std::stringstream additionalCIPs;
                std::string separator(",");
                strings::join(params.AdditionalCIPs, additionalCIPs, separator);
                auto CIPs = additionalCIPs.str();
                auto hexXPUBVersion = hex::toString(params.XPUBVersion);
                auto hexPubKeyPrefix = hex::toString(params.PubKeyPrefix);
                auto hexAddressPrefix = hex::toString(params.AddressPrefix);
                sql << "INSERT INTO cosmos_currencies VALUES(:name, :identifier, :xpub, "
                       ":pubkey_prefix, :address_prefix, :message_prefix, :chain_id, "
                       ":additionalCIPs)",
                    use(currency.name), use(params.Identifier), use(hexXPUBVersion),
                    use(hexPubKeyPrefix), use(hexAddressPrefix), use(params.MessagePrefix),
                    use(params.ChainId), use(CIPs);
                break;
            }
            case api::WalletType::ETHEREUM: {
                const auto &params = currency.ethereumLikeNetworkParameters.value();

                std::stringstream additionalEIPs;
                std::string separator(";");
                strings::join(params.AdditionalEIPs, additionalEIPs, separator);
                auto EIPs = additionalEIPs.str();
                auto xpubVersion = hex::toString(params.XPUBVersion);
                sql << "INSERT INTO ethereum_currencies VALUES(:name, :identifier, :chainID, :xpub, :prefix, :additionalBIPs)",
                        use(currency.name),
                        use(params.Identifier),
                        use(params.ChainID),
                        use(xpubVersion),
                        use(params.MessagePrefix),
                        use(EIPs);
                break;
            }
            case api::WalletType::RIPPLE: {
                const auto &params = currency.rippleLikeNetworkParameters.value();

                std::stringstream additionalRIPs;
                std::string separator(";");
                strings::join(params.AdditionalRIPs, additionalRIPs, separator);
                auto RIPs = additionalRIPs.str();
                auto xpubVersion = hex::toString(params.XPUBVersion);
                sql << "INSERT INTO ripple_currencies VALUES(:name, :identifier, :xpub, :prefix, :additionalRIPs)",
                        use(currency.name),
                        use(params.Identifier),
                        use(xpubVersion),
                        use(params.MessagePrefix),
                        use(RIPs);
                break;
            }
            case api::WalletType::TEZOS: {
                const auto &params = currency.tezosLikeNetworkParameters.value();
                std::stringstream additionalTIPs;
                std::string separator(";");
                strings::join(params.AdditionalTIPs, additionalTIPs, separator);
                auto TIPs = additionalTIPs.str();
                auto xpubVersion = hex::toString(params.XPUBVersion);
                auto implicitPrefix = hex::toString(params.ImplicitPrefix);
                auto originatedPrefix = hex::toString(params.OriginatedPrefix);
                sql << "INSERT INTO tezos_currencies VALUES(:name, :identifier, :xpub, :implicit_prefix, :originated_prefix, :prefix, :additionalTIPs)",
                        use(currency.name),
                        use(params.Identifier),
                        use(xpubVersion),
                        use(implicitPrefix),
                        use(originatedPrefix),
                        use(params.MessagePrefix),
                        use(TIPs);
                break; // TODO INSERT MONERO NETWORK PARAMS
            }
            case api::WalletType::MONERO:break;
            case api::WalletType::ALGORAND: {
                const auto &params = currency.algorandNetworkParameters.value();
                sql << "INSERT INTO algorand_currencies VALUES(:name, :genesis_id, :genesis_hash)",
                    use(currency.name),
                    use(params.genesisID),
                    use(params.genesisHash)
                ;
                break;
            }
            case api::WalletType::STELLAR: {
                const auto &params = currency.stellarLikeNetworkParameters.value();

                std::stringstream seps;
                std::string separator(";");
                auto addrVersion = hex::toString(params.Version);
                auto muxedAddrVersion = hex::toString(params.MuxedVersion);
                strings::join(params.AdditionalSEPs, seps, separator);
                auto SEPs = seps.str();

                sql << "INSERT INTO stellar_currencies VALUES(:name, :identifier, :version, :reserve, :fee, :pass, "
                       ":seps, :muxed_version)",
                    use(currency.name),
                    use(params.Identifier),
                    use(addrVersion),
                    use(params.BaseReserve),
                    use(params.BaseFee),
                    use(params.NetworkPassphrase),
                    use(SEPs),
                    use(muxedAddrVersion)
                ;
                break;
            }
        }
        inserted = true;
    }
    insertUnits(sql, currency);
    return inserted;
}

bool ledger::core::CurrenciesDatabaseHelper::insertERC20Token(soci::session &sql,
                                                              const ledger::core::api::ERC20Token &token) {

    auto count = 0;
    sql << "SELECT COUNT(*) FROM erc20_tokens WHERE contract_address = :address",
            soci::use(token.contractAddress),
            soci::into(count);
    if (count == 0) {
        sql << "INSERT INTO erc20_tokens VALUES(:contract_address, :name, :symbol, :number_of_decimal)",
                use(token.contractAddress),
                use(token.name),
                use(token.symbol),
                use(token.numberOfDecimal);
        return true;
    }
    return false;
}

void ledger::core::CurrenciesDatabaseHelper::getAllCurrencies(soci::session &sql,
                                                              std::vector<ledger::core::api::Currency> &currencies) {
    rowset<row> rows = (sql.prepare << "SELECT currencies.name, currencies.type, currencies.bip44_coin_type, currencies.payment_uri_scheme FROM currencies ");
    for (auto& currency_row : rows) {
        auto offset = 0;
        api::Currency currency;
        currency.name = currency_row.get<std::string>(0);
        currency.walletType = api::from_string<api::WalletType>(currency_row.get<std::string>(1));
        currency.bip44CoinType = currency_row.get<int32_t>(2);
        currency.paymentUriScheme = currency_row.get_indicator(3) == i_null ? "" : currency_row.get<std::string>(3);
        switch (currency.walletType) {
            case api::WalletType::BITCOIN: {
                rowset<row> btc_rows = (sql.prepare << "SELECT bitcoin_currencies.p2pkh_version, bitcoin_currencies.p2sh_version, bitcoin_currencies.xpub_version,"
                                                    " bitcoin_currencies.dust_amount, bitcoin_currencies.fee_policy, bitcoin_currencies.has_timestamped_transaction,"
                                                    " bitcoin_currencies.message_prefix, bitcoin_currencies.identifier, bitcoin_currencies.timestamp_delay,"
                                                    " bitcoin_currencies.sighash_type, bitcoin_currencies.additional_BIPs "
                                                    " FROM bitcoin_currencies "
                                                    " WHERE bitcoin_currencies.name = :currency_name", use(currency.name));
                for (auto& btc_row : btc_rows) {
                    api::BitcoinLikeNetworkParameters params;
                    params.P2PKHVersion = hex::toByteArray(btc_row.get<std::string>(0));
                    params.P2SHVersion = hex::toByteArray(btc_row.get<std::string>(1));
                    params.XPUBVersion = hex::toByteArray(btc_row.get<std::string>(2));
                    /*
                     * On Linux, if we use int64_t, we get std::bad_cast exception thrown,
                     * so we replace by a long long (which is supported by soci (soci::dt_long_long))
                     */
                    params.DustAmount = btc_row.get<long long>(3);
                    params.FeePolicy = api::from_string<api::BitcoinLikeFeePolicy>(btc_row.get<std::string>(4));
                    params.UsesTimestampedTransaction = btc_row.get<int>(5) == 1;
                    params.MessagePrefix = btc_row.get<std::string>(6);
                    params.Identifier = btc_row.get<std::string>(7);
                    params.TimestampDelay = btc_row.get<long long>(8);
                    params.SigHash = hex::toByteArray(btc_row.get<std::string>(9));
                    params.AdditionalBIPs = strings::split(btc_row.get<std::string>(10), ",");
                    currency.bitcoinLikeNetworkParameters = params;
                }
                break;
            }
            case api::WalletType::COSMOS: {
                rowset<row> cosmos_rows =
                    (sql.prepare
                         << "SELECT cosmos_currencies.name, cosmos_currencies.identifier, "
                            "cosmos_currencies.xpub_version, cosmos_currencies.pubkey_prefix, "
                            "cosmos_currencies.address_prefix, cosmos_currencies.message_prefix, "
                            "cosmos_currencies.chain_id, cosmos_currencies.additional_CIPs"
                            " FROM cosmos_currencies "
                            " WHERE cosmos_currencies.name = :currency_name",
                     use(currency.name));
                for (auto &cosmos_row : cosmos_rows) {
                    api::CosmosLikeNetworkParameters params;
                    params.Identifier = cosmos_row.get<std::string>(1);
                    params.XPUBVersion = hex::toByteArray(cosmos_row.get<std::string>(2));
                    params.PubKeyPrefix = hex::toByteArray(cosmos_row.get<std::string>(3));
                    params.AddressPrefix = hex::toByteArray(cosmos_row.get<std::string>(4));
                    params.MessagePrefix = cosmos_row.get<std::string>(5);
                    params.ChainId = cosmos_row.get<std::string>(6);
                    params.AdditionalCIPs = strings::split(cosmos_row.get<std::string>(7), ",");
                    currency.cosmosLikeNetworkParameters = params;
                }
                break;
            }
            case api::WalletType::ETHEREUM: {
                rowset<row> eth_rows = (sql.prepare << "SELECT ethereum_currencies.chain_id, ethereum_currencies.xpub_version,"
                        " ethereum_currencies.message_prefix, ethereum_currencies.identifier,"
                        " ethereum_currencies.additional_EIPs "
                        " FROM ethereum_currencies "
                        " WHERE ethereum_currencies.name = :currency_name", use(currency.name));
                for (auto& eth_row : eth_rows) {
                    api::EthereumLikeNetworkParameters params;
                    params.ChainID = eth_row.get<std::string>(0);
                    params.XPUBVersion = hex::toByteArray(eth_row.get<std::string>(1));
                    params.MessagePrefix = eth_row.get<std::string>(2);
                    params.Identifier = eth_row.get<std::string>(3);
                    params.AdditionalEIPs = strings::split(eth_row.get<std::string>(4), ",");
                    currency.ethereumLikeNetworkParameters = params;
                }

                break;
            }
            case api::WalletType::RIPPLE: {
                rowset<row> ripple_rows = (sql.prepare << "SELECT ripple_currencies.xpub_version,"
                        " ripple_currencies.message_prefix, ripple_currencies.identifier,"
                        " ripple_currencies.additional_RIPs "
                        " FROM ripple_currencies "
                        " WHERE ripple_currencies.name = :currency_name", use(currency.name));
                for (auto& ripple_row : ripple_rows) {
                    api::RippleLikeNetworkParameters params;
                    params.XPUBVersion = hex::toByteArray(ripple_row.get<std::string>(0));
                    params.MessagePrefix = ripple_row.get<std::string>(1);
                    params.Identifier = ripple_row.get<std::string>(2);
                    params.AdditionalRIPs = strings::split(ripple_row.get<std::string>(3), ",");
                    currency.rippleLikeNetworkParameters = params;
                }

                break;
            }
            case api::WalletType::TEZOS: {
                rowset<row> tezos_rows = (sql.prepare << "SELECT tezos_currencies.xpub_version, tezos_currencies.implicit_prefix,"
                        " tezos_currencies.originated_prefix, tezos_currencies.message_prefix,"
                        " tezos_currencies.identifier, tezos_currencies.additional_TIPs"
                        " FROM tezos_currencies"
                        " WHERE tezos_currencies.name = :currency_name", use(currency.name));
                for (auto &tezos_row : tezos_rows) {
                    api::TezosLikeNetworkParameters params;
                    params.XPUBVersion = hex::toByteArray(tezos_row.get<std::string>(0));
                    params.ImplicitPrefix = hex::toByteArray(tezos_row.get<std::string>(1));
                    params.OriginatedPrefix = hex::toByteArray(tezos_row.get<std::string>(2));
                    params.MessagePrefix = tezos_row.get<std::string>(3);
                    params.Identifier = tezos_row.get<std::string>(4);
                    params.AdditionalTIPs = strings::split(tezos_row.get<std::string>(5), ",");
                    currency.tezosLikeNetworkParameters = params;
                }

                break;
            }
            case api::WalletType::MONERO:break;
            case api::WalletType::ALGORAND: {
                rowset<row> algorand_rows = (sql.prepare << "SELECT genesis_id, genesis_hash FROM algorand_currencies "
                                                           "WHERE name = :name"
                        , use(currency.name));
                for (auto& algorand_row : algorand_rows) {
                    api::AlgorandNetworkParameters params;
                    params.genesisID = algorand_row.get<std::string>(0);
                    params.genesisHash = algorand_row.get<std::string>(1);
                    currency.algorandNetworkParameters = params;
                }
                break;
            }
            case api::WalletType::STELLAR: {
                rowset<row> stellar_rows = (sql.prepare << "SELECT identifier, address_version, base_reserve,"
                                                           "base_fee, network_passphrase, additional_SEPs, "
                                                           "muxed_address_version "
                                                           "FROM stellar_currencies "
                                                           "WHERE name = :name"
                        , use(currency.name));
                for (auto& stellar_row : stellar_rows) {
                    api::StellarLikeNetworkParameters params;
                    params.Identifier = stellar_row.get<std::string>(0);
                    params.Version = hex::toByteArray(stellar_row.get<std::string>(1));
                    params.BaseFee = soci::get_number<int64_t>(stellar_row, 2);
                    params.BaseReserve = soci::get_number<int64_t>(stellar_row, 3);
                    params.NetworkPassphrase = stellar_row.get<std::string>(4);
                    params.AdditionalSEPs = strings::split(stellar_row.get<std::string>(5), ",");
                    params.MuxedVersion = hex::toByteArray(stellar_row.get<std::string>(6));
                    currency.stellarLikeNetworkParameters = params;
                }
                break;
            }
        }
        getAllUnits(sql, currency);
        currencies.push_back(currency);
    }
}

void ledger::core::CurrenciesDatabaseHelper::getAllUnits(soci::session &sql, ledger::core::api::Currency &currency) {

    rowset<row> rows = (sql.prepare <<
                                    "SELECT name, magnitude, symbol, code FROM units WHERE currency_name = :currency", use(currency.name));
    for (auto& row : rows) {
        api::CurrencyUnit unit;
        unit.name = row.get<std::string>(0);
        unit.numberOfDecimal = row.get<int32_t>(1);
        unit.symbol = row.get<std::string>(2);
        unit.code = row.get<std::string>(3);
        currency.units.push_back(unit);
    }
}

void
ledger::core::CurrenciesDatabaseHelper::insertUnits(soci::session &sql, const ledger::core::api::Currency &currency) {
    for (const auto& unit : currency.units) {
        int count;
        sql << "SELECT COUNT(*) FROM units WHERE name = :name AND currency_name = :cname", use(unit.name), use(currency.name), into(count);
        if (count == 0) {
            sql << "INSERT INTO units VALUES(:name, :magnitude, :symbol, :code, :currency_name)",
            use(unit.name), use(unit.numberOfDecimal), use(unit.symbol), use(unit.code), use(currency.name);
        }
    }
}

void ledger::core::CurrenciesDatabaseHelper::removeCurrency(soci::session &sql, const std::string &currencyName) {
    sql << "DELETE FROM currencies WHERE name = :name", use(currencyName);
}
