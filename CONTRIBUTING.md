# Contributing to Ledger Core library

You’ve decided to help us build an awesome piece of software, thank you for this. This document
gathers important information you should read prior to starting working or even opening pull
requests on the repository. Please open issues if you think some important material is missing from
this very document.

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
  - We *tag* stable releases on `master` with [SemVer].
  - We *tag* instable releases on `develop` with the special `-rc` suffix to support *release
    candidates*.

## About coin integration

We might accept coin integration at some day but currently, **we do not accept pull-requests that add
new coin support**. Also, we don’t want you to waste your time, so please contact us (open an issue,
for instance) if you want to integrate a new coin. Thank you.

[gitflow]: https://fr.atlassian.com/git/tutorials/comparing-workflows/gitflow-workflow
[SemVer]: https://semver.org
[CMakeLists.txt]: ./CMakeLists.txt
