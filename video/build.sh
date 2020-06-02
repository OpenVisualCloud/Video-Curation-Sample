#!/bin/bash -e

IMAGE="lcc_video"
DIR=$(dirname $(readlink -f "$0"))

"$DIR/download.sh"
. "$DIR/../script/build.sh"
