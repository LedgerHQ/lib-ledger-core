#!/usr/bin/env bash
#This is a workaround
#if we use directly cmake used by gcc, we get executables
#built for iOS which of course fail tu be executed in bash
#Workaround: install another gcc with brew and use it
export HOMEBREW_NO_AUTO_UPDATE=1
brew install gcc || echo "gcc already installed with brew"
PREFIX_GCC=`brew --prefix gcc`
BREW_GCC=`ls $PREFIX_GCC/bin | grep "^gcc[-][0-9]"`

make sqlite3.h BCC=$PREFIX_GCC/bin/$BREW_GCC
otool -l mksourceid | grep VERSION
make sqlite3ext.h BCC="${PREFIX_GCC}/bin/${BREW_GCC} -fpermissive"
otool -l mkkeywordhash | grep VERSION
otool -l lemon | grep VERSION
make libsqlcipher.la BCC=$PREFIX_GCC/bin/$BREW_GCC