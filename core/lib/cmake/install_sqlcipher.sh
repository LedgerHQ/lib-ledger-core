#!/usr/bin/env bash
mkdir -p ../../include/sqlcipher || echo "SQLCipher: include directory exists"
mkdir -p ../../lib || echo "SQLCipher: lib directory exists"
cp sqlite3.h ../../include/sqlcipher
cp sqlite3ext.h ../../include/sqlcipher
cp .libs/libsqlcipher.a ../../lib