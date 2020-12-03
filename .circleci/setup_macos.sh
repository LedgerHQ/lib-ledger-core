#!/usr/bin/env bash

BUILD_CONFIG=$1

echo ">>>>>>> GETTING CIRCLE_TAG : $CIRCLE_TAG"
echo ">>>>>>> GETTING CIRCLE_BRANCH : $CIRCLE_BRANCH"

echo "========> Install basic config"
brew update || echo "Brew failed, try continue build"

echo "========> Install wget"
brew install wget

echo "========> Install Java"

brew cask install adoptopenjdk/openjdk/adoptopenjdk8

echo "========> Java Installed"
echo "$(/usr/libexec/java_home -v 1.8)"
export JAVA_HOME="$(/usr/libexec/java_home -v 1.8)"



if [ "$BUILD_CONFIG" == "Release" ]; then
	brew install awscli
fi

echo "========> Install sbt"
brew install sbt

echo "========> Install C++ dependencies"
brew install cmake

echo "========> Install Qt5"
brew install qt5
export PATH="/usr/local/Cellar/qt/5.*/bin:$PATH"

echo "========> Install PostgreSQL"
# Install with verbose otherwise the setup may timeout the CI because it doesn't log.
brew install --verbose postgresql
export CPLUS_INCLUDE_PATH="/usr/local/Cellar/postgresql/12.3_4/include:$CPLUS_INCLUDE_PATH"


echo "========> Install Sqlite"
brew install sqlite

echo "========> Install OpenSSL"
brew install openssl


