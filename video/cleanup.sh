#!/bin/bash -e

STORAGE_PATH="/var/www/gen"
while true; do
   find $STORAGE_PATH -mindepth 1 -mmin +$RETENTION_MINS -exec rm -f {} \; -print
   sleep $CLEANUP_INTERVAL
done
