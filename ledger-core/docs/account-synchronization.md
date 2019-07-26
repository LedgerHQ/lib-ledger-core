# Account synchronization mechanism

This document explains how a synchronization is performed in abstract terms.

## Overall principle

Account synchronization is performed via a blockchain explorer. This is needed as a synchronization
process needs to access transactions and metadata. In order to synchronize an account, its
associated wallet must also be accessible. An object holding the configuration must be accessible
too. That object is gotten from the wallet as well. A keychain, used to derive addresses and keys,
must be around — again, the account provides it. That is pretty much everything really mandatory
we need to perform a synchronization.

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

Transactions are streamed from an explorer via a mechanism of batches.

  1. First, batch indexed with `α` is retrieved.
    1. We derive all observable addresses from the keychain for the `α` batch. That algorithm works
      by indexing the range of addresses to retrieve with the batch index multiplied by the size of
      half a batch. That is due to the fact we need both send and receive addresses.
    2. All the derived addresses form a _batch_ of strings that is sent to the explorer along with
      the block hash of the current state.
    3. Transactions are gotten from explorers by joining addresses altogether with commas
      separated lists. They’re then injected in HTTP queries. An HTTP token is stored in the
      `X-LedgerWallet-SyncToken` HTTP header.
    4. The endpoint used is `"/blockchain/{}/{}/addresses/{}/transactions{}"`, where the `{}` are,
      respectively:
      1. The explorer version. We use mostly `v2` and `v3` now. The main differences between `v2`
        and `v3` is a difference of implementation (different internal architecture and
        infrastructure), for most part.
      2. The identifier of the blockchain network we target. For instance, for bitcoin, we use
        `"bitcoin"`.
      3. The list of addresses, comma-separated.
      4. A list of parameters for the explorers to work with.
    5. The result is then parsed as JSON by using a `LedgerApiParser` into a `TransactionsBulk`
      depending on the type of coin.
  2. The resulting batch is then inspected. For all transactions:
    1. We look for _pending transactions_. If one is found, it is injected the transaction;
      otherwise it’s erased.
    2. The transaction is injected into the account and the last block is updated.
