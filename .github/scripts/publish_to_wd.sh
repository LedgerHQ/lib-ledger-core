#!/usr/bin/env bash
set -euo pipefail

cd $GITHUB_WORKSPACE/ledger-wallet-daemon

# Bump libcore version
sed -i -E 's/(val libcore *= *\")(.*)\"/\1'$VERSION'\"/' build.sbt

# Commit & push changes
git checkout -b bump_libcore_$VERSION
git add build.sbt
git config --global user.name "${GITHUB_ACTOR}"
git config --global user.email "${GITHUB_ACTOR}@users.noreply.github.com"
git commit -m "Bump libcore to $VERSION"
git push --set-upstream origin bump_libcore_$VERSION
gh pr create --title "Bump libcore to $VERSION" \
   --body "Automatic update from libcore release" \
   --base main \
   --head bump_libcore_$VERSION