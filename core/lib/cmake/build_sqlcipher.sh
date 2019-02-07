#!/usr/bin/env bash
#This is used for Linux build
#make command is failing because it can't reference
#some symbols on libcrypto.a (which are contained in it)
make sqlite3.h
make sqlite3ext.h
make libsqlcipher.la

