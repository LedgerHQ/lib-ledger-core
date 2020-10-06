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

echo "========> Install PostgreSQL"
brew install postgresql
export CPLUS_INCLUDE_PATH="/usr/local/Cellar/postgresql/12.3_4/include:$CPLUS_INCLUDE_PATH"


echo "========> Install Sqlite"
brew install sqlite

echo "========> Install OpenSSL"
brew install openssl@1.1


