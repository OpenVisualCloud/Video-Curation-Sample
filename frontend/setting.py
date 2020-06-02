#!/usr/bin/python3

from tornado import web, gen
from tornado.concurrent import run_on_executor
from concurrent.futures import ThreadPoolExecutor

class SettingHandler(web.RequestHandler):
    def __init__(self, app, request, **kwargs):
        super(SettingHandler, self).__init__(app, request, **kwargs)
        self.executor= ThreadPoolExecutor(2)

    def check_origin(self, origin):
        return True

    @run_on_executor
    def _settings(self):
        return {
            "controls": [{
                "name": "person",
                "icon": "images/person.png",
                "description": "Find Person",
                "params": [{
                    "name": "Age Min",
                    "type": "number",
                    "value": 0,
                },{
                    "name": "Age Max",
                    "type": "number",
                    "value": 100,
                },{
                    "name": "Gender",
                    "type": "list",
                    "values": [
                       "skip",
                       "male",
                       "female",
                    ],
                    "value": "skip",
                },{
                    "name": "Emotion List",
                    "type": "list",
                    "values": [ 
                        "skip",
                        "neutral", 
                        "happy", 
                        "sad", 
                        "surprise", 
                        "anger" 
                    ],
                    "value": "skip",
                }],
            },{
                "name": "object",
                "icon": "images/object.png",
                "description": "Find Object",
                "params": [{
                    "name": "Object List",
                    "type": "list",
                    "values": [
                        "person", "bicycle", "car", "motorbike","aeroplane",
                        "bus","train", "truck", "boat", "traffic light", 
                        "fire hydrant", "stop sign", "parking meter", "bench", 
                        "bird", "cat", "dog", "horse", "sheep", "cow", 
                        "elephant", "bear", "zebra", "giraffe", "backpack", 
                        "umbrella", "handbag", "tie", "suitcase", "frisbee", 
                        "skis", "snowboard", "sports ball", "kite", 
                        "baseball bat", "baseball glove", "skateboard", 
                        "surfboard", "tennis racket", "bottle", "wine glass",
                        "cup", "fork", "knife", "spoon", "bowl", "banana",
                        "apple", "sandwich", "orange", "broccoli", "carrot",
                        "hot dog", "pizza", "donut", "cake", "chair", "sofa",
                        "pottedplant", "bed", "diningtable", "toilet", 
                        "tvmonitor", "laptop", "mouse", "remote", "keyboard",
                        "cell phone", "microwave", "oven", "toaster", "sink",
                        "refrigerator", "book", "clock", "vase", "scissors",
                        "teddy bear", "hair drier", "toothbrush"
                    ],
                    "value": "person",
                }],
            },{
                "name": "video",
                "icon": "images/video.png",
                "description": "Find Video",
                "params": [{
                    "name": "Video Name",
                    "type": "text",
                    "value": "*",
                }],
#            },{
#                "name": "advanced",
#                "icon": "images/advanced.png",
#                "description": "Advanced",
#                "params": [{
#                    "name": "Search Queries",
#                    "type": "text",
#                    "value": "",
#                }],
            }],
        }

    @gen.coroutine
    def get(self):
        settings=yield self._settings()
        self.write(settings)
        self.set_status(200,'OK')

