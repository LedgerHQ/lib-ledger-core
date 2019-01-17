FROM debian:buster

# Install build dependencies

RUN apt update
RUN apt install -y cmake
RUN apt install -y g++
RUN apt install -y qt5-default
RUN apt install -y qtbase5-dev
RUN apt install -y libqt5websockets5-dev
RUN apt install -y git
RUN apt install -y curl
RUN apt install -y zsh
RUN zsh -c "$(curl -fsSL https://raw.github.com/robbyrussell/oh-my-zsh/master/tools/install.sh)" || true
RUN apt install -y gdb

# Move library files to the image

ADD . /lib-ledger-core
WORKDIR /lib-ledger-core
RUN git clean -fdx

CMD zsh