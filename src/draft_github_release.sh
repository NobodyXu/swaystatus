#!/bin/sh -ex

get_libc_name() {
    libc=$(ldd release_build/swaystatus | grep libc | cut -d '>' -f 2 | sed 's/(.*)$//')
    output=$($libc 2>&1)
    if echo "$output" | grep -q 'GNU C Library'; then
        echo glibc
    elif echo "$output" | grep -q 'musl libc'; then
        echo musl
    else
        echo unknown libc! >&2
        exit 1
    fi
}

label_postfix="$(arch)-$(uname)-$(get_libc_name)"

if [ $# -lt 1 ] || [ $# -gt 2 ]; then
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

cd $(dirname $0)

./run_all_builds.sh

cp release_build/swaystatus "/tmp/swaystatus-with-python-support-$label_postfix"
cp release_no_py_build/swaystatus "/tmp/swaystatus-no-python-support-$label_postfix"

git push
exec gh release create "$tag" \
    --prerelease \
    $changelogOption "$changelogValue" \
    -t "swaystatus $tag" \
    "/tmp/swaystatus-with-python-support-$label_postfix" \
    "/tmp/swaystatus-no-python-support-$label_postfix"
