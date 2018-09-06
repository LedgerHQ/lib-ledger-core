#!/usr/bin/env bash

BUILD_CONFIG=$1

echo "========> Install basic config"
brew update
brew install wget

echo "========> Install Java"
brew cask install java
echo "========> Java Installed"
brew cask info java
echo "$(/usr/libexec/java_home -v 1.8)"
export JAVA_HOME="$(/usr/libexec/java_home -v 1.8)"



if [ "$BUILD_CONFIG" == "Release" ]; then
	brew install awscli
fi

echo "========> Install C++ dependencies"
brew install cmake

if [ "$BUILD_CONFIG" == "Debug" ]; then
	echo "========> Install Qt5"
	brew install qt5
	export PATH="/usr/local/Cellar/qt/5.*/bin:$PATH"
fi

echo "========> Install Sqlite"
brew install sqlite



