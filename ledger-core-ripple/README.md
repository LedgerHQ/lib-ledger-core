# Ledger Core Ripple (XRP) library

This library provides everything you need to work with the [Ripple] currency and all of its
child derived currencies.

## How to build

This project depends on [ledger-core]. In order to build it, you must have the public API of
[ledger-core] available. You don’t necessarily have to build the whole library; the sole interface
is needed.

From the root folder of this git repository, run the following commands:

  1. `./tools/idl_interfaces.sh ledger-core # This will generate the needed interface, if not yet generated`
  2. `./tools/idl_interfaces.sh ledger-core ledger-core-ripple # This will generate the Ripple API`
  3. `cd ledger-core-ripple`
  4. `mkdir build && cd build # create a build directory if none exists yet`
  5. `cmake -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=YES .. # This will configure the project in debug mode and output symbols locations for your editor (remove it if not needed)`
  6. `make -j8 # build with 8 threads`

Once everything is compiled, you can start playing with the C++ library or generating foreign
bindings.

## Using the Ledger Core Ripple library

### Native

**TODO**

### Foreign bindings

**TODO**

## Architecture

This project is organized around several concepts:

  - `idl`: directory containing all the Intermediate Definition Language files. Those are the first
    things you should read if you want to know about the contracts and public APIs of Ripple with
    Ledger Core. IDLs are used to generate foreign code for your favorite language, such as
    [node.js] or [React Native], for instance.
  - `src`: contains the code of the library. The code here is very specific to Ripple and implements
    interfaces defined in the `api` subfolder.
    - `api`: contains generated code from the root `idl` folder. **You must not add any files or
      edit any existing files in this directory, as it will always be cleansed by the IDL interfaces
      generator.**
    - The rest of the folder is composed with specific code to Ripple that implements the
      interfaces.
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
[ledger-core]: ../ledger-core
[node.js]: https://nodejs.org
[React Native]: https://facebook.github.io/react-native
