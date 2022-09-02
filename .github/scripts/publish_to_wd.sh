cp -v ledger-lib-core.jar $GITHUB_WORKSPACE/ledger-wallet-daemon/lib/ledger-lib-core.jar
cd $GITHUB_WORKSPACE/ledger-wallet-daemon/libcore/lib-ledger-core
git checkout ${{ github.ref }}
cd ../..
git checkout -b bump_libcore_$VERSION
git add lib libcore
git config --global user.name "${GITHUB_ACTOR}"
git config --global user.email "${GITHUB_ACTOR}@users.noreply.github.com"
git commit -m "Bump libcore to $VERSION"
git push --set-upstream origin bump_libcore_$VERSION
gh pr create --title "Bump libcore to $VERSION" \
   --body "Automatic update from libcore release" \
   --base main \
   --head bump_libcore_$VERSION