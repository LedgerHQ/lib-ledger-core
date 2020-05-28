- RFC name: `0004-future-of-lib-core`.
- Status: `draft`.
- Author: [Dimitri Sabadie](https://github.com/phaazon)
- Date Created: 2020/05/27
- Date Updated: 2020/05/27
- Summary: Current situation of the Core library and proposal for its future.

<!-- vim-markdown-toc GFM -->

* [Motivation](#motivation)
* [Content](#content)
  * [Analysis of the architecture and inherent complexity](#analysis-of-the-architecture-and-inherent-complexity)
    * [Why do we need the Core library, again?](#why-do-we-need-the-core-library-again)
    * [C++](#c)
    * [In-house coin code](#in-house-coin-code)
  * [Next architecture](#next-architecture)
    * [Modularization](#modularization)
    * [Language](#language)
    * [Adapters](#adapters)
    * [Open-Source coin libraries](#open-source-coin-libraries)
    * [In-house specific coin code](#in-house-specific-coin-code)
* [Rationale](#rationale)
* [Related work](#related-work)

<!-- vim-markdown-toc -->

# Motivation
> What’s the context?

The current situation of the Core library requires some clarifications. Especially :

- We have less and less people working on it, which leads to harder maintenance for the few
  remaining.
- The responsibility scope of the library grows as we support new coins. Also, as we add new coins,
  the Core maintainers have to learn and take ownership of both the functional aspect of a coin (
  which is okay) but also the technical and very deeply technical aspect (which is to be
  discussed).
- The complexity of the code base and, especially, the language it is written in – C++,
  doesn’t help with reducing (it actually makes it worse) the number of runtime bugs we and — more
  importantly — our clients encounter. For instance, a look at the last Jira tickets and fix PRs
  show an important amount of patches about memory leaks, memory bad accesses, heap and stack
  corruptions and various other memory-based violation errors. Those bugs also are indirectly
  visible in other products, like in the Wallet Daemon where data consumption can skyrocket,
  especially regarding residential memory. If you are interested about why we should move away from
  C++, please have a look at [this][0], or [this][1], slide 10.

This RFC is a simple proposal to explain what the situation of the Core is and what it could be
in a short-to-medium term. As a record, the so-called “v2” version of the Core, a friendly
alias to the _modularized Core_, is set up to be live very soon as the time of writing this
document. Because the Core library is used in several projects (Live, Vault), we want to be sure
it will still be able to grow and bloom with less people working on it, which is a challenge for
all the reasons listed above.

# Content
> Content of the PoC with comments and step-by-step procedure.

## Analysis of the architecture and inherent complexity

This section focuses on the problems arising from the current design.

### Why do we need the Core library, again?

The Core library is a necessary mean in Ledger. The reason for this is pretty simple: currently, we
have two cryptocurrency-based products — namely, the Live and the Vault. Both are lead by different
teams. The complexity of blockchains – serialization, formats, broadcast protocols, explorers,
protocol updates, bare concept / consensus, etc. — makes it hard for a product team to focus on
what really matters at their level: the abstraction level.

The Core library provides an abstraction which scope encompasses:

- **Wallets**: a wallet is simply a collection of accounts for a given coin (i.e. BTC, ETH, XRP,
  etc.).
- **Accounts**: an account has the same meaning it would have in your bank, for instance. It is
  simply an entity that is associated with a list of operations / transactions and on which you can
  retrieve and perform various information and commands, like the getting the current balance,
  making and broadcasting transactions, etc.
- **Operations**: an operation is simply a high-level representation of a transaction. Typically,
  people should be interested in only two kind of operations: _send_ operations (when the user
  _spends_ their funds) and _receive_ operations (when they _receive_ funds). With more complex and
  modern blockchains, other types of operations exist (such as _delegation_), but those are more
  specific.
- **Transactions**: a transaction is always linked to a blockchain and provides more details. This
  cannot, most of the time, be a shared representation.

Atop all this, the Core library provides a way to access the features of each blockchains by
performing _downcasts_ (i.e. when you go from an abstracted data type to a specific version of it).

Now imagine the Core library doesn’t exist anymore. Each product team will have to look for
alternatives (or implement themselves), leading in non-shared effort. Worse, if a protocol update
happens, if the teams do not share the efforts, they are not going to fix the problem at the same
time, which can lead to incomprehension and issues.

It is also likely that, even though the Core library has issues, it still provides an ~easy way to
work with blockchains. Not using it might result in a much more convoluted and harder solution —
that’s up to what the product teams does and how they abstract the cryptocurrency code, though.

### C++

C++ is the language that was chosen to implement the library. As discussed in the [#motivation]
part, this language has proven over the last decades how _unsafe_ it is and how easy it is to shoot
ourselves in the feet. There are several problems:

- Memory unsafety is flaw, both for us (Core devs) but also for product teams. Most of the bugs our
  clients report are linked to stack / heap corruptions, null pointer exceptions, double frees, use
  after free, etc. The design of C++ doesn’t provide anything to fight against that, which is
  unfortunate.
- C interop. C++ needs special care to be able to interop with C, which means harder work to make a
  language C-compatible used in the Core library. For instance, replacing sub-components of the
  Core library with Rust, for instance, will require some extra time spent changing and adapting
  the ABIs (Rust is compatible with the C ABI, but not C++). This is not a blocker, but is
  definitely something to take into account.
- Finally, no one really understands C++. If you think you do, then there is something you’re doing
  wrong. That might feel funny or weird but the point is: if you want to have a perfectly sound
  C++ codebase, you need C++ experts (people working in the committee, with decades of years of
  experiences). The complexity of C++ and all the features it exhibits make it a very hard language
  to both learn and use. For instance, copy constructors are easily silently hidden, while copies
  are critical to performance. Move semantics is a bug nest. Inheritance leads to complex designs
  that sometimes require more code to remove the spaghetti than to solve an actual problem (i.e.
  virtual inheritance).

Introspection is interesting here: why do we need C++? What is the subpart of C++ that is
absolutely mandatory for the Core library to be possible? The answer to this question is
surprisingly easy: only a few algorithms require good performances, and most of the time, those are
not even linked to raw computing features, but mostly about how DB locking is performed or how HTTP
calls are parallelized and asynchronous. Our code base uses an in-house version of _futures_ and
asynchronous programming, leading to very unsafe and suboptimal asynchronous solutions.

The final point is: the scope of the library should require some code to be written, but when we
look at the code, we have much more code than what the scope seems to need. For instance:

- We have HTTP clients in-house implementations.
- We have websockets clients in-house implementations.
- We have SAX parsers in-house implementations.
- We have execution contexts, futures / promises in-house implementations.
- We have a minimal parsing library in-house implementation (`BytesReader` / `BytesWriter`).
- We have a set of collection utilities and especially runtime dynamic algebraic types, in-house
  as well.
- We have a complete ORM in-house implementation based on `soci`!
- We have a pub/sub in-house mechanism.
- We have various helpers to convert dates.
- We have in-house coin code (i.e. the implementation of a coin is completely on our side, we don’t
  use any open-source libraries where experts of such a coin would provide everything).
- We maintain an absurd amount of vendored dependencies. This is subject to change the day we use
  a package manager, but that day is still to come.

### In-house coin code

The point about in-house coin code is critical. Right now, we support several coins:

- Bitcoin and alt coins..
- Ethereum and alt coins..
- Ripple.
- Tezos.
- Stellar (not currently modularized).
- Cosmos (not currently modularized).
- Algorand (not currently modularized).

A single coin requires to, at least, add code (both legacy and modularized) and also modify a large
amount of code (legacy only). The code that needs to be written is business code: we have to
understand the protocol stack, the serialization stack, etc. Every time an update of a protocol is
required, the Core members have to update the code.

Right now, we do not have any external dependencies regarding the code. The decision made about
that topic is unfortunately not documented, but it is safe to assume that:

- In-house code allows to be more flexible about the evolution we can make on a coin.
- It also implies that we have to write all the performance code ourselves. History has shown that
  performance needs tweaking, and the more coins we add, the less likely it is we have time to work
  on this.

## Next architecture

This section focuses on the suggested next-to-be architecture.

### Modularization

The modularization is a necessary step towards a better architecture. When it was imagined,
designed and implemented (Dimitri), the goals were:

- Reduce the cognitive complexity of the library so that on-boarding (both for Core members or
  external contributions — e.g. Coin Integration) and maintenance is easier. Especially, adding
  a new coin on the legacy library implies hundreds of new files and 10k+ changes. Reviewing
  such PRs sometimes takes weeks if not months.
- Clean up the Core architecture. The entanglement of the abstraction and the implementation is
  very strong in the legacy (there is no clear distinction between what is interface code and
  what is implementation code). For instance, the `Operation` type has technical details about
  all possible implementations (`BitcoinLikeOperation`, `EthereumLikeOperation`, etc.). That leads
  to coupled code: if one wants to add a new coin, they have to edit `Operation` to add the
  knowledge of the coin there. The modularization removes that need, making the whole _“adding a
  new coin”_ an ad-hoc only process, not an update one.
- Transform the library into smaller and loosely coupled parts that would only depend on the
  abstraction. `ledger-core-bitcoin` only depends on `ledger-core`, so it’s easy to switch the
  implementation of that code. The goals there were to be able to switch the coin code from
  in-house to external (by using a Bitcoin library for instance) or even interfacing with a better
  language, such as Rust, Go, etc.

### Language

The language part is a huge and important decision with the Core library. Most of our products are
using safe languages, ranging from JavaScript to Python and Scala. The fact the library is written
in C++ implies that they lose all the safety from their languages, because the common denominator —
C++ — is highly unsafe. As stated in the analysis section, that leads to a vast set of runtime bugs
that are sometimes very hard to fix — stack and heap corruptions, among others.

Some people are emotionally attached to C++, but really, it’s not about preferences, but
_production_. If companies like Google or Microsoft decided to move away from C++, showing similar
polls and stats about CVEs and runtime bugs (around ~70% due to memory safety), it would be
unprofessional not to take those into account.

Unfortunately, we cannot move away from C++ very quickly. The problem is that the toolchain — mostly
`djinni` — and the way we abstract and interface code (inheritance) makes it hard to move away from
C++ right now. However, an important note:

1. C++ is used for the interface (inheritance).
2. C++ is used for the implementation (actual code doing protocols, HTTP requests, DB
  connections / queries, balance history algorithms, etc.).

While it’s very unlikely we get enough resources and people to fix (1.), fixing (2.) should be
doable on a mid-term basis on the modularization part (not possible on the legacy). Because each
coin will be modularized, we can safely create another implementation of the same coin, but with a
different language (Rust, Go, Zig, Java, Haskell, Scala, whatever). The ABI and linking processes
should be discussed in another RFC but the fact each coin is independent from each other should
allow switching to either another language, or external dependencies.

The C++ interface code is not what creates bugs in production, so it shouldn’t a be problem for
at first. Basically, it’s a bunch of interfaces (abstract classes with virtual pure methods), with
no implementation. Because no implementation is living in this part of the code, no bugs can exist.
Bugs can appear, however, at link time (it has been the case with _OpenSSL_) but it should be very
rare.

Keeping C++ for the interface of the library and switching implementation to something safer should
enhance the stability of the library by a large magnitude. Until we find a better solution to
completely ditch C++.

### Adapters

The scope of the Core library should be narrowed. Writing in-house code coin provides interesting
flexibility, but at the same time, it also binds us more to the business rules of a coin. What it
means is that maintainers _must_ be aware of all the technical details of a coin — even the most
technical one in the consensus algorithm. For instance, the Tezos Babylon 2.0 update introduced a
breaking change in the way fees are computed. That details is burried deep in technical discussions
and finding it was _hard_. Worse, the nodes got updated without us giving the chance to update our
code. We don’t have any _“Tezos referent”_ people. So… we have to keep up-to-date about what a coin
is about, but on a much much more grainer level than just the bare concepts of the coin. In the case
of Tezos, the change implied to remove spending fees on KT1 accounts and make a pass in the tz1
accounts to take into account associated KT1 accounts so that we could zip output transactions and
take the fees into account.

Now multiply that kind of knowledge by the number of coins we support (and will support). It just
doesn’t scale, especially given the number of people working on the Core library and Coin
Integration. A decision must be made:

1. Do we want to be the owners of the coin code? If so, we need coin referents and coin experts
  **for each blockchain**.
2. Or do we want to outsource the complexity of a blockchain to a library?

(2.) is discussed in the next section. Whatever the choice, there is something important to notice:
sometimes, a coin might not have a library available for everybody to use. In that case, we will
still need an in-house solution. To solve that problem, we should be writing _adapters_.

An adapter is simply a funny name to describe an indirection. Supporting a coin in the Core library
really is more about the abstraction we provide the products than the implementation itself. A coin
must have a wallet abstraction. It must have an account abstraction. An operation and transaction
abstraction. The rest is just implementation details. An adapter would allow to _adapt_ an
implementation (in-house, open-source library, etc.) to the Ledger representation of a coin. Such a
representation will be deeply influenced by our explorers – because we might query things from them
— and by the broadcast mechanism. Besides that, we are free to use what we want.

Adapters are a simple solution to enjoy both worlds: open-source libraries and in-house
implementations otherwise.

### Open-Source coin libraries

Open-source coin libraries should be preferred over in-house solution. The reason for this is that
libraries are versioned. We can pin-point a version implementation, follow SemVer and update the
code base at the pace we want. But more importantly, if a protocol update occurs, experts will be
writing the code. For instance, if a completely new coin appears, should we expect our developers
to start getting all the deeply technical details about it? Or should we just write some adapters
to transform and extract the data we are interested from the blockchain? I think the latter option
should be the way to go.

Before jumping in, another RFC should be written to list all the open-source contributions about
those coins, to be sure we can migrate easily and remove our code:

- Bitcoin, with support of the various required SLIPs, like RBF, for instance.
- Ethereum (do not forget ERC20).
- Ripple.
- Tezos.
- Stellar.
- Cosmos.
- Algorand.

An important note here: if we fix something, we should open pull requests to those FOSS projects.
Forking and maintaining / vendoring local versions of dependencies creates technical debt.

### In-house specific coin code

As mentioned above, in-house code will be required. Two possible situations:

- No library exists for a given coin. We will have to write the implementation.
- A library exists, but lacks some features we need.

The flexibility here must be lead by adapters, and especially the design of the code of the
adapters. However, in-house code shouldn’t be the default anymore, because it makes us
loosely-coupled with the protocols: if a coin updates, we now have to update without knowing
it (or we need a technical watch mechanism, which doesn’t prevent delay / lag). We should use
open-source libraries as much as possible to benefit from what others do.

# Rationale
> Should we go for it? Drop it?

The Core library is a necessary piece of software for Ledger. Without it, the effort to support
new coins is _N_ times the one made in the Core library (_N_ being the number of product teams).
The modularization will bring a lot of fresh air and opportunities to make things easier for
everybody:

- The Coin Integration team will be able to move faster, without having to update code anymore.
- The Core team should be able to replace parts of the library, by switching language or using
  FOSS alternatives.

# Related work
> What else has been done and is similar?

Please refer to the following documents about the modularization of the Core library
for further technical decisions:

- [RFC-0001] about the PoC of the modularization.
- [RFC-0002] about the Node packaging.
- [Ripple segregation] for some (very) technical notes about the segregation of Ripple.

[0]: https://www.zdnet.com/article/chrome-70-of-all-security-bugs-are-memory-safety-issues
[1]: https://github.com/Microsoft/MSRC-Security-Research/blob/master/presentations/2019_02_BlueHatIL/2019_01%20-%20BlueHatIL%20-%20Trends%2C%20challenge%2C%20and%20shifts%20in%20software%20vulnerability%20mitigation.pdf
[RFC-0001]: https://github.com/LedgerHQ/lib-ledger-core/blob/design/modularization/doc/rfcs/0001-modularization.md
[RFC-0002]: https://github.com/LedgerHQ/lib-ledger-core/blob/design/modularization/doc/rfcs/0002-modularized-node-packaging.md
[Ripple segregation]: https://ledgerhq.atlassian.net/wiki/spaces/LLC/pages/1258618891/Ripple+Segregation
