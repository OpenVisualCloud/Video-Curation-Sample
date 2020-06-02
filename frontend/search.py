#!/usr/bin/python3

from urllib.parse import unquote,quote
from tornado import web,gen
from tornado.concurrent import run_on_executor
from concurrent.futures import ThreadPoolExecutor
from merge_iv import merge_iv
from requests import get
import os
import json
import vdms
import time

dbhost=os.environ["DBHOST"]
vdhost=os.environ["VDHOST"]

class SearchHandler(web.RequestHandler):
    def __init__(self, app, request, **kwargs):
        super(SearchHandler, self).__init__(app, request, **kwargs)
        self.executor= ThreadPoolExecutor(8)
        self._vdms=vdms.vdms()
        while True:
            try:
                self._vdms.connect(dbhost)
                break
            except Exception as e:
                print("Exception: "+str(e), flush=True)
            time.sleep(10)

    def check_origin(self, origin):
        return True

    def _value(self, query1, key):
        for kv in query1["params"]:
            if kv["name"]==key: return kv["value"]
        return None
        
    def _construct_queries(self, queries, ref):
        q_bbox={
            "FindBoundingBox": {
                "_ref": ref,
                "results": {
                    "list": [ "frameID", "objectID", "_coordinates" ],
                },
            },
        }
        q_conn={
            "FindConnection": {
                "class": "Frame2BB",
                "ref2": ref,
                "results": {
                    "list": [ "frameID", "video_name" ],
                }, 
            },
        }

        for query1 in queries:
            if query1["name"]=="video":
                name=self._value(query1, "Video Name")
                if name!="*" and name!="":
                    q_conn["FindConnection"].update({
                        "constraints": {
                            "video_name": [ "==", name ],
                        },
                    })
            if query1["name"]=="object":
                q_bbox["FindBoundingBox"].update({
                    "constraints": {
                        "objectID": [ "==", self._value(query1, "Object List") ],
                    },
                })
            if query1["name"]=="person":
                constraints={
                    "age": [ 
                        ">=", self._value(query1, "Age Min"),
                        "<=", self._value(query1, "Age Max"),
                    ],
                    "objectID": [ 
                        "==", "face" 
                    ],
                }
                if "age" not in q_bbox["FindBoundingBox"]["results"]["list"]:
                    q_bbox["FindBoundingBox"]["results"]["list"].append("age")

                emotion=self._value(query1, "Emotion List")
                if emotion!="skip": 
                    constraints["emotion"]=[ "==", emotion ]
                    if "emotion" not in q_bbox["FindBoundingBox"]["results"]["list"]:
                        q_bbox["FindBoundingBox"]["results"]["list"].append("emotion")

                gender=self._value(query1, "Gender")
                if gender!="skip": 
                    constraints["gender"]=[ "==", gender ]
                    if "gender" not in q_bbox["FindBoundingBox"]["results"]["list"]:
                        q_bbox["FindBoundingBox"]["results"]["list"].append("gender")

                q_bbox["FindBoundingBox"].update({ "constraints": constraints })

        return [q_bbox, q_conn]

    def _decode_response(self, response):
        clips={}
        for i in range(0,len(response)-1,2):
            if response[i+1]["FindConnection"]["status"]==0 and response[i]["FindBoundingBox"]["status"]==0:
                connections=response[i+1]["FindConnection"]["connections"]
                bboxes=response[i]["FindBoundingBox"]["entities"]
                if len(connections)!=len(bboxes): continue

                for j in range(0,len(connections)):
                    stream=connections[j]["video_name"]
                    if stream not in clips:
                        r=get(vdhost+"/api/info",params={"video":stream}).json()
                        clips[stream]={
                            "fps": r["fps"],
                            "duration": r["duration"],
                            "width": r["width"],
                            "height": r["height"],
                            "segs": [],
                            "frames": {},
                        }

                    # time stamp and duration
                    stream1=clips[stream]
                    ts=float(bboxes[j]["frameID"])/stream1["fps"]

                    # merge segs
                    segmin=1
                    seg1=[max(ts-segmin,0),min(ts+segmin,stream1["duration"])]
                    stream1["segs"]=merge_iv(stream1["segs"], seg1)

                    if ts not in stream1["frames"]: 
                        stream1["frames"][ts]={ 
                            "time": ts, 
                            "objects": [] 
                        }

                    if "objectID" in bboxes[j]:
                        bbc=bboxes[j]["_coordinates"]
                        stream1["frames"][ts]["objects"].append({
                            "detection" : {
                                "bounding_box" : {
                                    "x_max" : float(bbc["w"]+bbc["x"])/float(stream1["width"]),
                                    "x_min" : float(bbc["x"])/float(stream1["width"]),
                                    "y_max" : float(bbc["h"]+bbc["y"])/float(stream1["height"]),
                                    "y_min" : float(bbc["y"])/float(stream1["height"]),
                                },
                                "confidence" : 0.99,
                                "label" : bboxes[j]["objectID"],
                            },
                        })
        print("clips:", flush=True)
        print(clips, flush=True)

        # create segments
        segs=[]
        for name in clips:
            stream1=clips[name]
            for seg1 in stream1["segs"]:
                seg1c={
                    "name": name,
                    "stream": quote("/api/segment/"+str(seg1[0])+"/"+str(seg1[1])+"/"+name),
                    "thumbnail": quote("/api/thumbnail/"+str(seg1[0])+"/"+name+".png"),
                    "fps": stream1["fps"],
                    "time": seg1[0],
                    "duration": seg1[1]-seg1[0],
                    "offset": 0,
                    "width": stream1["width"],
                    "height": stream1["height"],
                    "frames": [],
                }
                for ts in stream1["frames"]:
                    if ts>=seg1[0] and ts<=seg1[1]:
                        stream1["frames"][ts].update({ "time": (ts-seg1[0])*1000 })
                        seg1c["frames"].append(stream1["frames"][ts])
                segs.append(seg1c)

        print("segs:", flush=True)
        print(segs, flush=True)
        return segs

    @run_on_executor
    def _search(self, queries, size):
        vdms_queries=[]
        for query1 in queries:
            vdms_queries.extend(self._construct_queries(query1, len(vdms_queries)+1))

        print(vdms_queries, flush=True)
        vdms_response, vdms_array =self._vdms.query(vdms_queries)
        print(vdms_response, flush=True)

        return self._decode_response(vdms_response)
        
        try:
            return [{
                "thumbnail": "images/segment.svg",
                "stream": "video/mock.mp4",
                "time": 0.01, # segment time
                "offset": 0, # segment offset time, usually zero
                "duration": 5.0,
                "fps": 30,
                "width": 1024,
                "height": 1024,
                "frames": [{
                    "time": 0.05, # seconds
                    "objects": [{
                        "detection" : {
                            "bounding_box" : {
                                "x_max" : 0.37810274958610535,
                                "x_min" : 0.2963270843029022,
                                "y_max" : 0.8861181139945984,
                                "y_min" : 0.784187376499176
                            },
                            "confidence" : 0.9999198913574219,
                            "label" : "vehicle",
                            "label_id" : 2
                        }
                    }],
                },{
                    "time": 0.06, # seconds
                    "objects": [{
                        "detection" : {
                            "bounding_box" : {
                                "x_max" : 0.37810274958610535,
                                "x_min" : 0.2963270843029022,
                                "y_max" : 0.8861181139945984,
                                "y_min" : 0.784187376499176
                            },
                            "confidence" : 0.9999198913574219,
                            "label" : "vehicle",
                            "label_id" : 2
                        }
                    }],
                }],
                "properties": [],  # additional name/value pairs to show in table
            }]
        except Exception as e:
            return str(e)

    @gen.coroutine
    def get(self):
        queries=json.loads(unquote(str(self.get_argument("queries"))))
        size=int(self.get_argument("size"))
        print("queries",flush=True)
        print(queries,flush=True)

        r=yield self._search(queries, size)
        if isinstance(r, str):
            self.set_status(400, str(r))
            return

        self.write({"response":r})
        self.set_status(200, 'OK')
        self.finish()
