# Easily setup a playground for libcore and Live Desktop!

You will need [docker] for this to work.

  1. `./manage build` will initiate the build of the docker image. You are likely to only need this
    command to get started.
  2. If you want to build the image again because a dependency has changed (or your branch has new
     commits), you can use `./manage rebuild`.
  3. You can either spawn and enter a container to do your own business…
  4. …or start Live Desktop: `docker run -it ledger/playground`.
  5. Have fun.

> Currently, customization is limited but in a very near future, you should be able to pass
> arguments to ./manage build or ./manage rebuild to specify the branch or even fork you want to
> work with.
