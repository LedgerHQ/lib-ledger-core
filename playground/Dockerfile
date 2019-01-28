FROM debian:buster

ARG LIBCORE_BRANCH=develop

# Install build and dev dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    cmake \
    curl \
    git \
    libXss1 \
    libasound2 \
    libgconf-2-4 \
    libgtk2.0 \
    libnss3 \
    libqt5websockets5-dev \
    libudev \
    libudev-dev \
    python3 \
    qt5-default \
    qtbase5-dev

# nvm
COPY install_nvm.sh /opt/install_nvm.sh
RUN export NVM_DIR="$HOME/.nvm"
RUN "$NVM_DIR/nvm.sh"
RUN nvm install 8.14.0
RUN nvm use 8.14.0

# yarn
RUN npm i yarn -g

# Get libcore, bindings and Live Desktop
RUN mkdir /ledger
WORKDIR /ledger
RUN git clone --depth 1 --recurse-submodules --branch $LIBCORE_BRANCH https://github.com/LedgerHQ/lib-ledger-core.git
RUN git clone --depth 1 https://github.com/LedgerHQ/lib-ledger-core-node-bindings
RUN git clone --depth 1 --branch develop https://github.com/LedgerHQ/ledger-live-desktop.git

# Compile libcore
RUN mkdir -p lib-ledger-core/build
WORKDIR /ledger/lib-ledger-core/build
RUN pwd
RUN cmake -DCMAKE_BUILD_TYPE=Debug ..
RUN make -j8

# Node binding
WORKDIR /ledger/lib-ledger-core-node-bindings

# Apply a patch to ensure we don’t grab libcore library from S3 but use ours instead
RUN sed -i /"python preinstall"/d package.json

# Copy libcore library
RUN mkdir -p {lib,build/Release}
RUN cp /ledger/lib-ledger-core/build/core/src/libledger-core.so lib
RUN cp /ledger/lib-ledger-core/build/core/src/libledger-core.so build/Release

# Register the binding with yarn
RUN yarn link

# Live Desktop
WORKDIR /ledger/ledger-live-desktop

# Link to the node binding
RUN yarn link "@ledgerhq/ledger-core"

# Compile Like Desktop
RUN yarn install

# Aaaaand we’re done; default to running Live Desktop
CMD ["yarn", "start"]
