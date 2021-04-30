#!/bin/bash -ex

cd $(dirname $0)/src
exec make all_builds -j $(nproc)
