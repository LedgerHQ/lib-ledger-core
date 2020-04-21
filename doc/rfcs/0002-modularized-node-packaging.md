- RFC name: `0002-modularized-node-packaging`.
- Status: `implemented`.
- Author: [Dimitri Sabadie](https://github.com/phaazon)
- Date Created: 2020/02/12
- Date Updated: 2020/02/12
- Summary: **N:1 NPM packaging of the modularized core (N coins, 1 NPM package).**

<!-- vim-markdown-toc GFM -->

* [Motivation](#motivation)
* [Content](#content)
  * [How packaging is done on the legacy path](#how-packaging-is-done-on-the-legacy-path)
  * [How packaging is done on the modularized path](#how-packaging-is-done-on-the-modularized-path)
  * [How to use the generated package](#how-to-use-the-generated-package)
    * [Publishing](#publishing)
    * [Adding the package as a dependency](#adding-the-package-as-a-dependency)
  * [Implementation details](#implementation-details)
* [Rationale](#rationale)
* [Related work](#related-work)

<!-- vim-markdown-toc -->

# Motivation
> Why such a PoC?

The modularization of the Core library took several months but eventually covered all already supported
coins. Unit tests and integration tests were migrated so that we can test the modularization and all has been
green.

Even though that assurance is pretty strong, we still don’t really know _how_ the new Core is going to behave
in end-user products, such as the [Ledger Live]. The goal of this RFC and related implementation is to be able
to test our new library inside a `node` environment and benefit from all the unit, integration and QA tests of
[Ledger Live]. In order to do this, we need to support NPM packaging in a new and fancy way, since we don’t
know how many coins are going to be packaged anymore.

This document describes the N:1 NPM packaging.

# Content
> Content of the PoC with comments and step-by-step procedure.

## How packaging is done on the legacy path

On the legacy, the NPM packaging mechanism is not done in the [lib-ledger-core] repository. It is done in the
[lib-ledger-core-node-bindings] repository. The way it works depends on some ordered steps:

1. First, we need to compile a version of the [lib-ledger-core]. This is a manual and pre-packaging
  operation. This is not automated, for the reason that we want to be able to package any exotic
  configuration of the library. For instance, as Core developers, we often build a _debug_ version of the
  Core, while [Ledger live] uses a _release_ version.
2. Then, we have a script, called `generateBindings.sh`, that must be called. It takes as first argument the
  path to the bindings project and as second, the path to the build directory of the Core library (done in
  step 1.). That script will call [djinni] to generate all the required code for a NPM package generation.
3. We typically then `cd` into the bindings project, and we can call `yarn` to generate the package.

This works in a pretty straightforward way as the legacy Core has a big monolithic IDL tree. Everything is
referenced from within that tree. The generator used in [djinni] reads that tree to generate C++ source files
that will get compiled with the `Nan` library for `none`.

## How packaging is done on the modularized path

With the modularized Core, things are a bit different. First, we cannot assume _any_ coin will be used. This
is a requirement for making a good coin-agnostic packaging mechanism — and then, being able to scale well. We
still need to compile projects by hand for the same reason as (1.) from the previous section.

However, we do not use `generateBindings.sh` anymore, as we have the `lc` script. That script was augmented
with a new command: `pkg`. `lc pkg` is the entry point to any packaging of the Core. It expects a first
argument, which is the target package system. Currently, we only support `npm` as argument.

The base command is then:

```
lc pkg npm
```

The rest of the arguments is a list of projects to bundle into the NPM package. Because all coins depend on
the `ledger-core` project, you will want to add it to the list as well — typically added as first argument with
the special `core` name. Projects (`ledger-core` included) are added with their _names_, not folder names
(i.e. use `abc` instead of `ledger-core-abc`), the same way you did when creating them with `lc new`. They are
laid on the line and separated with commas.

Example:

```
lc pkg npm core bitcoin ethereum ripple tezos
```

> As always, the script must be run from the root of the `lib-ledger-core` project.

Once this is done, you will find the NPM package ready to be built in `bindings/node`, where you can simply
run `yarn` to build it. It’s currently named `@ledgerhq/ledger-core-modularized` to prevent clashing with the
already existing node package, but the name is subject to change in the future after stabilization.

## How to use the generated package

### Publishing

Once the package is generated, one can use any mean to add / link it to a Ledger Live project — or really,
any project. It is highly recommended to use the `yalc` utility to register on your system the package:

```
yalc publish
```

Or, if you have already configured other projects with it, you can directly push the changes to them
after having updated the local store:

```
# these two commands do exactly the same thing, so prefer the latter!
yalc publish --push
yalc push
```

### Adding the package as a dependency

Here, again, it’s pretty simple: you use `yalc`. Inside your project, like `ledger-live-common/cli`, simply
add the package as a dependency:

```
yalc add @ledgerhq/ledger-core-modularized
```

Don’t forget to `yarn` to rebuild your project. Everytime a change is made on the package, you need to
re-publish it and either re-run the `yalc add` command, or use `yalc push` to automate that process.

## Implementation details

`lc pkg` calls a script called `idl_pkg.sh` which gathers the logic of IDL-based packaging. Because of
limitation in [djinni], some work must be done to achieve packaging with a modularized library. On the
`npm` path, this script will first clean the `binding/node` folder. Although the legacy Core has a
(committed) `binding.gyp` file, we cannot use that file directly. [GYP] has a configuration files that
doesn’t easily allow to depend on multiple libraries — remember we have one for `ledger-core` and one for
each coin.

For this reason, a Python script (`tools/gyp_generator.py`) was created. That script takes the list of
projects (the same passed to `lc pkg npm`) and generates a `bindings/node/binding.gyp` for the selected
configuration of coins. The script is called at the end of packaging, after an important phase is done:
the generation of C++ source files.

C++ source files are used to make a bridge between `node` (by using the `Nan` library) and our Core library.
Normally, we should write that code, but [djinni] takes care of that for us. Calling [djinni] with some
arguments will generate C++ source files to be part of the [GYP] compilation that will generate a `node`
package. The problem is that [djinni] has been designed to work with a single C++ library at a time, not
several — we have several; one for each coin. The result of this is that, having [djinni] generate a C++
source file for each project (in the form `ledgercore-$coin.cpp`) will duplicate the initialization
process of the `Nan` library for each coin, resulting in a pretty bad situation. This is due to the fact
that [djinni] assumes a strong correlation between a .cpp file and a node project.

To fix that problem, after generation of the C++ source files, the `idl_pkg.sh` script will extract all the
important information from all the `ledgercore{,-$coin}.cpp` files, like `#include` directives and
specific objects’ initialization, and concatenate them inside a single file, called `final-output.cpp`. All
the `ledgercore{,-$coin}.cpp` files will then be deleted to prevent [GYP] from compiling them.

Once this is done, calling `yarn` will:

- Copy all the libraries (1 for `ledger-core` and N for the N coins you asked for) into the `node` package.
- Move the header files.
- Move the C++ files.
- Compile each C++ files.
- Compile the entry point (i.e. `node` _module_) by using the `final-output.cpp` file.
- Done.

# Rationale
> Should we go for it? Drop it?

The current implementation is limited by the features of [djinni]. Especially, we had to drop the YAML
archive — previously initiated to make projects isolation easier — because the NPM generator of [djinni]
needs to initialize arguments of each function calls by accessing records’ fields and fields are not stored
in YAML archives. See the `MDef` vs. `MExtern` types in the Scala sources for further information.

There is the question of whether we should move the bindings in a separate project or not. Since they can
entirely be generated, having to maintain a repository for them doesn’t seem like a good option — and it’s in
the scope of the [lib-ledger-core] library, as we officially support such bindings. The only part we still have
to maintain is the `package.json`.

[ReactNative] is not in the scope of this design document nor implementation.

# Related work
> What else has been done and is similar?

N/A

[Ledger Live]: https://github.com/LedgerHQ/ledger-live
[lib-llib-ledger-core]: https://github.com/LedgerHQ/ib-llib-ledger-core
[lib-ledger-core-node-bindings]: https://github.com/LedgerHQ/lib-ledger-core-node-bindings
[djinni]: https://github.com/LedgerHQ/djinni
[GYP]: https://gyp.gsrc.io
[ReactNative]: https://reactnative.dev
