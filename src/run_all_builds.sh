#!/bin/bash -ex

exec make all_builds -j $(nproc)
