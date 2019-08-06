# Contributing to Ledger Core library

You’ve decided to help us build an awesome piece of software, thank you for this. This document
gathers important information you should read prior to starting working or even opening pull
requests on the repository. Please open issues if you think some important material is missing from
this same document.

## Workflow

  - No one directly work on the central repository; we use forks.
  - We use [gitflow].
    - The `master` branch only contains a set of curated and stable features.
    - The `develop` branch is the branch you should start your contribution from.
    - Everyone creates new *feature branches* in order to implement something. We don’t enforce the
      naming of the branch, but we highly recommend to use special prefix:
        - `fix/`: if your branch is about fixing a bug, a regression or anything that requires a
          patch to a bad behavior.
        - `feature/`: if your branch introduces a new concept or an enhancement.
        - `misc/`: for anything else (like adding docs, READMEs, etc.).
  - We open a *pull request* when we think the branch should be merged. **Becareful: when opening
    the pull request, select `develop` as base branch!**
  - Do not forget to update the CHANGELOG.md **before** opening a pull request.
  - We review the *pull request*. We don’t set a fixed number of reviewers, but don’t forget the
    golden rule: the more people review, the better.
  - If your change impacts no path from the CI (for instance, you just added a new tool or fixed a
    typo in documentation, even in code), please include the special `[skip ci]` in one or all
    commit(s) in your PR. This will help ease runners and ensure a better collaboration between
    developers.
  - When we have reviewed — and you have addressed everything — we shall merge your contribution
    into the `develop` branch. It’s possible that, given your contribution, we also ask you to
    update the version of the library.
      - In [CMakeLists.txt], edit the `VERSION_MAJOR`, `VERSION_MINOR` and `VERSION_PATCH` according
        to your changes.
      - Do the same thing in [appveyor.yml] for the `LIB_VERSION` key.
      - [We appologize for not having automated that yet.](#117)
  - Do not forget to bump the version, if needed, **before** opening a pull request
  - We *tag* stable releases on `master` with [SemVer].
  - We *tag* instable releases on `develop` with the special `-rc` suffix to support *release
    candidates*.

If you would like to know more about the architecture and the internals (configuring / building),
feel free to refer to [this document](doc/architecture.md).

## Release process
  - Development: fixes, features are done on specific branches on developers' fork,
  - Pull requests: submitted to LedgerHQ/develop with updated CHANGELOG.md,
  - Merge into `LedgerHQ/develop`: triggers CI builds and deploys under `x.y.z-rc-commitHash`,
  - Merge into `LedgerHQ/master`: once a version is confirmed to be stable at least by one of our clients
    (Vault or Live), we bump project’s version, for this we have a special pull request. Once merged in
    `LedgerHQ/master` and tagged as a stable release `x.y.z`, a build and a deployment are triggered.
    To sum up, tags point to stable releases that are in production. **These versions should be used by any application
    client of libcore**
  - Patches: only applied on previously affected releases that has been in production, to apply a patch to a version `x.y.z`,
    we create a branch from master at `x.y.z` tag, push all fixes to it, once stable (confirmed by client)
    we tag a new patched stable release (e.g. `x.y.(z+1)`). Then this branch is merged to `LedgerHQ/develop`.
    Each 3/4 weeks we clean upstream from unused branches.

<p align="center">
 <img src="/ressources/patch.png" width="550"/>
</p>

## About coin integration

We might accept coin integration at some day but currently, **we do not accept pull-requests that add
new coin support**. Also, we don’t want you to waste your time, so please contact us (open an issue,
for instance) if you want to integrate a new coin. Thank you.

[gitflow]: https://fr.atlassian.com/git/tutorials/comparing-workflows/gitflow-workflow
[SemVer]: https://semver.org
[CMakeLists.txt]: ./CMakeLists.txt
