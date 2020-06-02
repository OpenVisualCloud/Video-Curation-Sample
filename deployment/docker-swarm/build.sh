#!/bin/bash -e

DIR=$(dirname $(readlink -f "$0"))
NCURATIONS=${2:-1}
INGESTION="$3"
REGISTRY="$4"

if test -f "${DIR}/docker-compose.yml.m4"; then
    echo "Generating docker-compose.yml"
    m4 -DREGISTRY_PREFIX=$REGISTRY -DINGESTION="$INGESTION" -DNCURATIONS=${NCURATIONS} -I "${DIR}" "${DIR}/docker-compose.yml.m4" > "${DIR}/docker-compose.yml"
fi
