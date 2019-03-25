## 2.7.0 (on-going)

> ????/??/??

- Native Segwit support:
    - Implement P2WPKH and P2WSH keychains, references:
        - BIP141: https://github.com/bitcoin/bips/blob/master/bip-0141.mediawiki#P2WSH
        - BIP143: https://github.com/bitcoin/bips/blob/master/bip-0143.mediawiki
    - Support of Bech32 addresses, references:
         - BIP173: https://github.com/bitcoin/bips/blob/master/bip-0173.mediawiki#Segwit_address_format)
         - BTC Bech32: https://github.com/sipa/bech32/tree/master/ref
         - BCH Bech32: https://github.com/bitcoincashjs/cashaddrjs
- Add `ERC20LikeAccount::getBalanceHistoryFor`, a method used to retrieve the balance history of an
  ERC20 account using inclusive date ranges.
- Fix some bugs related to JNI when compiling for Java / Android.
- The library now builds with a compiled version of
  [SQLCipher](https://github.com/sqlcipher/sqlcipher) and the Windows code-path is now patched.
- Improve the cross-compilation support of the library to allow to compile to several platforms in
  parallel. This is not perfect and needs a few more love to be complete but we’re getting there.
  This is especially useful when compiling for mobile (emulator x86, several variations of ARM
  depending on your phone, etc.).
- XRP is fully integrated.
- Full support of password change with `WalletPool::changePassword`. It is now possible to change
  one’s password by calling this function. The previous work from 2.6.0 that wasn’t exposed is now
  exposed and calling that function will change both the encryption on the database and the
  preferences.
- Some Windows-related fix (compilers, mostly).
- [fmt](https://github.com/fmtlib/fmt) is now a submodule of ours.

## 2.6.0

> 2019/02/22

- Change the internal representation of `DynamicObject`. The new representation is optimized not
  only memory-wise but also type-safety-wise. The internal interfaces were also redesigned to allow
  for a smoother transition to C++17’s `std::variant` when we see fit.
- Add support for changing password in `Preferences`. This is not exposed directly via the
  interface: instead, you must use the password changing mechanisme of `WalletPool`. See below for
  further details.
- Implement sqlite3 database encryption through SQLCipher (https://github.com/sqlcipher/sqlcipher). If the
  `WalletPool::newInstance` is used with native (sqlite3) backend and called with a password (non-empty string),
  the database will be encrypted. To change password, a `changeDatabasePassword` is provided.
  SQLCipher is not available for Windows builds mainly because of build issues (C compiler containing spaces).
- Changed deployement process, now RCs are deployed with format `x.y.z-rc-{COMMIT_HASH}`, this will allow us
  not to always bump the versions when merging PRs and also keeping the binaries deployed to our bucket.
- Update used libraries, to be able to compile under VS2017.
- XRP integration:
    - XRP Keychian, address derivation and account creation.
    - XRP API explorer and parser.
    - XRP Node explorer and parser.
    - Accounts synchronization.
    - XRP transaction serialization and parsing.
    - Tests of above features.

## 2.5.0

> 2019/01/22

- Resetting the libcore to its fresh state is now possible via the `WalletPool::freshResetAll`
  function.

## 2.4.0

> 2019/01/22

- Add the possibility to switch off the internal logger of libcore — this rely on the
  `DynamicObject` that now has a new boolean configuration, `"ENABLE_INTERNAL_LOGGING"`. Set it
  to `true` to enable the internal logging (default), otherwise, set to `false` to completely
  disable internal logging (this also will prevent the core library from creating log files).

## 2.3.0

> 2019/01/21

- Change the way the library is compiled and linked by using explicit interface libraries. This
  also fixes a Windows issues related to non-explicitly exported symbols.

### 2.2.1

> 2019/02/07

- Backport a patch to fix encryption password in `WalletPool`. A bug was introduced in previous
  version (not affecting production) that caused a non-empty string to be accepted by a
  `WalletPool`, yield encryption. As that should allow for explicit encryption with empty passwords,
  we forbid that and empty password will result in no encryption.

## 2.2.0

> 2019/01/15

- Add a new mechanism to `DatabaseEngine`. This allows custom database driver, which allow switching
  between database backends easier.'

## 2.1.0

> 2019/01/15

- Implement `Preferences` encryption via LevelDB. Only the values are encrypted. Be careful
  though: this change doesn’t provide a solution when you want to reset or change your password
  (already encrypted values will remain still). A fix for this is foreseen later.

## 2.0.0

> 2019/01/04

- Support of new coin family: Ethereum-like currencies, with an integration of Ethereum as first
  currency of this family.
- Support of ERC20 tokens.
- We do not support other smart contracts which are not ERC20 yet; to be more specific, internal
  transactions are not caught during synchronization of Ethereum accounts.

# 0.9.0

> 2018/05/14

- First beta release of libledger-core for Ledger Vault integration
