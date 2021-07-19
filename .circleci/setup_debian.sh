#!/usr/bin/env bash

BUILD_CONFIG=$1

apt-get update
apt-get -y install git ssh

echo "========> Install basic config"
apt-get update
apt-get install -y apt-transport-https wget python build-essential libx11-xcb-dev
apt-get install -y libssl-dev curl tcl

echo "========> Install Java"
apt-get install openjdk-8-jdk -y
echo "========> Java Installed"
echo $JAVA_HOME
ls -la /usr/lib/jvm/java-8-openjdk || echo "!!!! java openjdk not found"

apt-get install -y awscli
echo "========> Install sbt"
echo "deb https://repo.scala-sbt.org/scalasbt/debian all main" | tee /etc/apt/sources.list.d/sbt.list
echo "deb https://repo.scala-sbt.org/scalasbt/debian /" | tee /etc/apt/sources.list.d/sbt_old.list
curl -sL "https://keyserver.ubuntu.com/pks/lookup?op=get&search=0x2EE0EA64E40A89B84B2DF73499E82A75642AC823" | apt-key add
apt-get update
apt-get install -y sbt scala
sbt sbtVersion

echo "========> Install C++ dependencies"
apt-get install -y g++ make
export PATH=$HOME/cmake-3.16.5/bin:$PATH

echo "========> Install Qt5"
apt-get install -y qt5-default libqt5websockets5 libqt5websockets5-dev

echo "========> Install PostgreSQL"
apt-get install -y postgresql-9.6 libpq-dev postgresql-server-dev-all

echo "========> Install Sqlite"
apt-get install -y sqlite3 sqlite libsqlite3-dev

echo "========> Install kerberos"
apt-get install -y libkrb5-dev



