### [Next Release Here]

> 2019/??/??

- Change the internal representation of `DynamicObject`. The new representation is optimized to
  take less memory and to be safer in terms of typing. The internal interfaces were also
  redesigned to allow for a smoother transition to C++17’s `std::variant` when we see fit.

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

- Change the way the library is compiled and linked by using explicti interface libraries. This
  also fixes a Windows issues related to non-explicitly exported symbols.

## 2.2.0

> 2019/01/15

- Add a new mechanism to `DatabaseEngine`. This allows for custom database driver, which should
  allow database switching in easier ways.

## 2.1.0

> 2019/01/15

- Implement `Preferences` encryption via LevelDB. Only the values are encrypted. Be careful
  though: this change doesn’t provide a solution when you want to reset or change your password
  (already encrypted values will remain still). A fix for this is foreseen later.

## 2.0.0

> 2019/01/04

- Integration of Ethereum as an officially supported currency and currency family into libcore.
  This version has breaking changes a bit everywhere that must be addressed by applications due
  to the refactoring that was necessary for this change to happen.

# 0.9.0

> 2018/05/14

- First beta release of libledger-core for Ledger Vault integration
