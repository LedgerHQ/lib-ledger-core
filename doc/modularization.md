# Coin integration and/or migration

This document gives a detailed description of a *coin integration* and/or migration
from `lib-ledger-core` to `ledger-core`. For the record, `lib-ledger-core` is the first, legacy
codebase in which all coins are gathered. A typical release includes a single dynamic library
(`.so`, `.dll`, `.dylib`, depending on the target platform) and some generated binding files.
`ledger-core` is the new, abstract and coin-agnostic library. On its own, it’s not really usable by
the application team as it just gives an interface to other coins.

> This is not strictly true as it also includes a lot of shared and common code, but you wouldn’t do
> much with them without at least one coin type.

For the purpose of this document, we will consider the migration of a coin called `abrac (ABC)`

<!-- vim-markdown-toc GFM -->

* [Overall architecture](#overall-architecture)
* [Migration foreword](#migration-foreword)
* [Setting up a new project](#setting-up-a-new-project)
    * [Create a new Ledger Core Coin project](#create-a-new-ledger-core-coin-project)
    * [Generate the public API](#generate-the-public-api)
    * [Configure and compile your libraries](#configure-and-compile-your-libraries)
        * [CMake packages](#cmake-packages)
        * [Configuring, building and installing ledger-core](#configuring-building-and-installing-ledger-core)
        * [Configuring and building your coin library](#configuring-and-building-your-coin-library)
* [Migrating the database system](#migrating-the-database-system)
    * [Operation queries migration](#operation-queries-migration)
* [Tests](#tests)
* [Glossary](#glossary)
    * [Services](#services)
    * [WalletStore](#walletstore)

<!-- vim-markdown-toc -->

## Overall architecture

The new architecture is rather simple.

  - **Ledger Core** is the name of the team.
  - `lib-ledger-core` is both the name of our git repository but also the legacy library, containing
    all coins gathered in the same binary.
  - `ledger-core` is the new abstract Core library. It is completely coin-agnostic and lots of
    features from `lib-ledger-core` has been either removed or moved somewhere else.
  - `ledger-core-<coin-name>` is a library that provides Core support for `<coin-name>`. For
    instance, `ledger-core-ripple` provides the Ripple blockchain via our Core library.
  - `ledger-core-factories` is a special library that lists all currently supported coins. It is
    not necessarily mandatory in order to use the Core library but highly recommended to get started
    with minimal amount of effort as it contains all objects to automatically synchronize a given
    account without you to care about the dependent objects.

## Migration foreword

Before doing anything else, you need to know a script exists to automate **a lot** of things for
you. We strongly advise you to add the `tools/` path to your `PATH` variable:

```
export PATH=$PATH:<path/to/lib-ledger-core/tools>
```

> However, currently, the script only works correctly if you’re located at the top-level of the git
> repository.

## Setting up a new project

### Create a new Ledger Core Coin project

The new Ledger Core architecture implies to have one project per coin. That helps to add ad hoc code
without having to change the interface and, thus, allowing to incrementally and smoothly support
coin integration.

In order to do so, you need to create a new Ledger Core project. You do that with the `lc` command:

```
lc project new <coin-name> [--force]
```

The `<coin-name>` identifier can be anything you would like for your project. We suggest to use
lowercase identifiers describing your currency. In our case, we’ll use `abrac`.

```
lc project new abrac
```

This will create a project in your current working directory. For easier interaction with
`ledger-core`, we strongly advise to put the project at the top-level of the `lib-ledger-core` git
repository. That will ease future operations.

If you want to force regeneration of the project, such as CMake files or such, you can pass the
`--force` argument. That will not erase your source code but it will override all files that were
there in the first place (such as CMake files, the main IDL file, etc.). Be careful when using
this command — or count on git!

### Generate the public API

The public API is really easy to generate with the new Ledger Core: you must generate the public
API of the Core library first, and then you can generate the API of your coin library. This is done,
respectively, as such:

```
lc api # generate the Ledger Core API
lc project api <coin-name> # generate the coin library’s API
```

You will notice a new directory in your library’s `inc/` directory: `inc/api/`.

### Configure and compile your libraries

The initial step is to compile the `ledger-core` library, as it’s a dependency of everything else.
In order to do so, you need to understand how to package libraries.

#### CMake packages

We use _CMake_ to build our libraries. Although _CMake_ is not a package manager, we can still use
it to _package_ libraries and pass configuration around for dependent projects to find and link
against us. The way it’s done is multi-step process:

1. You _configure_ your library. This is a mandatory step that implies generating the right
  constants / setup for your environment (or for cross-compilation purposes). It’s typically the
  the time when you call the `cmake` command.
2. You _compile_ your library. Compilation artifacts go into a directory of your choice — chosen
  at step (1.). It is important to notice that such a folder is a called a _compilation tree_ and
  **only** contains compilation artifacts. Those artifacts and paths shouldn’t be considered as
  _deployed_. They’re just output of the raw compilation. It’s typically when you call
  `make` or whatever compilation toolchain you’re using.
3. You _install_ your library. _CMake_ doesn’t do that by default but can generate scripts to
  automate installing for you. Typical commands here are `make install`. Install directories
  are set at step (1.) **and must be different from the compilation tree.**

Because we need two directories (one for the compilation artifacts, one for
deployment / installing), we can create various and interesting situations.

- The default `ledger-core/build` directory should be used as compilation tree. In order to
  _select_ that directory as compilation tree, all you have to do is to `cd ledger-core/build`
  and invoke your `cmake` command from here.
- The installation directory is completely free to pick and there’s no default.
  - If you don’t specify any, _CMake_ will use its default system directory, which defaults to
    `/usr/local` on Unix-like systems. On some platforms (like Archlinux), that might require you
    super-user rights, which is not ideal.
  - If you specify one, you should consider that directory as a _sandbox_. For instance,
    `/tmp/ledger-core-install`.
- Whatever the chosen install directory, you’ll have to stick with it. If you don’t use any,
  don’t introduce one in the dependent project or `ledger-core` might not be found. If you use one,
  stick with it.
- The install directory is set when invoking `cmake` with the `CMAKE_INSTALL_PREFIX` variable.
- Because we’re using _Qt5_ as dependency, we **also** need to append special _Qt5_ paths to the
  _CMake_ resolution paths when invoking `cmake`. This is done via the `CMAKE_PREFIX_PATH`
  variable. It is not always mandatory (you might need it on macOS).

#### Configuring, building and installing ledger-core

That’s already a lot to take into your mind. Let’s configure `ledger-core` so that we install it
in `/tmp/ledger-core-install` — on macOS:

```
cd ledger-core/build
cmake \
  -DCMAKE_PREFIX_PATH=/usr/local/Cellar/qt/5.13.2 \
  -DCMAKE_INSTALL_PREFIX=/tmp/ledger-core-install \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  ..
```

`-DCMAKE_BUILD_TYPE=Debug` should always been used on your machine when working or debugging as it
will provide DWARF symbols and more information. It’s especially useful when injecting the library
into a Ledger product such as Live Desktop / Mobile or Vault.

`-DCMAKE_EXPORT_COMPILE_COMMANDS=ON` generates a _ctags-like_ file to help you with completions in
your editor (if it doesn’t support LSP).

Next step is simple: just compile your library:

```
make -j8
```

Upon completion, `ledger-core/build` contains a lot of artifacts. You can execute the tests with
`make test` (or `ctest`), or install the artifacts into `/tmp/ledger-core-install`.

Installation is easy as well:

```
make install
```

Once you’ve installed the library, you can start hacking on your coin library!

#### Configuring and building your coin library

Now that you have:

1. Generated the public interface for `ledger-core` and `ledger-core-abrac`.
2. Configured, compiled and installed `ledger-core` into `/tmp/ledger-core-install`.

You are read to get moving. Go into `ledger-core-abrac/build` and repeat the configuration process.
Don’t forgot to be consistent with the `CMAKE_INSTALL_PREFIX`.

```
cd ledger-core-abrac/build
cmake \
  -DCMAKE_PREFIX_PATH=/usr/local/Cellar/qt/5.13.2 \
  -DCMAKE_INSTALL_PREFIX=/tmp/ledger-core-install \
  -DCMAKE_BUILD_TYPE=Debug \
  -DCMAKE_EXPORT_COMPILE_COMMANDS=ON \
  ..
```

Same thing, build:

```
make -j8
```

And here you are!

## Migrating the database system

One of the most important aspect in a coin integration is the persistence. Persistent objects are
usually stored in a database. Currently, you can choose the way you want to store your objects but
we strongly advise you to use the provided mechanism.

The `ledger-core` library provides a simple way to store objects in a database. For that matter,
the [soci] library is used to create SQL statements. Those statements are gathered in *migrations*.
Currently, migrations are composed of two parts:

- **Forward migrations**: this kind of migration migrates the content of the coin’s database from
  version `N` to version `N + 1`. They typically add new tables, columns, data, but they can also
  remove data depending on what needs to be done.
- **Backward migrations**: this kind of migration is often called a *rollback migration*, as it
  cancels the associated *forward migration*. The SQL statements in such migrations will reflect
  the opposite actions done in the *forward migration*. For instance, if you add a table in the
  *forward migration*, the *rollback migration* will remove it. Please try to provide useful
  *rollback migrations* as much often as possible, even though sometimes *rollback migrations* are
  not a lossless operation.

The `ledger-core` library also has some migrations to handle the abstract and common objects. You
should follow the same conventions to handle your migrations, that are:

- Edit the `COIN_ID` static variable in the `database/Migrations.hpp` file to put the correct
  value, depending on your coin. Please refer to <https://github.com/satoshilabs/slips/blob/master/slip-0044.md#registered-coin-types>
  if you don’t know which value to use.
- Edit the `CURRENT_VERSION` static variable in the same file. If you’re just creating your
  project for the first time, you’ll likely to just put `1` here, but you should update that value
  as soon as you have a new migration pair to add.
- Migrations use C++’s template system. You have to edit two functions to fully implement a
  migration:
  - The `migrate<N, T>` function. `N` is the number of the migration and `T` is the *tag type*
    representing your coin (it’s defined in `database/Migrations.hpp`).
    You can change the name of this type if you want, but don’t forget to change it in the `.cpp`
    version too.
  - The `rollback<N, T>` function.
- When you need a new migration, all you have to do is to create a new pair of functions by
  incrementing the `N` value and change the `CURRENT_VERSION` static variable to match the latest,
  expected version.

You’re done with migrations, but you still have something to do: tell `ledger-core` to setup the
database structure when using your project. There’s not a single way to do it, but there’s a
preferred fashion: ask for the creation of the database structure when creating a new wallet
factory. In the constructor of your factory, you should be passed a `Services` object, which you can
use to create your database structure.

```cpp
RippleLikeWalletFactory::RippleLikeWalletFactory(
    const api::Currency &currency,
    const std::shared_ptr<Services>& services
):
  AbstractWalletFactory(currency, services) {
    _keychainFactories = {{api::KeychainEngines::BIP49_P2SH, std::make_shared<RippleLikeKeychainFactory>()}};
    // create the DB structure if not already created
    services->getDatabaseSessionPool()->forwardMigration<XRPMigration>();
}
}
```

The `DatabaseSessionPool::forwardMigration<T>` method allows you to perform, if not already done,
the database setup for a given coin identified by `T`.

### Operation queries migration

The `OperationQuery` type is now a pure abstract class because its children must implement the
`OperationQuery::inflateCompleteTransaction`. The content of that function can be taken out the
legacy `OperationQuery::inflateCompleteTransaction`’s `switch` case, depending on the coin you’re
implementing. You will have to read some more code (for instance, for Bitcoin, the
`OperationQuery::inflateBitcoinLikeTransaction`) in order to implement the pure virtual method in
the coin specific code.

If you’re not migrating from an existing coin, you shouldn’t find the whole thing hard to deal
with.

## Tests

Testing is supported via the `ledger-core` library which exposes some features. In your test
projects, you can link to:

- `ledger-test`: basic framework which gives you transaction test helpers, base fixtures, etc.
- `ledger-qt-host`: _Qt5_-based implementation of HTTP clients, thread dispatcher, execution
  contexts, etc.
- `ledger-core-integration-test`: some more integration primitives to help you write integration
  tests.
- `gtest` and `gtest_main`: mandatory GoogleTest and GoogleMock libraries.

## Glossary

### Services

The *wallet pool* has been a very coupled type. It has bindings and links to several other types
and objects:

  - The *pool name*.
  - The *password* used to encrypt the whole wallet pool and subsequent wallets.
  - A *configuration* object, which is some kind of dynamic local storage that exists only when the
    wallet pool lives.
  - A *path resolver*, used to resolve paths used internally.
  - An *http engine*, used to provide HTTP connection to clients.
  - A set of connected HTTP clients.
  - A WebSocket, connected client.
  - An internal and external *preference* system, allowing to store data in a local storage used to
    customize the wallet application.
  - A *database* usable by coin backends to store various data.
  - A *logger*.
  - A *thread dispatcher*.
  - A *random number generator*.
  - List of *currencies* currently available in the wallet application.
  - An *event publisher*.
  - Some other data regarding block and event synchronizations.
  - List of *wallets*. Those are abstract wallets and you shouldn’t try to use them directly.

Factories used to be in the wallet pool but they were removed to make the abstract code completely
coin-agnostic. Currencies are abstract objects used to represent each coin metadata. It contains:

  - Name of the coin.
  - Unique ID (integer) representing the coin. (SLIP-0044).
    See [this](https://github.com/satoshilabs/slips/blob/master/slip-0044.md) for further details.
  - Payment URI scheme.
  - List of currency units, which each contains:
    - Name of the currency unit.
    - Code name of the currency (short name, typically on three letters).
    - Number of decimals.

Currencies used to hold network parameters but they don’t anymore. The idea is that they must
remain coin-agnostic.

The concept of _wallet pool_ and _wallets_ was temporary removed. A new type was introduced to
hold all common and sharable properties: `Services`. The `Services` type is a simple type that can
formulated as:

```
Services = WalletPool - wallets - factories - currencies
```

So it’s basically the same thing as an old `WalletPool` but removed from the wallets, factories and
currencies objects.

### WalletStore

The `WalletStore` is the remaining part of modularized `WalletPool`.
`WalletStore + Services = old WalletPool`. A `WalletStore` contains everything related to
wallets and is, indeed, a map of name to wallets.

[soci]: https://github.com/SOCI/soci
