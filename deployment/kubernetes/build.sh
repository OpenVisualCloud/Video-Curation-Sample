#!/bin/bash -e

DIR=$(dirname $(readlink -f "$0"))
NCURATIONS=${2:-1}
INGESTION="$3"
REGISTRY="$4"
HOSTIP=$(ip route get 8.8.8.8 | awk '/ src /{split(substr($0,index($0," src ")),f);print f[2];exit}')

find "${DIR}" -maxdepth 1 -mindepth 1 -name "*.yaml" -exec rm -rf "{}" \;
for template in $(find "${DIR}" -maxdepth 1 -mindepth 1 -name "*.yaml.m4" -print); do
    yaml=${template/.m4/}
    m4 -DREGISTRY_PREFIX=$REGISTRY -DINGESTION="$INGESTION" -DNCURATIONS=$NCURATIONS -DHOSTIP=$HOSTIP -DUSERID=$(id -u) -DGROUPID=$(id -g) -I "${DIR}" "${template}" > "${yaml}"
done
