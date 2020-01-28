# Account synchronization mechanism

This document explains how a synchronization is performed in abstract terms.

## Overall principle

Account synchronization is performed via a blockchain explorer. This is needed as a synchronization
process needs to access transactions and metadata about the blockchain it operates on. In order to
synchronize an account, its associated wallet must also be accessible. An object holding the
wallet configuration must be accessible as well. That object is gotten from the wallet. A keychain,
used to derive addresses and keys, must be around — again, the account provides it. That is pretty
much everything really mandatory we need to perform a synchronization.

> Everything about logging, persistence and preferences can be ignored for the sake of understanding
> the raw synchronization mechanism.

This schema sums up the objects that are needed:

```text
Synchronizer.synchronize(Account)
                            ^
                            │
                          Wallet
                            │
                ┌───────────┴───────────┐
                v                       v
          Configuration              Keychain
```

A wallet is just a lib-core concept that gathers, for the same coin network, several accounts. It
shouldn’t have to provide `Configuration` nor `Keychain`.

Transactions are streamed from an explorer via a mechanism of batches.

1. First, we create a _buddy_ object. That object is badly named and serves as a _synchronization
  state_ only. It shares several properties from the account and the application configuration.
2. The first important thing to do is to check whether a reorganization had happened. That is
  encoded in our library if there’s a saved state regarding synchronization (i.e. a previous
  synchronization has put some state in the preferences local storage). If that’s the the case:
  1. We take the deepest batches from the saved state by ordering them by block height.
  2. We look at blocks one by one and check whether they exist in database. We stop at the first
    block that doesn’t exist in database and its height is the _deepest failed block height_.
  3. If the _deepest failed block height_ doesn’t refer to the genesis block, we get the last block
    currently living in database and then for all batches in the buddy saved state, if the batch’s
    block height is greater or equal to the _deepest failed block height_, we update the batch
    block’s height and hash according to the value of the last black from DB.
3. We create / initialize the saved state and update both the current block and the transactions to
  drop — i.e. that is, transactions without an associated block.
4. The interaction with the explorers starts here. The current code is based on a recursing future,
  `synchronizeBatches`, that takes the current batch index as argument.
5. The `synchronizeBatch` is then called with the batch index. That function performs several
  derivation of the observable addresses from the keychain of the account. This gives us several
  addresses we send to the explorer by asking for the list of transactions.
  1. We don’t get the whole set of transactions directly but instead a _transactions bulk_ — an
    object gathering some but not all of the asked transcations.
  2. Each transaction in that bulk object are put into database and removed from pending
    transactions if they were in.
  3. We loop back by recalling `synchronizeBatch` if there’s still transactions in the bulk.
6. If the account can have multiple addresses, we recall `synchronizeBatches` until we don’t get
  any transaction and that we pass the limit of discoverable addresses without transaction.

[crypto assets repository]: https://github.com/LedgerHQ/crypto-assets/tree/master/coins
