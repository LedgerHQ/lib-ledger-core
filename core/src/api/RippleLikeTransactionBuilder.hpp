// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from ripple_like_wallet.djinni

#ifndef DJINNI_GENERATED_RIPPLELIKETRANSACTIONBUILDER_HPP
#define DJINNI_GENERATED_RIPPLELIKETRANSACTIONBUILDER_HPP

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
class RippleLikeTransaction;
class RippleLikeTransactionCallback;
struct Currency;
struct RippleLikeMemo;

class LIBCORE_EXPORT RippleLikeTransactionBuilder {
public:
    virtual ~RippleLikeTransactionBuilder() {}

    /**
     * Send funds to the given address. This method can be called multiple times to send to multiple addresses.
     * @param amount The value to send
     * @param address Address of the recipient
     * @return A reference on the same builder in order to chain calls.
     */
    virtual std::shared_ptr<RippleLikeTransactionBuilder> sendToAddress(const std::shared_ptr<Amount> & amount, const std::string & address) = 0;

    /**
     * Send all available funds to the given address.
     * @param address Address of the recipient
     * @return A reference on the same builder in order to chain calls.
     */
    virtual std::shared_ptr<RippleLikeTransactionBuilder> wipeToAddress(const std::string & address) = 0;

    /**
     * Set fees (in drop) the originator is willing to pay
     * @return A reference on the same builder in order to chain calls.
     */
    virtual std::shared_ptr<RippleLikeTransactionBuilder> setFees(const std::shared_ptr<Amount> & fees) = 0;

    /**
     * Set correlation id
     * @return A reference on the same builder in order to chain calls.
     */
    virtual std::shared_ptr<RippleLikeTransactionBuilder> setCorrelationId(const std::string & correlationId) = 0;

    /**
     * Add a memo.
     * @return A reference on the same builder in order to chain calls.
     */
    virtual std::shared_ptr<RippleLikeTransactionBuilder> addMemo(const RippleLikeMemo & memo) = 0;

    /** An arbitrary unsigned 32-bit integer that identifies a reason for payment or a non-Ripple account */
    virtual std::shared_ptr<RippleLikeTransactionBuilder> setDestinationTag(int64_t tag) = 0;

    /** Build a transaction from the given builder parameters. */
    virtual void build(const std::shared_ptr<RippleLikeTransactionCallback> & callback) = 0;

    /**
     * Creates a clone of this builder.
     * @return A copy of the current builder instance.
     */
    virtual std::shared_ptr<RippleLikeTransactionBuilder> clone() = 0;

    /** Reset the current instance to its initial state */
    virtual void reset() = 0;

    static std::shared_ptr<RippleLikeTransaction> parseRawUnsignedTransaction(const Currency & currency, const std::vector<uint8_t> & rawTransaction);

    static std::shared_ptr<RippleLikeTransaction> parseRawSignedTransaction(const Currency & currency, const std::vector<uint8_t> & rawTransaction);
};

} } }  // namespace ledger::core::api
#endif //DJINNI_GENERATED_RIPPLELIKETRANSACTIONBUILDER_HPP
