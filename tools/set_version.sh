#!/bin/bash


#
# Function that parses semantic versioning striings of
# the form:
#   MAJOR.MINOR.PATCH([+-].*)?
#
# Source: https://gist.github.com/jlinoff/9211310b738e83c4a2bbe14876cd2e55
#
# Parse the major, minor and patch versions
# out.
# You use it like this:
#    semver="3.4.5+xyz"
#    a=($(parse_semver "$semver"))
#    major=${a[0]}
#    minor=${a[1]}
#    patch=${a[2]}
#    printf "%-32s %4d %4d %4d\n" "$semver" $major $minor $patch
function parse_semver() {
    local token="$1"
    local major=0
    local minor=0
    local patch=0

    if egrep '^[0-9]+\.[0-9]+\.[0-9]+' <<<"$token" >/dev/null 2>&1 ; then
        # It has the correct syntax.
        local n=${token//[!0-9]/ }
        local a=(${n//\./ })
        major=${a[0]}
        minor=${a[1]}
        patch=${a[2]}
    fi

    echo "$major $minor $patch"
}

version=($(parse_semver $1))
major=${version[0]}
minor=${version[1]}
patch=${version[2]}

echo "major" $major
echo "minor" $minor
echo "patch" $patch

sed -i -E 's/(VERSION_MAJOR *)([0-9]+)/\1'$major'/' CMakeLists.txt
sed -i -E 's/(VERSION_MINOR *)([0-9]+)/\1'$minor'/' CMakeLists.txt
sed -i -E 's/(VERSION_PATCH *)([0-9]+)/\1'$patch'/' CMakeLists.txt