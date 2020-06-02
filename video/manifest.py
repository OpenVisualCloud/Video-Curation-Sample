#!/usr/bin/python3

from tornado import web, gen
from tornado.concurrent import run_on_executor
from concurrent.futures import ThreadPoolExecutor
from urllib.parse import unquote
import os

class ManifestHandler(web.RequestHandler):
    def __init__(self, app, request, **kwargs):
        super(ManifestHandler, self).__init__(app, request, **kwargs)
        self.executor= ThreadPoolExecutor(2)

    def check_origin(self, origin):
        return True

    @run_on_executor
    def _manifest(self):
        return [x for x in os.listdir("/var/www/mp4") if x.endswith(".mp4")]

    @gen.coroutine
    def get(self):
        manifest=yield self._manifest()
        self.write("\n".join(manifest)+"\n")
        self.set_status(200,'OK')
