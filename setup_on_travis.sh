#!/usr/bin/env bash

if [ "$TRAVIS_OS_NAME" == "linux" ]; then
    echo "deb http://ftp.debian.com/debian/ unstable main" | cat - /etc/apt/source.list | sudo tee /etc/apt/source.list
    sudo add-apt-repository "deb http://ftp.debian.com/debian/ unstable main"
    sudo apt-get update
    sudo apt-get -o Dpkg::Options::="--force-all" -y --force-yes install gcc-5 g++-5
    sudo cp /bin/true /usr/bin/dpkg-maintscript-helper
    bash tools/install_mingw64.sh
elif [ "$TRAVIS_OS_NAME" == "osx" ]; then
   brew update
   brew upgrade cmake || true
   brew install wget || true
   wget https://dl.google.com/android/repository/android-ndk-r13b-darwin-x86_64.zip && unzip android-ndk-r13b-darwin-x86_64.zip > /dev/null
fi;
