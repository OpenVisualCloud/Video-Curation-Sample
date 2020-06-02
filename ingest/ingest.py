#!/usr/bin/python3

from kafka import KafkaConsumer
from subprocess import call
from zkstate import ZKState
import traceback
import socket
import time
import os

topic="video_curation_sched"
groupid="curaters"
clientid=socket.gethostname()

kkhost=os.environ["KKHOST"]
vdhost=os.environ["VDHOST"]
dbhost=os.environ["DBHOST"]

while True:
    try:
        c=KafkaConsumer(topic,bootstrap_servers=kkhost,
               client_id=clientid, group_id=groupid, auto_offset_reset="earliest",
               api_version=(0,10))

        for msg in c:
            mode,clip_name=msg.value.decode('utf-8').split(",")
            zk=ZKState("/state/"+clip_name,mode)
            if not zk.processed():
                if zk.process_start():

                    print("Processing "+clip_name+":"+mode+"...", flush=True)
                    while True:
                       print("Downloading "+clip_name, flush=True)
                       sts=call(["/usr/bin/wget","-O",clip_name,vdhost+"/mp4/"+clip_name])
                       if sts==0: break
                       time.sleep(1)

                    call(["/opt/gstreamer_gva/metaData_extract","-i",clip_name,"-n","-x",mode,"-a",dbhost,"-l"])
                    os.remove(clip_name)
                    zk.process_end()
            zk.close()

    except:
        print(traceback.format_exc(), flush=True)
