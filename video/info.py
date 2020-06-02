#!/usr/bin/python3

from tornado import web, gen
from tornado.concurrent import run_on_executor
from concurrent.futures import ThreadPoolExecutor
from urllib.parse import unquote
from subprocess import Popen, PIPE, STDOUT
import os

class InfoHandler(web.RequestHandler):
    def __init__(self, app, request, **kwargs):
        super(InfoHandler, self).__init__(app, request, **kwargs)
        self.executor= ThreadPoolExecutor(2)
        self._mp4path="/var/www/mp4"

    def check_origin(self, origin):
        return True

    @run_on_executor
    def _get_info(self, video):
        width=height=duration=fps=0
        cmd=["/usr/local/bin/ffprobe","-v","error","-show_streams","-i",self._mp4path+"/"+video]
        with Popen(cmd,stdout=PIPE,stderr=STDOUT,bufsize=1,universal_newlines=True) as p:
            for line in p.stdout:
                line=line.strip()
                if line.startswith("width="): width=int(line.split("=")[-1])
                if line.startswith("height="): height=int(line.split("=")[-1])
                if line.startswith("duration="):
                    duration=max(duration, float(line.split("=")[-1]))
                if line.startswith("avg_frame_rate=") and line!="avg_frame_rate=0/0":
                    eq=line.split("=")[-1].split("/")
                    fps=float(eq[0])/float(eq[1])
            p.stdout.close()
            p.wait()
        return { "width": width, "height": height, "duration": duration, "fps": fps }

    @gen.coroutine
    def get(self):
        video=unquote(str(self.get_argument("video")))
        info=yield self._get_info(video)
        self.write(info)
        self.set_status(200,'OK')
