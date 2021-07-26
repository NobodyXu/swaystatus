#!/bin/sh -ex

get_libc_name() {
    libc=$(ldd src/release_build/swaystatus | grep libc | cut -d '>' -f 2 | sed 's/(.*)$//')
    output=$($libc 2>&1)
    if echo "$output" | grep -q 'GNU C Library'; then
        version="$(echo $output | grep -o 'release version [0-9]*\.[0-9]*' | sed -e 's/release version //')"
        echo "glibc-$version"
    elif echo "$output" | grep -q 'musl libc'; then
        version="$(echo $output | grep -o 'Version [0-9]*\.[0-9]*\.[0-9]*' | sed -e 's/Version //')"
        echo "musl-$version"
    else
        echo unknown libc! >&2
        exit 1
    fi
}

cd $(dirname $0)

label_postfix="$(arch)-$(uname)-$(get_libc_name)"

if [ $# -lt 1 ] || [ $# -gt 2 ] || [ "$1" = "--help" ] || [ "$1" = "-h" ]; then
    echo "Usage: $0 <tag> [Release notes]" >&2
    exit 1
fi

tag=$1
if [ $# -eq 2 ]; then
    changelogOption="-n"
    changelogValue="$2"
else
    changelogFile=$(mktemp)
    "$EDITOR" "$changelogFile"

    changelogOption="-F"
    changelogValue="$changelogFile"
fi

./run_all_builds.sh

cp src/release_build/swaystatus "/tmp/swaystatus-with-python-support-$label_postfix"
cp src/release_no_py_build/swaystatus "/tmp/swaystatus-no-python-support-$label_postfix"

git push
exec gh release create "$tag" \
    --prerelease \
    $changelogOption "$changelogValue" \
    -t "swaystatus $tag" \
    "/tmp/swaystatus-with-python-support-$label_postfix" \
    "/tmp/swaystatus-no-python-support-$label_postfix"
