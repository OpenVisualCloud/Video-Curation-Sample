#!/usr/bin/python3

from tornado import ioloop, web
from tornado.options import define, options, parse_command_line
from search import SearchHandler
from workload import WorkloadHandler
from setting import SettingHandler
from subprocess import Popen
from signal import signal, SIGTERM, SIGQUIT
import os
import json

tornadoc=None

def quit_service(signum, frame):
    if tornadoc: tornadoc.add_callback(tornadoc.stop)
        
app = web.Application([
    (r'/api/search',SearchHandler),
    (r'/api/workload',WorkloadHandler),
    (r'/api/setting',SettingHandler),
])

if __name__ == "__main__":
    signal(SIGTERM, quit_service)

    define("port", default=2222, help="the binding port", type=int)
    define("ip", default="127.0.0.1", help="the binding ip")
    parse_command_line()
    print("Listening to " + options.ip + ":" + str(options.port))
    app.listen(options.port, address=options.ip)

    tornadoc=ioloop.IOLoop.instance();
    tornadoc.start()
