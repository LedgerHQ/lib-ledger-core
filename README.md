# Ledger Core Library

* [Clone project](#clone-project)
* [Dependencies](#dependencies)
    * [Build](#build)
    * [External dependencies:](#external-dependencies)
* [Build of C++ library](#build-of-c-library)
    * [Building for JNI](#building-for-jni)
    * [Build library on docker](#build-library-on-docker)
* [Documentation](#documentation)
* [Binding to node.js](#binding-to-nodejs)
    * [Using the node module](#using-the-node-module)
    * [Generating a new node module for your system](#generating-a-new-node-module-for-your-system)
* [Test NodeJs](#test-nodejs)
* [Developement guidelines](#developement-guidelines)
    * [CI](#ci)

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

* [Qt5](https://www.qt.io/download) is needed to build tests of the library.
* Generation of binding is automated with [Djinni](https://github.com/dropbox/djinni).
* Build on multiple Operating Systems is based on [polly](https://github.com/ruslo/polly) toolchains.

## Build of C++ library

**_cmake_** is building out of source, you should create a build directory (e.g. `lib-ledger-core-build`):

	.                           # Directory where clone command was launched
    ├── lib-ledger-core         # Source files directory
    ├── lib-ledger-core-build   # Build directory

If you respect this folder structure (and naming), after `cd lib-ledger-core-build`, you can build the library by running:

```
cmake -DCMAKE_INSTALL_PREFIX=/path/to/qt5 ../lib-ledger-core && make
```

> (*macOSX users*) If you struggle to find where Qt5 is located, for example, on `macOSX`, `qt5` can
> be found at:

```
/usr/local/Cellar/qt/<qt_version>/bin
```

### Building for JNI

Building with JNI (Java Native Interface), allows you to use the library with Java based software. In order to enable JNI mode use

```
cmake -DTARGET_JNI=ON
```

This will add JNI files to the library compilation and remove tests. You need at least a JDK 7 to build for JNI (OpenJDK or Oracle JDK)

### Build library on docker

You can build the core library or debug it from a docker image:

1. Build the image `docker build -t ledger-core-env .` (considering that you are currently at the root of the repository)
2. Run the image `docker run -ti --cap-add=SYS_PTRACE --security-opt seccomp=unconfined ledger-core-env`
3. Notice that stopping a container will wipe it. If you need multiple instance over the same container one way is to start the container as a daemon and then get a shell on it.
    1. Start the container as daemon `docker run --cap-add=SYS_PTRACE --security-opt seccomp=unconfined -d ledger-core-env`
    2. Get the container ID with `docker ps`
    3. Open shells `docker exec -ti :container_id zsh` where :container_id has to be replaced by the container you got from `docker ps`

Note: If you feel on fire you could use docker volumes to persist data.

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

## Test NodeJs

```
node ledger-core-samples/nodejs/tests/wallet-pool-test.js
```
## Support

###Libcore:
Libcore can be built for following OSes:
 - MacOS: minimum supported version is `macOS 9.0`, with `x86_64` architecture,
 - Linux: Debian (stretch), Ubuntu and Arch are supported, with `x86_64` architecture,
 - Windows: 64-bit architecture is built with `MSVC` (starting from Visual Studio 15), 32-bit is built with `MinGW`,
 - iOS: `x86_64`, `armv7` and `arm64` architectures are supported, minimum supported version is `iOS 10.0`,
 - Android: `x86`, `armeabi-v7a` and `arm64-v8a` architectures are supported, minimum supported version is `Android 7 (API 24)` (Java 8 is needed).
###Bindings:
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

[contribution guidelines]: ./CONTRIBUTING.md
[lib-ledger-core-node-bindings]: https://github.com/LedgerHQ/lib-ledger-core-node-bindings
[CircleCI]: https://circleci.com
[Appveyor]: https://www.appveyor.com
