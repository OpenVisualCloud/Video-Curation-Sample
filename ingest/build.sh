#!/bin/bash -e

IMAGE="lcc_ingest"
DIR=$(dirname $(readlink -f "$0"))

# merge yolov3.bin
yolodir="${DIR}/openvino_models/2019R3_models/yolov3_openvino"
if [ ! -f "$yolodir/yolo_v3.bin" ]; then
    cat "$yolodir"/yolo_v3.bin.* > "$yolodir/yolo_v3.bin"
fi

. "$DIR/../script/build.sh" 
