#!/usr/bin/python3

from tornado import web, gen
from tornado.concurrent import run_on_executor
from concurrent.futures import ThreadPoolExecutor
from urllib.parse import unquote
from subprocess import call
import os

class ThumbnailHandler(web.RequestHandler):
    def __init__(self, app, request, **kwargs):
        super(ThumbnailHandler, self).__init__(app, request, **kwargs)
        self.executor= ThreadPoolExecutor(2)
        self._mp4path="/var/www/mp4"
        self._genpath="/var/www/gen"

    def check_origin(self, origin):
        return True

    @run_on_executor
    def _gen_thumbnail(self, video, start):
        output=video.replace(".mp4","-"+start+".png")
        if not os.path.exists(self._genpath+"/"+output):
            call(["/usr/local/bin/ffmpeg","-ss",start,"-i",self._mp4path+"/"+video,"-vf","thumbnail,scale=640:360","-frames:v","1","-y",self._genpath+"/"+output])
        return output

    def _format(self, time):
        hours=int(time/3600)
        mins=int((time%3600)/60)
        seconds=int(time%60)
        macroseconds=int((time*1000)%1000)
        return str(hours)+":"+str(mins)+":"+str(seconds)+"."+str(macroseconds)

    @gen.coroutine
    def get(self):
        mm=unquote(str(self.request.path)).split("/")
        video=mm[-1].replace(".png","")
        start=self._format(float(mm[-2]))
        thumbnail=yield self._gen_thumbnail(video, start)
        self.add_header('X-Accel-Redirect','/gen/'+thumbnail)
        self.set_status(200,'OK')
