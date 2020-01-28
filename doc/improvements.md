# Future codebase improvements

This document gathers all improvements we would like to implement in a future iteration of the Core
library. There is no order nor priority, just a storm of ideas.

<!-- vim-markdown-toc GFM -->

* [What’s the real value of lib-ledger-core?](#whats-the-real-value-of-lib-ledger-core)
* [Operation](#operation)
  * [Let's improve it!](#lets-improve-it)
* [Inheritance](#inheritance)
  * [Clean intermediate classes](#clean-intermediate-classes)
  * [Composition over inheritance](#composition-over-inheritance)
* [Smart pointers](#smart-pointers)
  * [Define ownership](#define-ownership)
* [Redundant names](#redundant-names)
  * [Namespace by library](#namespace-by-library)
  * [Remove redundant information](#remove-redundant-information)
* [Remove or review abstract types](#remove-or-review-abstract-types)

<!-- vim-markdown-toc -->

## What’s the real value of lib-ledger-core?

The _core_ algorithms are not explained nor easy to find. For instance, the cryptographic code is
well located (mostly in `math/`) but what is intrinsic to Bitcoin or to Ethereum is not.
There are many folders and subfolders spawning that knowledge. Worse, it’s hard to pinpoint what
exactly `lib-ledger-core` provides.

## Operation

Our current implementation of the `Operation` class is extremely dependent of the
**coin transaction** and the **explorer transaction** which yields several heat points:

- We cannot create an operation without providing the underlying transaction. This implies that
  the operation can be set to a poor or weird intermediate state: any change to the operation is not
  reflected to the underlying transaction and vice versa.
- Operation is a public-facing view-only representation of transactions. An operation is just a
  projection of transaction or, if you prefer, a reinterpretation. For instance, a single
  transaction on the Bitcoin blockchain that sends funds from `A` to `B` but also sends funds from
  `A` to `A` (change) will generate two virtual operations: one for sending to `B` and one for
  change.

### Let's improve it!

1. Separation of concerns. The `Operation` should represent something abstract and
  agnostic whatever the underlying transaction is. If some code needs the transaction
  attached to an operation, it should treat both objects explicitly but not as nested.
2. Remove duplicated data. So far, the `Operation` class holds some duplicated data
  from the underlying transaction which also holds duplicated data from the explorer
  transaction. We need to determine whether there are any reason for such duplications, which
  can be translated as _“Is it possible to mutate an operation without changing the transaction?”_

## Inheritance

Our codebase is several years old now. Since modern C++ brings a lot of new features and new
paradigms come to life - and other die - we probably might do much better. For instance, we have a
lot of inheritance which doesn't add any value and this is quite the **opposite** in fact: the
codebase becomes more and more complex.

### Clean intermediate classes

First thing first, we have a first layer of inheritance which serves as interface for all our
bindings. However, under this first layer we also have multiple - and most of the time - poor layer
of inheritance which seems to be here for coin integration. By reworking the interfaces, we probably
might throw away all this undesired intermediate classes and make the codebase more and more clean.

Furthermore, the work on [Ubinder] might be a big game changer here as using inheritance to modelize
public contracts will make zero sense.

### Composition over inheritance

On top of that cleaning, our codebase is strongly stucked in the old fashion OOP: **inheritance
everywhere.** Instead of overabusing inheritance, we can try to use composition by assembling
smaller parts with less cognitive complexities and smaller scopes. It would:

- Help separate data from logic.
- Ease composition of small things to provide new complex combinations.
- Debugging is often easier with composition as no virtual tables are involved.

## Smart pointers

The first assumption we can make about our usage of smart pointers is we use them too much and not
properly. After a quick glance to our codebase, `shared_ptr` (but also `weak_ptr`) is
**everywhere**. `shared_ptr` shouldn't be used by default for several reasons:

- Having several simultaneous owners of a resource implies duplicating ownerships. Per-se, it’s not
  necessarily a problem, but it’s often the sign of an overengineering usage of ownership. If one
  doesn’t need ownership, giving them ownership is too much power. Plus, it makes it harder to
  determine objects lifecycles, especially when we explicitly want to destroy objects (from the
  _real_ owner point of view).
- **Thread-safety** is harder.
- It can add a **performance** cost, both in time and memory because of the
  underlying reference counter. For some parts of the code, we use `weak_ptr` to mitigate the
  ownership issue discussed above. That implies a _lock_ when accessing the resources, creating a
  new class of problem when trying to access the resource concurrently (dead-locks, race condition,
  etc.).

### Define ownership

1. Use `unique_ptr` to give/take the ownership of a resource. No one else will delete this resource
  and we get the confidence that we are free to modify the value pointed by the `unique_ptr`.
2. Use references to borrow a resource for a **scoped time**. References represent a temporary
  access to an object (immutable or mutable). Most of the time, they don’t need to take ownership.
  References should be used as much as possible unless one needs to take ownership or the lifecycle
  of the object is not known.
3. Raw pointers should be avoided as much as possible as they bring no value compared to reference
 except for optional typing and for interfacing with FFI (especially with C, which requires
 `nullptr`, for instance).
4. Use `shared_ptr` and `weak_ptr` for all other cases where ownership is involved.

## Redundant names

Our codebase contains very **verbose class names** specifically the ones related to coins. This
brings the cognitive complexity (but also fuzzy searching files) to the highest level.

### Namespace by library

The first thing we need to add to avoid name clashing is a proper namespace by library.
We already have one for `core` - let's do the same for `ethereum`, `bitcoin` and so on. There’s
also nothing wrong in adding more namespacing if that makes sense inside the same library.

### Remove redundant information

We can remove the information in each class name that refers to the coin. For instance, a
`BitcoinLikeAccount` will become `bitcoin::Account` — we could even decide to go with
`btc::Account`. We have to discuss about the usage of the `Like` word in class names. It has a real
meaning: it states that we’re talking about a coin family and forks / subcoins will work too.
However, maybe the information of the namespace is enough to deduce it’s a family.

## Remove or review abstract types

Currently, there’re many files dealing with `Abstract*`, `ApiImpl*` etc. and we have no documentation
nor design document to explain what those are and what was the rationale (i.e. why did we go for
this design instead of something simpler / easier). Those abstract types are very important as they
are intensively used throughout the library.

[Ubinder]: https://github.com/LedgerHQ/ubinder
