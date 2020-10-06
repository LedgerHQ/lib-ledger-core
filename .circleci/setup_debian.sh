#!/usr/bin/env bash

BUILD_CONFIG=$1

apt-get update
apt-get -y install git ssh

echo "========> Install basic config"
apt-get update
apt-get install -y apt-transport-https wget python build-essential libx11-xcb-dev
apt-get install -y libssl-dev curl tcl

echo "========> Install Java"
#apt-get install openjdk-8-jdk -y
apt-get install default-jdk -y
echo "========> Java Installed"
echo $JAVA_HOME
ls -la /usr/lib/jvm/java-8-openjdk || echo "!!!! java openjdk not found"

apt-get install -y awscli || apt --fix-broken install -y
echo "========> Install sbt"
SBT_DEB="sbt-1.3.10.deb"
curl -L -o $SBT_DEB https://dl.bintray.com/sbt/debian/$SBT_DEB
dpkg -i $SBT_DEB
rm $SBT_DEB
sbt sbtVersion
apt-get install -y scala

echo "========> Install C++ dependencies"
apt-get install -y g++ make
export PATH=$HOME/cmake-3.16.5/bin:$PATH

echo "========> Install Qt5"
apt-get install -y qt5-default libqt5websockets5 libqt5websockets5-dev

echo "========> Install PostgreSQL"
apt-get install -y postgresql-11 libpq-dev postgresql-server-dev-all || apt --fix-broken install -y

echo "========> Install Sqlite"
apt-get install -y sqlite3 sqlite libsqlite3-dev

echo "========> Install kerberos"
apt-get install -y libkrb5-dev



