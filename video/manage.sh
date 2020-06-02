#!/bin/bash -e

# ingest video list into Kafka
cd /var/www/mp4
ls -1 | awk -v ingest=$INGESTION 'BEGIN{split(ingest,modes,",")}{for (i in modes) print modes[i]","$0}' > /tmp/videolist.txt

while true; do
    sout="$(cat /tmp/videolist.txt | kafkacat -P -D '\n' -b $KKHOST -t video_curation_sched -p -1 -T -X partition.assignment.strategy=roundrobin 2>&1 || echo)"
    case "$sout" in
    *ERROR*)
        echo "$sout"
        sleep 1s
        continue;;
    esac
    break
done

# run tornado
exec /home/manage.py
