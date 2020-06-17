// AUTOGENERATED FILE - DO NOT MODIFY!
// This file generated by Djinni from preferences.djinni

#ifndef DJINNI_GENERATED_PREFERENCESBACKEND_HPP
#define DJINNI_GENERATED_PREFERENCESBACKEND_HPP

#include "../utils/optional.hpp"
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

class RandomNumberGenerator;
struct PreferencesChange;

/** Interface describing the behaviour of the backend used by Preferences. */
class LIBCORE_EXPORT PreferencesBackend {
public:
    virtual ~PreferencesBackend() {}

    /**
     * Gets the value associated to the given key.
     * @param key The data key.
     * @return The value associated to the key if it exists, an empty option otherwise.
     */
    virtual std::experimental::optional<std::string> get(const std::vector<uint8_t> & key) = 0;

    /**
     * Commit a change.
     * @param changes The list of changes to commit.
     * @return false if unsuccessful (might happen if the underlying DB was destroyed).
     */
    virtual bool commit(const std::vector<PreferencesChange> & changes) = 0;

    /**
     * Turn encryption on for all future uses.
     * This method will set encryption on for all future values that will be persisted.
     * If this function is called on a plaintext storage (i.e. first encryption for
     * instance), it will also encrypt all data already present.
     * @param rng Random number generator used to generate the encryption salt.
     * @param password The new password.
     */
    virtual void setEncryption(const std::shared_ptr<RandomNumberGenerator> & rng, const std::string & password) = 0;

    /**
     * Turn off encryption by disabling the use of the internal cipher. Data is left
     * untouched.
     * This method is suitable when you want to get back raw, encrypted data. If you want
     * to disable encryption in order to read clear data back without password, consider
     * the resetEncryption method instead.
     */
    virtual void unsetEncryption() = 0;

    /**
     * Reset the encryption with a new password by first decrypting on the
     * fly with the old password the data present.
     * If the new password is an empty string, after this method is called, the database
     * is completely unciphered and no password is required to read from it.
     * @return true if the reset occurred correctly, false otherwise (e.g. trying to change
     * password with an old password but without a proper salt already persisted).
     */
    virtual bool resetEncryption(const std::shared_ptr<RandomNumberGenerator> & rng, const std::string & oldPassword, const std::string & newPassword) = 0;

    /**
     * Get encryption salt, if any.
     * @return the encryption salt if it exists, an empty string otherwise.
     */
    virtual std::string getEncryptionSalt() = 0;

    /** Clear all preferences. */
    virtual void clear() = 0;
};

} } }  // namespace ledger::core::api
#endif //DJINNI_GENERATED_PREFERENCESBACKEND_HPP
