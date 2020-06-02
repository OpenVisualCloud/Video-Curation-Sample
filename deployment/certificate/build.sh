#!/bin/bash -e

IMAGE="lcc_certificate"
DIR=$(dirname $(readlink -f "$0"))

. "$DIR/../../script/build.sh"
