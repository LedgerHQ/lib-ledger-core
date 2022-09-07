#!/usr/bin/env bash
set -euo pipefail
cd $GITHUB_WORKSPACE/ledger-wallet-daemon
sed -i -E 's/(val libcore *= *\")(.*)\"/\1'$JAR_VERSION'\"/' build.sbt

# Bump libcore version

# Commit & push changes
git checkout -b bump_libcore_$LIB_VERSION
git add lib # libcore
git config --global user.name "${GITHUB_ACTOR}"
git config --global user.email "${GITHUB_ACTOR}@users.noreply.github.com"
git commit -m "Bump libcore to $LIB_VERSION"
git push --set-upstream origin bump_libcore_$LIB_VERSION
gh pr create --title "Bump libcore to $LIB_VERSION" \
   --body "Automatic update from libcore release" \
   --base main \
   --head bump_libcore_$LIB_VERSION