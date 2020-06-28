FROM debian:stretch

COPY .circleci/ circleci
WORKDIR circleci

RUN ./setup_debian.sh Release
RUN ./cmake-install.sh

ENV PATH="${PATH}:/root/cmake_folder/bin"

