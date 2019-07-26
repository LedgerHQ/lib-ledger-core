# Pre migration notes

This document holds several notes about the previous design prior to migrating to the new,
modularized Core library.

  - [ ] Several objects (wallet pools, wallets, accounts, etc.) own shared pointers to common
    objects they need for some operations. That is a very OOP way of doing and we need to get rid of
    that.
  - [ ] The `halfBatchSize` in `syncBatches` method was designed and imagined for Bitcoin, but that
    feature lives in abstract code, which doesn’t make any sense whatsoever anymore.
