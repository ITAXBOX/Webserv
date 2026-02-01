#!/usr/bin/env python3
import os

print("Content-Type: text/html\r\n\r\n")
print("<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><title>Environment Variables</title>")
print("<link rel='stylesheet' href='../style.css'></head><body>")
print("<div class='container'>")
print("<header><h1>Environment Variables</h1><p class='subtitle'>Server execution context</p></header>")
print("<nav><a href='../index.html'>Dashboard</a><a href='../cgi.html'>CGI Tests</a></nav>")
print("<div class='card'>")
print("<pre style='white-space: pre-wrap; word-break: break-all;'>")
for key, value in sorted(os.environ.items()):
    print(f"<strong>{key}</strong>: {value}")
print("</pre>")
print("</div>")
print("<footer><p>WebServ Project &copy; 2026</p></footer>")
print("</div></body></html>")
