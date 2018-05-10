#!/bin/bash

GIT_REPO="https://github.com/LedgerHQ/lib-ledger-core.git"
GIT_BRANCH="develop"
GIT_BIN=$(which git)
APT_BIN=$(which apt)
CMAKE_BIN=$(which cmake)
MAKE_BIN=$(which make)
WGET_BIN=$(which wget)
EXIT_SUCCESS=0
EXIT_FAILURE=1
BUILD_DIR=$(mktemp -d /tmp/build.XXXXXXXXXX) || { echo "Failed to create temp file"; exit $EXIT_FAILURE; }

function check_deps() {
  if [ -z "${1}" ]; then
    echo "Missing dependencies: ${2}, installing it for you"
    apt-get -y install ${2}
  fi
}

function exit_on_error() {
  if [ $1 != 0 ]; then
    echo "Oooooops something went wrong. Exiting"
    exit $EXIT_FAILURE
  fi
}

while getopts "rb" option; do
  case $option in
  r)
    GIT_REPO="${OPTARG}"
    ;;
  b)
    GIT_BRANCH="${OPTARG}"
    ;;
  j)
    BUILD_OPS="-DTARGET_JNI=on"
    ;;
  *)
    usage: "${0} [-r git repository] [-b git branch] [-j to enable jni bindings]"
    exit $EXIT_FAILURE
    ;;
  esac
done

check_deps "${GIT_BIN}" "git"
check_deps "${CMAKE_BIN}" "cmake"
check_deps "${MAKE_BIN}" "make"
check_deps "${WGET_BIN}"

cd ${BUILD_DIR}

# Check cmake version and install it when required
cmake_version=$($CMAKE_BIN --version | head -n 1 | awk '{print $3}')
cmake_major=$(echo ${cmake_version} | cut -f 1 -d.)
cmake_minor=$(echo ${cmake_version} | cut -f 2 -d.)

if [ $cmake_major -lt 3 ] || [ $cmake_minor -lt 10 ]; then
  echo "Cmake is too old, building it for you"
  ${WGET_BIN} https://cmake.org/files/v3.10/cmake-3.10.3.tar.gz
  tar -xvf cmake-3.10.3.tar.gz
  cd cmake-3.10.3
  mkdir ${BUILD_DIR}/cmake
  ./bootstrap --prefix=${BUILD_DIR}/cmake
  make
  make install
  export PATH=${BUILD_DIR}/cmake/bin:${PATH}
fi

echo "Cloning the libcore sources"
cd ${BUILD_DIR}
src_dir=$(echo ${GIT_REPO} | rev | cut -d'/' -f 1 | rev | cut -f 1 -d.)
git clone "${GIT_REPO}"
cd ${src_dir}

if [ "$(git branch | grep '^*' | awk '{print $2}')" != "${GIT_BRANCH}" ]; then
  git checkout -b "${GIT_BRANCH}" "origin/${GIT_BRANCH}"
fi

git submodule update --init
mkdir build
cd build
cmake "${BUILD_OPS}" ../
exit_on_error $?

make
exit_on_error $?

EXIT $EXIT_SUCCESS
