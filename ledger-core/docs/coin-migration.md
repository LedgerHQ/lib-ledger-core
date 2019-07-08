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

## Preamble

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

### Configure and compile your library

Next step is quite simple. Go into the `build/` directory of your library and invoke the CMake
command to generate the configuration for your platform:

```
cd ledger-core-abrac/build
cmake -DCMAKE_INSTALL_PREFIX=/usr/local/Cellar/qt/5.11.2_1 -DCMAKE_BUILD_TYPE=Debug -DCMAKE_EXPORT_COMPILE_COMMANDS=YES ..
```

> Please do not copy-paste without ensuring the `CMAKE_INSTALL_PREFIX` variable is correctly set.

Once this is done, you can compile your library in a straight-forward way:

```
make -j8
```

And here you are!

## Migrate the database system

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

  - Edit the `coinID` static variable in the `database/src/migrations.cpp` file to put the correct
    value, depending on your coin. Please refer to <https://github.com/satoshilabs/slips/blob/master/slip-0044.md#registered-coin-types>
    if you don’t know which value to use.
  - Migrations use C++’s template system. You have to edit two functions to fully implement a
    migration:
    - The `migrate<N, T>` function. `N` is the number of the migration and `T` is the *tag type*
      representing your coin (it’s defined in `database/inc/migrations.hpp`).
      You can change the name of this type if you want, but don’t forget to change it in the `.cpp`
      version too.
    - The `rollback<N, T>` function.
  - When you need a new migration, all you have to do is to create a new pair of functions by
    incrementing the `N` value and change the `currentVersion` static variable to match the latest,
    expected version.

[soci]: https://github.com/SOCI/soci
