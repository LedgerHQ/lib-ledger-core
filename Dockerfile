FROM debian:stretch

# Install build dependencies

# Enable sourcefiles repositories, for use with apt build-dep
RUN echo "deb-src http://deb.debian.org/debian stretch main" >> /etc/apt/sources.list \
  && echo "deb-src http://deb.debian.org/debian-security/ stretch/updates main" >> /etc/apt/sources.list \
  && echo "deb-src http://deb.debian.org/debian stretch-updates main" >> /etc/apt/sources.list

RUN apt update

RUN apt-get install -y \
  build-essential \
  qt5-default \
  qtbase5-dev \
  libqt5websockets5-dev \
  git \
  curl \
  zsh \
  wget

# Dependencies for building cmake
RUN apt build-dep -y cmake

# Get and build a more up to date version of cmake
RUN cd /usr/local/src \ 
  && wget https://cmake.org/files/v3.12/cmake-3.12.4.tar.gz \
  && tar xf cmake-3.12.4.tar.gz \ 
  && cd cmake-3.12.4 \
  && ./bootstrap --system-curl -- -DCMAKE_BUILD_TYPE:STRING=Release \
  && make -j4 \
  && make install \
  && cd .. \
  && rm -rf cmake*

RUN zsh -c "$(curl -fsSL https://raw.github.com/robbyrussell/oh-my-zsh/master/tools/install.sh)" || true

# Move library files to the image

ADD . /lib-ledger-core
WORKDIR /lib-ledger-core
RUN git clean -fdx

CMD zsh