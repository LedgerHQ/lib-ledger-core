#!/usr/bin/env bash
mkdir -p ../../include/sqlcipher
cp sqlite3.h ../../include/sqlcipher
cp sqlite3ext.h ../../include/sqlcipher
cp .libs/libsqlcipher.a ../../lib