#!/usr/bin/env bash

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
    echo "deb http://ftp.debian.com/debian/ unstable main" | cat - /etc/apt/source.list | sudo tee /etc/apt/source.list
    sudo add-apt-repository "deb http://ftp.debian.com/debian/ unstable main"
    sudo apt-get update
    sudo apt-get -o Dpkg::Options::="--force-all" -y --force-yes install gcc-5 g++-5
    sudo cp /bin/true /usr/bin/dpkg-maintscript-helper
    sudo apt-get -o Dpkg::Options::="--force-all" -y --force-yes install mingw-w64
elif [ "$TRAVIS_OS_NAME" == "osx" ]; then
   brew update
   brew upgrade cmake || true
fi;