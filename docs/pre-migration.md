# Pre migration notes

This document holds several notes about the previous design prior to migrating to the new,
modularized Core library.

  - [ ] Several objects (wallet pools, wallets, accounts, etc.) own shared pointers to common
    objects they need for some operations. That is a very OOP way of doing and we need to get rid of
    that, because it creates virtual, non-necessary links between objects. Such links, when needed,
    are there to showcase a strong relationship between too objects. A dependency relationship is
    not a strong relationship as it’s only important when doing some specific operations. An example
    of a strong relationship is owning (i.e. an object must drop a resource when it gets
    deallocated). The library is spread with such links and we need to remove most of them.
  - [ ] The `halfBatchSize` in `syncBatches` method was designed and imagined for Bitcoin, but that
    feature lives in abstract code, which doesn’t make any sense whatsoever anymore.
  - [ ] There’re many files dealing with `Abstract*`, `ApiImpl*` etc and we have no documentation
    nor design document to explain what those are and what was the rationale (i.e. why did we go for
    this design instead of something simpler / easier). I really think such a document would be very
    important.
  - [ ] The _core_ algorithms are not explained nor easy to find. For instance, the cryptographic
    code is well located (mostly in `math/`) but what is intrinsic to Bitcoin or to Ethereum is not.
    There are many folders and subfolders spawning that knowledge.
