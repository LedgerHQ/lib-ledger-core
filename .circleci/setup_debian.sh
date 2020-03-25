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
    echo "========> Install sbt"
    curl -L -o sbt-1.2.8.deb https://dl.bintray.com/sbt/debian/sbt-1.2.8.deb
    dpkg -i sbt-1.2.8.deb
    rm sbt-1.2.8.deb
    sbt sbtVersion
fi

echo "========> Install C++ dependencies"
apt-get install -y g++ make
export PATH=$HOME/cmake-3.16.5/bin:$PATH

if [ "$BUILD_CONFIG" == "Debug" ]; then
    echo "========> Install Qt5"
    apt-get install -y qt5-default libqt5websockets5 libqt5websockets5-dev
fi

echo "========> Install PostgreSQL"
apt-get install -y postgresql-9.6 libpq-dev postgresql-server-dev-all

echo "========> Install Sqlite"
apt-get install -y sqlite3 sqlite libsqlite3-dev

echo "========> Install node"
curl -sL https://deb.nodesource.com/setup_9.x | bash -
apt-get install -y nodejs

echo "========> Install yarn"
curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | apt-key add -
echo "deb https://dl.yarnpkg.com/debian/ stable main" | tee /etc/apt/sources.list.d/yarn.list
apt-get update && apt-get install -y yarn

