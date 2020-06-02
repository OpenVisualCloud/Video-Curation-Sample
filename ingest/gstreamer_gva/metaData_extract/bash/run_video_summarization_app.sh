#!/bin/bash

VIDEO_FILE_LIST=$1
METADATA_X_TYPE=$2
QUERY_TYPE=$3

rm -f proc_log.txt
for VID_FN in `sed '/^$/d' $VIDEO_FILE_LIST`; do
    echo " "
    echo "Processing $VID_FN"
    echo " "
    ../build/debug/metaData_extract -i $VID_FN -m $METADATA_TYPE -n -u
    echo $VID_FN >> proc_log.txt
done

for VID_FN in `sed '/^$/d' ./proc_log.txt`; do
    echo " "
    echo "Summarizing $VID_FN"
    echo " "
    ../../client/build/debug/client -i $VID_FN -m $QUERY_TYPE
done
