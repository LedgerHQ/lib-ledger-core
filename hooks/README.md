# Hooks

## Auto formatting

In purpose to make the codebase more consistent, all source files should be formatted using the `clang-format` version 8.0+ and `cmake-format`.
There are several ways to automatize the formatting :
1. Enable formatting on commit(s). For instance, you should run the below command :
    ```
    cd lib-ledger-core
    git config core.hooksPath ./hooks
    git config hooks.autoFormat true
    ```
2. Use a proper IDE plugin and format on save
3. Call `make clang-format && make cmake-format` to format the core (or coin) library