# Ledger Core: architecture draft

This document gathers all the architecture decisions made to the codebase, starting from the 6th
of August 2019. The codebase had existed for several years back then but no design documentation
existed beforehand.


<!-- vim-markdown-toc GFM -->

* [Foreword and overall architecture](#foreword-and-overall-architecture)

<!-- vim-markdown-toc -->

## Foreword and overall architecture

The [Core team] is responsible for the _core_ library, a library developed for [Ledger] and pushed
to all product and services teams to work with. Because we think Free and Open Source software
matters, the library can be used by anyone (refer to
[licensing](https://github.com/LedgerHQ/lib-ledger-core/blob/master/LICENSE) if you plan on using
it).

> For better understanding, we’ll call `$ROOT` the root of the Git repository.

The current state (_2019/08/06_) of the codebase is a bit complex. There is the existing, _in-prod_
library that we call `lib-ledger-core`. That library has a convoluted architecture:

  - The `$ROOT/core` directory contains all the files required to compile the C++ library and
    generate the foreign interfaces.
    - `$ROOT/core/idl` contains the IDL (Intermediatee Definition Language) files that defines the
      public foreign interface. What it means is that all the symbols (data types, constants,
      functions, etc.) defined in those files are the ones that will be publicly exported and
      available when using the library. If you use the _React Native_ version of the library, you
      will get a public Javascript file that will export the symbols you can find in the IDLs.
    - `$ROOT/core/lib` vendors most of our dependencies. Currently, the libraries needed at runtime
      are located in that directory. The rest of our dependencies (for instance, the CMake
      toolchains or [djinni]) are located in specific locations. Consider reading the [.gitmodules]
      files for further information.
    - `$ROOT/core/src` contains all the C++ code required to implement the library and its bindings.
      In that folder, there are two important sub-directories you must undertand the usecase:
      - `$ROOT/core/src/api` is the place where all the public API for C++ goes to. What it means is
        that in order to provide foreign symbols, the C++ library must implement a _logical
        interface_. That interface lies in that directory. Notice that such a directory is generated
        and you shouldn’t try to add or edit files in it.
      - `$ROOT/core/src/jni` is the place where the generated code to bind to Java / Scala is
        located.
    - `$ROOT/core/test` is where we put all our unit and integration tests. Nothing more, nothing
      less.
  - The `$ROOT/api` directory contains foreign interfaces and is deprecated.
  - The `$ROOT/build` directory might not be present, depending on how you configured your build,
    but if it is, it contains all the generated binary and intermediary representation output of the
    compilation stage. You will find test binaries as well as the Core library.
  - The `$ROOT/doc` directory contains useful Markdown documentation you really should read while
    being onboarded in the team as well as some files used to generate documentation.
  - The `$ROOT/playground` directory contains a small playground you play with to try using the Core
    library with Ledger Live Desktop. It’s very likely to be deprecated.
  - The `$ROOT/toolchains` folder contains CMake toolchains required to cross-compile the library.
  - `$ROOT/tools` contains several utils and tools we use to generate files, create new sub-projects
    or do maintainance on the codebase.
    - `$ROOT/generate_interfaces.sh` must be used when IDL files are altered, deleted or added, in
      order to update the C++ interface files.
    - `$ROOT/generateBindings{,RN}.sh` should be used when IDL files are altered so that foreign
      projects (_Javascript_, _React Native_, _Scala_, etc.) are modified as well.
    - `$ROOT/idl_interfaces.sh` is the new method to generate interfaces with the incoming revision
      of the Core library. You can ignore this script for now.
    - `$ROOT/lc` is an executable to do maintainance on the new Core library. Ignore that.

The typical workflow is the following:

  1. If you have introduced new public symbols via IDL files:
    1. Add, remove and edit all the IDL files you need in `$ROOT/core/idl`.
    2. Regenerate the C++ interface.
      - `./tools/generate_interfaces.sh`
  2. Create a `build` directory that will containt the output files of the compilation stage.
    - `mkdir $ROOT/build`
    - `cd $ROOT/build`
  3. Configure your setup based on your platform. For instance, here, on _macOSX_:
    - `cmake -DCMAKE_INSTALL_PREFIX=/path/to/qt5 -CMAKE_BUILD_TYPE=Debug ..`
  4. Compile the library.
    - `make -j8`
  5. If you want to test locally with a product, like Ledger Live Desktop or Mobile, you need to
    have cloned their repository beforehand and place them at the same level as `lib-ledger-core`.
    You also need to have cloned the appropriate binding projects.
      - For Ledger Live Desktop, you want [lib-ledger-core-node-bindings].
      - For Ledger Live Mobile, you want [lib-ledger-core-react-native-bindings].
      - For Vault, no need to generate anything but you have to recompile the library with JNI
        support:
          - `cmake -TARGET_JNI=ON […]`
  6. Generate the foreign interfaces with `$ROOT/tools/generateBindings.sh` or
    `$ROOT/tools/generateBindingsRN.sh`, depending on your target.

Refer to the appropriate projects for further information on how to build and use them.

[Core team]: https://github.com/orgs/LedgerHQ/teams/core-library
[Ledger]: https://www.ledger.com
[djinni]: https://github.com/LedgerHQ/djinni
[.gitmodules]: .gitmodules
[lib-ledger-core-node-bindings]: https://github.com/LedgerHQ/lib-ledger-core-node-bindings
[lib-ledger-core-react-native-bindings]: https://github.com/LedgerHQ/lib-ledger-core-react-native-bindings
