FROM debian:stretch

COPY .circleci/ circleci
WORKDIR circleci

RUN apt-get update
RUN apt-get -y install git ssh
RUN ./setup_debian.sh
RUN ./cmake-install.sh

ENV PATH="${PATH}:/root/cmake_folder/bin"

