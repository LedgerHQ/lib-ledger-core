// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from wallet.djinni

#ifndef DJINNI_GENERATED_COSMOSLIKETRANSACTION_HPP
#define DJINNI_GENERATED_COSMOSLIKETRANSACTION_HPP

#include <chrono>
#include <cstdint>
#include <memory>
#include <string>
#include <vector>
#ifndef LIBCORE_EXPORT
    #if defined(_MSC_VER)
       #include <libcore_export.h>
    #else
       #define LIBCORE_EXPORT
    #endif
#endif

namespace ledger { namespace core { namespace api {

class Amount;
class BigInt;
class CosmosLikeMessage;

/**Class representing a Cosmos transaction */
class LIBCORE_EXPORT CosmosLikeTransaction {
public:
    virtual ~CosmosLikeTransaction() {}

    /** Get the time when the transaction was issued or the time of the block including this transaction */
    virtual std::chrono::system_clock::time_point getDate() const = 0;

    /** Get Fee (in drop) */
    virtual std::shared_ptr<Amount> getFee() const = 0;

    /** Get gas Wanted (maximum gas advertised in transaction) */
    virtual std::shared_ptr<Amount> getGas() const = 0;

    /** Get gas used (gas actually used in the transaction) */
    virtual std::shared_ptr<BigInt> getGasUsed() const = 0;

    /** Get gas Wanted (in BigInt form) */
    virtual std::shared_ptr<BigInt> getGasWanted() const = 0;

    /** Get the hash of the transaction. */
    virtual std::string getHash() const = 0;

    /** Get memo */
    virtual std::string getMemo() const = 0;

    /** Get the list of messages */
    virtual std::vector<std::shared_ptr<CosmosLikeMessage>> getMessages() const = 0;

    /** Get Signing public Key */
    virtual std::vector<uint8_t> getSigningPubKey() const = 0;

    /** Get Signing public Key */
    virtual std::string getCorrelationId() const = 0;

    /**
     * Set the correlation id which can be used to debug transaction errors
     * through the full ledger stack
     * @return the OLD Correlation ID, if it was set (empty string if it was unset)
     */
    virtual std::string setCorrelationId(const std::string & correlationId) = 0;

    /** Serialize the transaction to be signed */
    virtual std::string serializeForSignature() = 0;

    /** Set signature of transaction, when a signature is set it can be broadcasted */
    virtual void setSignature(const std::vector<uint8_t> & rSignature, const std::vector<uint8_t> & sSignature) = 0;

    virtual void setDERSignature(const std::vector<uint8_t> & signature) = 0;

    /**
     * Serialize the transaction to be broadcast
     * @param mode The supported broadcast modes include
     *        "block"(return after tx commit), (https://docs.cosmos.network/master/basics/tx-lifecycle.html#commit)
     *        "sync"(return afer CheckTx), (https://docs.cosmos.network/master/basics/tx-lifecycle.html#types-of-checks) and
     *        "async"(return right away).
     * @return string the json payload to broadcast on the network
     */
    virtual std::string serializeForBroadcast(const std::string & mode) = 0;
};

} } }  // namespace ledger::core::api
#endif //DJINNI_GENERATED_COSMOSLIKETRANSACTION_HPP
