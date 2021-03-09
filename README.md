# Ledger Core Library

* [Clone project](#clone-project)
* [Dependencies](#dependencies)
    * [Build](#build)
    * [External dependencies:](#external-dependencies)
* [Build of C++ library](#build-of-c-library)
    * [Building for JNI](#building-for-jni)
    * [Build library with PostgreSQL](#build-library-with-postgresql)
* [Documentation](#documentation)
* [Binding to node.js](#binding-to-nodejs)
    * [Using the node module](#using-the-node-module)
    * [Generating a new node module for your system](#generating-a-new-node-module-for-your-system)
* [Support](#support)
    * [Libcore:](#libcore)
    * [Bindings:](#bindings)
* [Developement guidelines](#developement-guidelines)
    * [CI](#ci)
* [Q/A and troubleshooting](#qa-and-troubleshooting)
    * [I have updated an include file and test code doesn’t see the changes!](#i-have-updated-an-include-file-and-test-code-doesnt-see-the-changes)
    * [I have upgraded my macOSX system and now I can’t compile anymore.](#i-have-upgraded-my-macosx-system-and-now-i-cant-compile-anymore)

Core library which will be used by Ledger applications.

> If you’re a developer and want to contribute, please refer to our [contribution guidelines]
> specific documentation.

## Clone project

```
git clone --recurse-submodules https://github.com/LedgerHQ/lib-ledger-core.git
```

> If you had already forked / cloned the repository before issuing that command, it’s okay. You can
> initiate the submodules with the following commands:

```
cd lib-ledger-core
git submodule init
git submodule update
```

## Dependencies

### Build

This project is based on **_cmake_** as a build system so you should install it before starting (at least version 3.7).

### External dependencies:

* [OpenSSL](https://www.openssl.org/) is needed to build tests of the library.
* Generation of binding is automated with [Djinni](https://github.com/dropbox/djinni).
* Build on multiple Operating Systems is based on [polly](https://github.com/ruslo/polly) toolchains.

## Build of C++ library

**_cmake_** is building out of source, you should create a build directory (e.g. `lib-ledger-core-build`):

	.                           # Directory where clone command was launched
    ├── lib-ledger-core         # Source files directory
    ├── lib-ledger-core-build   # Build directory

If you respect this folder structure (and naming), after `cd lib-ledger-core-build`, you can build the library by running:

```
cmake -DSYS_OPENSSL=ON -DOPENSSL_ROOT_DIR=<path-to-openssl-root-dir>  -DOPENSSL_INCLUDE_DIR=<path-to-openssl-include-files>  -DOPENSSL_SSL_LIBRARIES=<path-to-openssl-libraries> -DOPENSSL_USE_STATIC_LIBS=TRUE ../lib-ledger-core && make
```
NB. if you want to build on `Windows` with Visual Studio by adding the argument `-G "Visual Studio 16 2019"` in the above cmake command, instead of using `make` to build the project, you should open the 'ledger-core.sln' solution file with Visual Studio and build the solution with it

> If you struggle with how openssl is installed, for example, on `macOSX`, `openssl` can be installed with
```
brew install openssl
```
you can then use the argument `-DOPENSSL_ROOT_DIR=/usr/local/opt/openssl` in the above cmake command
"DOPENSSL_INCLUDE_DIR" and "DOPENSSL_SSL_LIBRARIES" are not necessary on mac.

> On `Linux`, 
```
apt-get install libssl-dev
```
you can then use the argument `-DOPENSSL_SSL_LIBRARIES=/usr/lib/x86_64-linux-gnu -DOPENSSL_INCLUDE_DIR=/usr/include/openssl` in the above cmake command
"DOPENSSL_ROOT_DIR" is not necessary on linux.

> On `Windows`, 
Openssl can be downloaded and installed from https://slproweb.com/products/Win32OpenSSL.html
"DOPENSSL_ROOT_DIR" is then the installed path of Openssl in the above cmake command
"DOPENSSL_INCLUDE_DIR" and "DOPENSSL_SSL_LIBRARIES" are not necessary on windows.

Several CMake arguments might interest you there:

  - `-DCMAKE_BUILD_TYPE=Debug`: you should always set that when testing as you will get DWARF debug
    symbols and debugging instruments support.
  - `-DCMAKE_EXPORT_COMPILE_COMMANDS=YES`: useful when you’re using a C++ linter, such as [cquery].
  - `-G "Visual Studio 16 2019"`: build libcore with Visual Studio on Windows
  - `-G Xcode`: build libcore with Xcode on Mac
  - `-DBUILD_TESTS=OFF`: build libcore without unit tests. In this case, openssl arguments are not needed

### Building for JNI

Building with JNI (Java Native Interface), allows you to use the library with Java based software. In order to enable JNI mode use

```
cmake -DTARGET_JNI=ON
```

This will add JNI files to the library compilation and remove tests. You need at least a JDK 7 to build for JNI (OpenJDK or Oracle JDK)

### Build library with PostgreSQL

#### Dependencies

Make sure that your have `PostgreSQL` installed on your machine, otherwise the `CMake` 
command `find_package(PostgreSQL REQUIRED)` will fail during configuration.

#### Build

To compile libcore with PostgreSQL support, you should add `-DPG_SUPPORT=ON` to your 
`CMake` configuration command.

You also need to add `-DPostgreSQL_INCLUDE_DIR=path/to/include/dir` in your configuration
as a hint for headers' location (e.g. `/usr/include/postgresql`).

#### Wallet Pool Configuration

To use with libcore, simply set value of the key `api::PoolConfiguration::DATABASE_NAME`
to the database's URL connection and set it in the pool's configuration.

It is also possible to configure the size of the connection pool and read-only connection pool when instantiating the 
PostgreSQL `DatabaseBackend` : `api::DatabaseBackend::getPostgreSQLBackend(int32_t connectionPoolSize, int32_t readonlyConnectionPoolSize)`.

#### Local testing

If you don't build the library with PostgreSQL, sqlite3 shall be used as Database.
If you build the library with PostgreSQL, make sure to have a running PostgreSQL server or PostgreSQL docker container
As an example, if you are running it on `localhost:5432` and `test_db` as database name,
database's name forwarded to the pool (through configuration key `api::PoolConfiguration::DATABASE_NAME`)  
should look like : `postgres://localhost:5432/test_db` .  
In order to run local tests
```
cd lib-ledger-core-build
```
> On `Linux` or `macOSX`,
```
ctest
```
> On `Windows`,
```
ctest -C Debug -VV
```
if you want to run only one specific unit test. (e.g. the test case `BitcoinLikeWalletSynchronization.MediumXpubSynchronization` in the test project `ledger-core-integration-tests`)
```
./core/test/integration/build/ledger-core-integration-tests "--gtest_filter=BitcoinLikeWalletSynchronization.MediumXpubSynchronization"
```

## Documentation

You can generate the Doxygen documentation by running the `doc` target (for instance, `make doc`
with makefiles).

## Binding to node.js

The library can be compiled and integrated as an node module in a pretty straightforward way. You
will be interested in either using it, or making a new version of the node module.

### Using the node module

The [lib-ledger-core-node-bindings] repository contains the node.js bindings you will need to
interface with `lib-ledger-core`. You can either clone the git repository or simply install from
`npm` directly:

```
npm i @ledgerhq/ledger-core
```

### Generating a new node module for your system

Generating bindings is a several steps process:

  1. First, you need to make some changes to `lib-ledger-core` and generate a fresh version of
     `lib-ledger-core`.
  2. Clone [lib-ledger-core-node-bindings] and edit the `package.json` file in order to remove or
     comment the `"preinstall"` line in `"scripts"`.
  3. In the folder of `lib-ledger-core`, run the `tools/generateBindings.sh` script by giving it the
     path to the bindings (i.e. where you cloned [lib-ledger-core-node-bindings]) and as second
     argument the path to the directory where you built the `lib-ledger-core` — it should be
     something like `$(your-lib-ledger-core-dir)/../lib-ledger-core-build` or
     `$(your-lib-ledger-core-dir)/build`.
       - This script requires an up-to-date **djinni**. To ensure it’s correctly up to date, go
         into `lib-ledger-core/djinni` and run
         `get fetch origin --prune && git rebase origin/master`.
       - You will need `sbt` and `java8` for a complete, working install.
       - The script will generate files in both projects. You’re advised to remove the ones created
         in `lib-ledger-core` — if any — with a `git checkout .` and/or `git reset .`.
  4. `cd` into `lib-ledger-core-bindings` and run `yarn` to generate the bindings.
  5. You will have the module in `build/Release/ledgerapp_nodejs.node` in the bindings project.
  6. `npm i` should install your own version.

## Support

### Libcore:
Libcore can be built for following OSes:
 - MacOS: minimum supported version is `macOS 9.0`, with `x86_64` architecture,
 - Linux: Debian (stretch), Ubuntu and Arch are supported, with `x86_64` architecture,
 - Windows: 64-bit architecture is built with `MSVC` (starting from Visual Studio 15), 32-bit is built with `MinGW`,
 - iOS: `x86_64`, `armv7` and `arm64` architectures are supported, minimum supported version is `iOS 10.0`,
 - Android: `x86`, `armeabi-v7a` and `arm64-v8a` architectures are supported, minimum supported version is `Android 7 (API 24)` (Java 8 is needed).
### Bindings:
 - NodeJS bindings:
   - Please use `node` with version `>=8.4.0` and `<9.0.0` (other versions are not tested (yet)),
   - Node-gyp is used to build native module and requires `python` with version `2.7.x`.

## Developement guidelines

### CI

You are advised to link your GitHub account to both [CircleCI] and [Appveyor] by signing-in. Because
we are using shared runners and resources, we have to share CI power with other teams. It’s
important to note that we don’t always need to run the CI. Example of situations when we do not need
it:

  - When we are updating documentation.
  - When we are changing a tooling script that is not part of any testing suite (yet).
  - When we are making a *WIP* PR that doesn’t require running the CI until everyone has agreed on
    the code (this is a tricky workflow but why not).

In those cases, please include the `[skip ci]` or `[ci skip]` text **in your commit message’s
title**. You could tempted to put it in the body of your message but that will not work with
[Appveyor].

Finally, it’s advised to put it on every commit and rebase at the end to remove the `[skip ci]` tag
from your commits’ messags to have the CI re-enabled, but some runners might be smart enough to do
it for all commits in the PR.

Rebasing is done easily. If your PR wants to merge `feature/stuff -> develop`, you can do something
like this — assuming you have cloned the repository with a correctly set `origin` remote:

```
git checkout feature/stuff
git rebase -i origin/develop
```

Change the `pick` to `r` or `reword` at the beginning of each lines **without changing the text of
the commits** — this has no effect. Save the file and quit. You will be prompted to change the
commits’ messages one by one, allowing you to remove the `[skip ci]` tag from all commits.

## Q/A and troubleshooting

### I have updated an include file and test code doesn’t see the changes!

Currently, interface files (headers, .hpp) **are not linked by copied directly into the test
directory**. That means that every time you make a change in the interface that is tested by any
code in core/test/, you need to update the copy.

Just run this command:

```
cd $your_build_folder
rm -rf CMakeFiles CMakeCache.txt
```

### I have upgraded my macOSX system and now I can’t compile anymore.

Especially if you’ve upgraded to Mojave for which there are some breaking changes, you will need to
perform some manual tasks — here, for macOSX Mojave:

```
xcode-select --install
open /Library/Developer/CommandLineTools/Packages/macOS_SDK_headers_for_macOS_10.14.pkg
```

[contribution guidelines]: ./CONTRIBUTING.md
[lib-ledger-core-node-bindings]: https://github.com/LedgerHQ/lib-ledger-core-node-bindings
[CircleCI]: https://circleci.com
[Appveyor]: https://www.appveyor.com
[cquery]: https://github.com/cquery-project/cquery
