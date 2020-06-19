# Ledger Core Library

* [Clone project](#clone-project)
* [Dependencies](#dependencies)
    * [Build](#build)
    * [External dependencies:](#external-dependencies)
* [Build of C++ library](#build-of-c-library)
    * [Building for JNI](#building-for-jni)
    * [Build library on docker](#build-library-on-docker)
    * [Build library with OpenSSL](#build-library-with-openssl)
    * [Build library with PostgreSQL](#build-library-with-postgresql)
* [Documentation](#documentation)
* [Binding to node.js](#binding-to-nodejs)
    * [Using the node module](#using-the-node-module)
    * [Generating a new node module for your system](#generating-a-new-node-module-for-your-system)
* [Test NodeJs](#test-nodejs)
* [Support](#support)
    * [Libcore:](#libcore)
    * [Bindings:](#bindings)
* [Developement guidelines](#developement-guidelines)
    * [CI](#ci)
* [Q/A and troubleshooting](#qa-and-troubleshooting)
    * [I have updated an include file and test code doesn’t see the changes!](#i-have-updated-an-include-file-and-test-code-doesnt-see-the-changes)
    * [I have upgraded my macOSX system and now I can’t compile anymore.](#i-have-upgraded-my-macosx-system-and-now-i-cant-compile-anymore)

The blockckhain Ledger Core library.

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
git submodule update --init
```

## Dependencies

### Build

This project is based on [CMake] as a build system so you should install it before starting (at least version 3.12).

### External dependencies:

- [Qt5](https://www.qt.io/download) is needed to build the tests of the library.
- Generation of interfaces and bindings is triggered with [Djinni](https://github.com/dropbox/djinni),
  a tool by [dropbox] that got discontinued — our fork is now one of the most advanced in terms of
  languages support.
- Make sure that you have [PostgreSQL] installed on your machine if you want to compile with it.
- Multi-platform is based on [polly] toolchains, a collection of [CMake] scripts implementing various
  toolchains.

## Documentation

You can generate the Doxygen documentation by running the `doc` target (for instance, `make doc`
with makefiles).

An online version is available [here](https://docs.libcore.ledger.com).

## Build of C++ library

### CMake configuration

It is up to you to decide where to put [CMake] compilation’s artifacts; however, we strongly advise
to build the library in a dedicated, isolated directory — such a directory is typically called a
_build tree_. That directory is, by convention, called `build` and is at the top-level of the
project directory.

If you respect this folder structure (and naming), after `cd lib-ledger-core/build`, you can configure
the library by running:

```
cmake .. -DCMAKE_BUILD_TYPE=Debug -DBUILD_TESTS=ON -DBUILD_COINS="bitcoin,ethereum" -DSYS_OPENSSL=ON -DOPENSSL_ROOT_DIR=~/github/openssl
```

> Note (macOS only*): you will have to provide the path to your Qt5 installation by defining the
> `CMAKE_PREFIX_PATH` variable. If you struggle to find where Qt5 is located, it can most of the
> time be found at:
>
> ```
> /usr/local/Cellar/qt/<qt_version>/bin
> ```
>
> To take that into account, simply add `-DCMAKE_PREFIX_PATH=/path/to/qt5` to the `cmake` command
> line.

Several [CMake] arguments might interest you there (non-exhaustive list, only the most useful
arguments are showed here):

- `CMAKE_BUILD_TYPE`: you should always set that to `Debug` when testing as you will get DWARF
  symbols and debugging instruments support. If you want to get a release build, set it to
  `Release`.
- `BUILD_TESTS`: set to `ON` to compile all test suites so that you can run unit and integration
  tests. You will typically disable it (set to `OFF`) if you want to integrate the library in a
  Ledger product on your local machine — you will not need tests for that and compilation will
  then be much faster.
- `CMAKE_EXPORT_COMPILE_COMMANDS`: set to `YES`, useful when you’re using a C++ linter, such as
  [cquery], as it will export the symbol database for you to play with in your editor for
  auto completion,etc.
- `BUILD_COINS`: a mandatory option defining a list of coins (separated by commas) to bundle in
  the library. The output library will contain code for all the coins listed in that option, so
  this is useful to tweak if a product doesn’t support all coins implemented by Core, for instance.
- `SYS_OPENSSL`: set to `ON`, it will make [CMake] use your system-wise OpenSSL installation. This
  is going to be a default behavior at some point.
- `OPENSSL_ROOT_DIR`: path to an OpenSSL project directory. This option is often mandatory on some
  systems that won’t ship OpenSSL [CMake] targets. If, while configuring with the `cmake` command,
  you have problem about OpenSSL modules, clone the official OpenSSL project on your machine,
  checkout the right tag (it will depend on what product you target, but typically, you will want a
  1.1.1 tag), configure and compile it, and then reference that path in `OPENSSL_ROOT_DIR`.
- `PG_SUPPORT`: set to `ON` to compile with [PostgreSQL].

### Interface generation

In order to be able to compile the library, you must generate the C++ interface files. Those files
are mostly common code and abstract code containing interfaces implemented by the rest of the
library. Those interfaces define a bridge between our implementations and the external (i.e.
products / clients) world. You can see this interface has the public contract we have with our
clients.

The entry point is `./bundle/bundle.djinni`. This file contains the public interface for the
library to _bundle_. A _bundle_ is simply a collection of coins code plus the common / base code of
the library, all bundled together.

> The fact we _bundle_ the code is possible because of how modularized the code is. You can pick
> which coins will go in the bundle with the `BUILD_COINS` CMake variable.

Generating the interface is sensible to which coins are going to be bundled, so you need to pass
the same list as you did within `BUILD_COINS`. You use the `./tools/lc api <list of coins>` command
to do so — coin names separated by spaces instead of commis.

For instance:

```
./tools/lc api bitcoin ethereum ripple tezos
```

> There is no “all” coins; you have to pass them all by hand, for now.

If you haven’t compiled `djinni` yet, this command will compile a local version of `djinni` and
will treat the IDL files to generate the C++ interfaces. It will also generate a bunch of other
interface files that you can all find in the `./bundle` directory.

### Compilation

Once your project is configured and the public interface generated, you’re one command away from
building the library:

```
cd build
make -j16 # if you have a 16 threads CPU
```

As simple as that.

### Building for JNI

Building with JNI (Java Native Interface), allows you to use the library with JVM based software,
such as Scala projects. Simply add the `-DTARGET_JNI=ON` to your `cmake` command line to enable JNI
support.

```
cmake .. -DTARGET_JNI=ON # rest of arguments
```

This will add JNI files to the library compilation and remove tests – we do not support testing with
JNI enabled. You need at least a JDK 8 to build for JNI (OpenJDK or Oracle JDK). Some operating
systems will require you to set environment variables for this to work, such as `JAVA_HOME`, etc.

### Build library on docker

You can build the core library or debug it from a docker image:

1. Build the image `docker build -t ledger-core-env .` (considering that you are currently at the root of the repository)
2. Run the image `docker run -ti --cap-add=SYS_PTRACE --security-opt seccomp=unconfined ledger-core-env`
3. Notice that stopping a container will wipe it. If you need multiple instance over the same container one way is to start the container as a daemon and then get a shell on it.
  1. Start the container as daemon `docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -d ledger-core-env`
  2. Get the container ID with `docker ps`
  3. Open shells `docker exec -ti :container_id zsh` where :container_id has to be replaced by the container you got from `docker ps`

> This way of building is not the recommended way for Core developers. You should stick with the
> regular install mechanism as we don’t really maintain the Dockerfile anymore.

### Build library with OpenSSL

It is possible to build the library with embedded version of OpenSSL (which is advised for
Android and iOS builds) or you can rely on OpenSSL provided by the system.

To build the library with system's OpenSSL you should configure the project by providing
`-DSYS_OPENSSL=ON` option plus some additional variables to allow `cmake` to find OpenSSL library:

We are currently supporting OpenSSL `1.0` and `1.1`.

### Build library with PostgreSQL

To compile with [PostgreSQL] support, you should add `-DPG_SUPPORT=ON` to your
[CMake] configuration command.

#### Configuration in the code

To use with libcore, simply set value of the key `api::PoolConfiguration::DATABASE_NAME`
to the database's URL connection and set it in the `Services`' configuration.

It is also possible to configure the size of the connection pool when instantiating the
[PostgreSQL] `DatabaseBackend`: `api::DatabaseBackend::getPostgreSQLBackend(int32_t connectionPoolSize)`.

#### Local testing

If testing locally, make sure to have a running PostgreSQL server.
As an example, if you are running it on `localhost:5432` and `test_db` as database name,
database's name forwarded to the pool (through configuration key `api::PoolConfiguration::DATABASE_NAME`)
should look like : `postgres://localhost:5432/test_db` .

> It is advised to use a `docker` container here to speed up local testing, so that you don’t have
> to install the whole server on your system.

## Binding to node.js

The library can be compiled and integrated as a node module in a pretty straightforward way. You
will be interested in either using it, or making a new version of the node module.

### Using the node module

The [lib-ledger-core-node-bindings] repository contains the node.js bindings you will need to
interface with `lib-ledger-core`. You can either clone the git repository or simply install from
`npm` directly:

```
npm i @ledgerhq/ledger-core
```

### Generating a new node module for your system

In order to generate bindings, you need to have:

- A Java / JRE (at least 7) runtime available to compile the local copy of `djinni`.
- [yarn].
- Optionally, [yalc].

Generating bindings is a several steps process:

1. First, you need to make some changes to `lib-ledger-core` and generate a fresh version of
   the library. Refer to the [building section](#build-of-c-library) for further information.
2. Then, you can use the `lc` script located in `./tools` to generate the binding. Simply give
   it the `npm` argument and it will generate the whole node package for you. This script
   expects by default the build tree to be `./build`, but you can customize this behavior with
   the `BUILD_DIR` environment variable:
   ```
   ./tools/lc npm # will look for the library in build
   BUILD_DIR=./build_npm_special ./tools/lc npm # will look for the library in build_npm_special
   ```
3. (Optional): if you have `yarn` and `yalc` installed, this command will also publish (and push!)
  the new package to your local store and dependent local projects.
4. If you need to access the package, it is located in `./bindings/node`.

## Test NodeJs

```
node ledger-core-samples/nodejs/tests/wallet-pool-test.js
```

## Platforms support

### Libcore

The library can be built for the following platforms:

- macOS: minimum supported version is `macOS 9.0`, with `x86_64` architecture.
- GNU/Linux: Debian (stretch), Ubuntu and Arch are supported, with `x86_64` architecture. In theory,
  any GNU/Linux distribution satisfying the prerequisites in the build section is supported.
- Windows: 64-bit architecture is built with `MSVC` (starting from Visual Studio 15), 32-bit is built with `MinGW`.
- iOS: `x86_64`, `armv7` and `arm64` architectures are supported; minimum supported version is `iOS 10.0`.
- Android: `x86`, `armeabi-v7a` and `arm64-v8a` architectures are supported; minimum supported version is `Android 7 (API 24)`
  (Java 8 is needed).

### Node

The node.js bindings can be generated for:

- `node` with version `>=8.4.0` and `<9.0.0` (other versions are not tested – yet),
- `node-gyp` is used to build native module and requires `python` with version `2.7.x`.

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
title**. You could be tempted to put it in the body of your message but that will not work with
[Appveyor].

Finally, it’s advised to put it on every commit and rebase at the end to remove the `[skip ci]` tag
from your commits’ messags to have the CI eventually run, but some runners might be smart enough to
do it for all commits in the PR.

Rebasing is done easily. If your PR wants to merge `feature/stuff -> develop`, you can do something
like this — assuming you have cloned the repository with a correctly set `origin` remote:

```
git switch feature/stuff
git rebase -i origin/develop
```

Change the `pick` to `r` or `reword` at the beginning of each lines **without changing the text of
the commits** — this has no effect. Save the file and quit. You will be prompted to change the
commits’ messages one by one, allowing you to remove the `[skip ci]` tag from all commits.

## Q/A and troubleshooting

### I have updated an include file and test code doesn’t see the changes!

Currently, interface files (headers, .hpp) **are not linked but copied directly into the test
directory**. That means that every time you make a change in the interface that is tested by any
code in core/test/, you need to update the copy.

Just run this command:

```
cd $your_build_folder
rm -rf CMakeFiles CMakeCache.txt
```

### I have upgraded my macOS system and now I can’t compile anymore.

Especially if you’ve upgraded to Mojave for which there are some breaking changes, you will need to
perform some manual tasks — here, for Mojave:

```
xcode-select --install
open /Library/Developer/CommandLineTools/Packages/macOS_SDK_headers_for_macOS_10.14.pkg
```

[CMake]: https://cmake.org
[dropbox]: https://www.dropbox.com
[polly]: https://github.com/ruslo/polly
[PostgreSQL]: https://www.postgresql.org
[yarn]: https://yarnpkg.com
[yalc]: https://github.com/whitecolor/yalc
[contribution guidelines]: ./CONTRIBUTING.md
[lib-ledger-core-node-bindings]: https://github.com/LedgerHQ/lib-ledger-core-node-bindings
[CircleCI]: https://circleci.com
[Appveyor]: https://www.appveyor.com
[cquery]: https://github.com/cquery-project/cquery
