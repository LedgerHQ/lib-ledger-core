/*
 *
 * ledger
 * ledger-core
 *
 * Created by Pierre Pollastri on 14/09/2016.
 *
 * The MIT License (MIT)
 *
 * Copyright (c) 2016 Ledger
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 */

#ifndef LEDGER_CORE_LEDGER_CORE_H
#define LEDGER_CORE_LEDGER_CORE_H

/*! \mainpage \image Official Ledger Core Library documentation
 *
 * ![](/logo.svg)
 *
 * Core library which will be used by Ledger applications.
 * 
 * ## Clone project
 * 
 * ```
 * git clone --recurse-submodules https://github.com/LedgerHQ/lib-ledger-core.git
 * ```
 * 
 * ## Dependencies
 * 
 * ### Build
 * This project is based on **cmake** as a build system so you should install it before starting (at least version 3.7).
 * 
 * ### External dependencies:
 * * [Qt5](https://www.qt.io/download) is needed to build tests of the library.
 * * Generation of binding is automated with [Djinni](https://github.com/dropbox/djinni).
 * * Build on multiple Operating Systems is based on [polly](https://github.com/ruslo/polly) toolchains.
 * 
 * ## Build of C++ library
 * 
 * **cmake** is building out of source, you should create a build directory (e.g. `lib-ledger-core-build`):
 * 
 * 	.                           # Directory where clone command was launched
 *     ├── lib-ledger-core         # Source files directory
 *     ├── lib-ledger-core-build   # Build directory
 * 
 * If you respect this folder structure (and naming), after `cd lib-ledger-core-build`, you can build the library by running:
 * 
 * ```
 * cmake -DCMAKE_INSTALL_PREFIX=/path/to/qt5 ../lib-ledger-core && make
 * ```
 * 
 * ### Building for JNI
 * 
 * Building with JNI (Java Native Interface), allows you to use the library with Java based software. In order to enable JNI mode use
 * ```
 * cmake -DTARGET_JNI=ON
 * ``` 
 * 
 * This will add JNI files to the library compilation and remove tests. You need at least a JDK 7 to build for JNI (OpenJDK or Oracle JDK)
 * 
 * ## Binding to node JS
 * 
 * Generate binding (under `build/Release/ledgerapp_nodejs.node`):
 * 
 * ```
 * npm i
 * ```
 * 
 * ## Test NodeJs
 * 
 * ```
 * node ledger-core-samples/nodejs/tests/wallet-pool-test.js
 * ```
 */

#ifndef LIBCORE_EXPORT
    #if defined(_MSC_VER) && _MSC_VER <= 1900
        #include <libcore_export.h>
    #else
        #define LIBCORE_EXPORT
    #endif
#endif

#include <string>

/**
 * Ledger root namespace
 */
namespace ledger {

    /**
     * Ledger core root namespace
     */
    namespace core {

        extern LIBCORE_EXPORT const int LIB_VERSION;
        extern LIBCORE_EXPORT const int VERSION_MAJOR;
        extern LIBCORE_EXPORT const int VERSION_MINOR;
        extern LIBCORE_EXPORT const int VERSION_PATCH;
        extern LIBCORE_EXPORT const std::string LIB_STRING_VERSION;

    }

}

#endif //LEDGER_CORE_LEDGER_CORE_H
