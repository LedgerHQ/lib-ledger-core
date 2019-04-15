#!/usr/bin/env bash

BUILD_CONFIG=$1

echo ">>>>>>> GETTING CIRCLE_TAG : $CIRCLE_TAG"
echo ">>>>>>> GETTING CIRCLE_BRANCH : $CIRCLE_BRANCH"

echo "========> Install basic config"
apt-get update
apt-get install -y apt-transport-https wget python build-essential libx11-xcb-dev
apt-get install -y libssl-dev curl tcl

echo "========> Install Java"
apt-get install openjdk-8-jdk -y
echo "========> Java Installed"
echo $JAVA_HOME
ls -la /usr/lib/jvm/java-8-openjdk || echo "!!!! java openjdk not found"

if [ "$BUILD_CONFIG" == "Release" ]; then
    apt-get install -y awscli
fi

echo "========> Install C++ dependencies"
apt-get install -y g++-7 make
update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-7 700 --slave /usr/bin/g++ g++ /usr/bin/g++-7
export PATH=$HOME/cmake-3.12.3/bin:$PATH

if [ "$BUILD_CONFIG" == "Debug" ]; then
    echo "========> Install Qt5"
    apt-get install -y qt5-default libqt5websockets5 libqt5websockets5-dev
fi

echo "========> Install Sqlite"
apt-get install -y sqlite3 sqlite libsqlite3-dev

#if [ "$BUILD_CONFIG" == "Debug" ]; then
    echo "========> Install node"
    curl -sL https://deb.nodesource.com/setup_9.x | bash -
    apt-get install -y nodejs
    echo "========> Install yarn"
    curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | apt-key add -
    echo "deb https://dl.yarnpkg.com/debian/ stable main" | tee /etc/apt/sources.list.d/yarn.list
    apt-get update && apt-get install -y yarn
#fi

