#!/usr/bin/env bash
echo "========> Install basic config"
brew update
brew install wget
brew install awscli
echo "========> Install C++ dependencies"
brew install cmake
echo "========> Install Qt5"
brew install qt5
export PATH="/usr/local/Cellar/qt/5.10.*/bin:$PATH"
echo "========> Install Sqlite"
brew install sqlite



