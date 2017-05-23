// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from errors.djinni

package co.ledger.core;

public enum ErrorCode {
    /** Pool errors */
    UNKNOWN_NETWORK_PARAMETERS,
    /**
     * Device errors
     * Wallet errors
     */
    WALLET_NOT_FOUND,
    WALLET_ALREADY_EXISTS,
    RAW_TRANSACTION_NOT_FOUND,
    /** User land errors */
    CANCELLED_BY_USER,
    UNSUPPORTED_CURRENCY,
    CURRENCY_ALREADY_EXISTS,
    CURRENCY_NOT_FOUND,
    /** Others */
    INVALID_BASE58_FORMAT,
    INVALID_CHECKSUM,
    INVALID_VERSION,
    /** DeterministicPublicKey */
    PRIVATE_DERIVATION_NOT_SUPPORTED,
    /** Bitcoin error */
    INVALID_NETWORK_ADDRESS_VERSION,
    /** Generic */
    RUNTIME_ERROR,
    OUT_OF_RANGE,
    ILLEGAL_ARGUMENT,
    ILLEGAL_STATE,
    NULL_POINTER,
    UNSUPPORTED_OPERATION,
    UNKNOWN,
    IMPLEMENTATION_IS_MISSING,
    FUTURE_WAS_SUCCESSFULL,
    ALREADY_COMPLETED,
    NO_SUCH_ELEMENT,
    /** Preferences Error */
    UNABLE_TO_OPEN_LEVELDB,
    /** Network errors */
    NO_INTERNET_CONNECTIVITY,
    UNABLE_TO_RESOLVE_HOST,
    UNABLE_TO_CONNECT_TO_HOST,
    HTTP_ERROR,
    SSL_ERROR,
    TOO_MANY_REDIRECT,
    AUTHENTICATION_REQUIRED,
    HTTP_TIMEOUT,
    PROXY_ERROR,
    /** API errors */
    API_ERROR,
    TRANSACTION_NOT_FOUND,
    /** Format */
    INVALID_DATE_FORMAT,
    INVALID_DERIVATION_SCHEME,
    ;
}
