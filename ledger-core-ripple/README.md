# Ledger Core Ripple (XRP) library

This library provides everything you need to work with the [Ripple] currency and all of its
child derived currencies.

## Architecture

This project is organized around several concepts:

  - `idl`: directory containing all the Intermediate Definition Language files. Those are the first
    things you should read if you want to know about the contracts and public APIs of Ripple with
    Ledger Core. IDLs are used to generate foreign code for your favorite language, such as
    [node.js] or [React Native], for instance.
  - `src`: contains the code of the library.
  - `lib`: contains dependencies (libraries, forks, etc.) required for this project to build.

This project exposes the following concepts from Ripple:

  - [x] Ripple transactions.
  - [x] Ripple raw transactions (serialization / deserialization).
  - [x] Ripple accounts.
  - [x] Ripple addresses.
  - [x] Ripple extended public keys.
  - [x] Ripple network parameters.
  - [x] Ripple observers.
  - [x] Ripple synchronizers.

[Ripple]: https://ripple.com
[node.js]: https://nodejs.org
[React Native]: https://facebook.github.io/react-native
