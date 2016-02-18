#!/usr/bin/python
# -*- coding: utf-8 -*-
#
# credit: http://sheep.art.pl/Wiki%20Engine%20in%20Python%20from%20Scratch


import http.server
import socketserver
import itertools
import logging
import platform
import os
import re
import subprocess
import tempfile
import urllib

logging.basicConfig(level=logging.DEBUG)

class Handler(http.server.SimpleHTTPRequestHandler):
    def do_HEAD(self):
        """Send response headers"""
        if self.path != "/":
            return http.server.SimpleHTTPRequestHandler.do_HEAD(self)
        self.send_response(200)
        self.send_header("content-type", "text/html;charset=utf-8")
        self.send_header('Access-Control-Allow-Origin', '*')
        self.end_headers()

    def do_GET(self):
        """Send page text"""
        if self.path != "/":
            return http.server.SimpleHTTPRequestHandler.do_GET(self)
        else:
            self.send_response(302)
            self.send_header("Location", os.getcwd())
            self.end_headers()

    def do_POST(self):
        """Save new page text and display it"""
        if self.path != "/":
            return http.server.SimpleHTTPRequestHandler.do_POST(self)

        length = self.headers.get('content-length')
        if length:
            text = self.rfile.read(int(length))
            f=open(os.path.join('s2aio\\blocklytemp', 'blocklytemp.ino'), "wb")
            f.write(text)
            f.close()
			
            rc = subprocess.Popen("arduino_web_server.bat", shell=True)

            if rc != 0:
                self.send_response(400)
            else:
                self.send_response(200)
                self.send_header('Access-Control-Allow-Origin', '*')
                self.end_headers()
        else:
            self.send_response(400)


if __name__ == '__main__':
    print ("Blockly@rduino:888")
    server = socketserver.TCPServer(("127.0.0.1", 888), Handler)
    server.pages = {}
    server.serve_forever()
