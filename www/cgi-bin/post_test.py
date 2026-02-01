#!/usr/bin/env python3
import sys
import os

print("Content-Type: text/html\r\n\r\n")
print("<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><title>POST Data Echo</title>")
print("<link rel='stylesheet' href='../style.css'></head><body>")
print("<div class='container'>")
print("<header><h1>POST Request Echo</h1><p class='subtitle'>Received POST data</p></header>")
print("<nav><a href='../index.html'>Dashboard</a><a href='../cgi.html'>CGI Tests</a></nav>")

# Read POST body
content_length = int(os.environ.get("CONTENT_LENGTH", 0))
post_body = sys.stdin.read(content_length) if content_length > 0 else ""

print("<div class='card'>")
print("<h2>Received Data</h2>")
if not post_body:
    print("<p style='color: #e74c3c;'>No body data received!</p>")
else:
    print("<p>The server received the following data in the POST body:</p>")
    print(f"<pre>{post_body}</pre>")
print("</div>")

print("<div class='card'>")
print("<h3>Request Headers</h3>")
print("<ul>")
print(f"<li><strong>Content-Length:</strong> {content_length}</li>")
print(f"<li><strong>Content-Type:</strong> {os.environ.get('CONTENT_TYPE', 'N/A')}</li>")
print("</ul>")
print("</div>")

print("<footer><p>WebServ Project &copy; 2026</p></footer>")
print("</div></body></html>")
