#!/usr/bin/env bash

BUILD_CONFIG=$1

echo "========> Install basic config"
apt-get update
apt-get install -y apt-transport-https wget git python build-essential libx11-xcb-dev
apt-get install -y libssl-dev curl

if [ "$BUILD_CONFIG" == "Release" ]; then
	apt-get install -y awscli
fi

echo "========> Install C++ dependencies"
apt-get install -y g++ make
export PATH=$HOME/cmake-3.10.3/bin:$PATH

if [ "$BUILD_CONFIG" == "Debug" ]; then
	echo "========> Install Qt5"
	apt-get install -y qt5-default libqt5websockets5 libqt5websockets5-dev
fi

echo "========> Install Sqlite"
apt-get install -y sqlite3 sqlite libsqlite3-dev

echo "========> Install Boost"
apt-get install -y libboost-all-dev

#if [ "$BUILD_CONFIG" == "Debug" ]; then
	echo "========> Install node"
	curl -sL https://deb.nodesource.com/setup_9.x | bash -
	apt-get install -y nodejs
	echo "========> Install yarn"
	curl -sS https://dl.yarnpkg.com/debian/pubkey.gpg | apt-key add -
	echo "deb https://dl.yarnpkg.com/debian/ stable main" | tee /etc/apt/sources.list.d/yarn.list
	apt-get update && apt-get install -y yarn
#fi

