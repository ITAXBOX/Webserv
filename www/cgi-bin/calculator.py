#!/usr/bin/env python3
import sys
import os

print("Content-Type: text/html\r\n\r\n")
print("<!DOCTYPE html><html lang='en'><head><meta charset='UTF-8'><title>CGI Calculator</title>")
print("<link rel='stylesheet' href='../style.css'>")
print("<style>.calc-form input { padding: 10px; margin: 5px; width: 80px; } .result { font-size: 2em; color: var(--primary-color); font-weight: bold; }</style>")
print("</head><body>")
print("<div class='container'>")
print("<header><h1>CGI Calculator</h1><p class='subtitle'>Server-side arithmetic via POST</p></header>")
print("<nav><a href='../index.html'>Dashboard</a><a href='../cgi.html'>CGI Tests</a></nav>")

# Helper to read POST body
def get_post_data():
    content_length = int(os.environ.get("CONTENT_LENGTH", 0))
    if content_length > 0:
        return sys.stdin.read(content_length)
    return ""

def parse_qs(qs):
    # Simple query string parser (e.g. "a=1&b=2&op=add")
    params = {}
    for pair in qs.split('&'):
        if '=' in pair:
            key, val = pair.split('=', 1)
            params[key] = val
    return params

data = get_post_data()
message = "Enter numbers to calculate"
result = None

if data:
    params = parse_qs(data)
    try:
        num1 = float(params.get('num1', 0))
        num2 = float(params.get('num2', 0))
        op = params.get('op', 'add')
        
        if op == 'add':
            result = f"{num1} + {num2} = {num1 + num2}"
        elif op == 'sub':
            result = f"{num1} - {num2} = {num1 - num2}"
        elif op == 'mul':
            result = f"{num1} * {num2} = {num1 * num2}"
        elif op == 'div':
            if num2 == 0:
                result = "Error: Division by Zero"
            else:
                result = f"{num1} / {num2} = {num1 / num2}"
    except ValueError:
        message = "Invalid input data"

print("<div class='card' style='text-align: center;'>")
if result:
    print(f"<div class='result'>{result}</div>")
else:
    print(f"<p>{message}</p>")

print("<form action='calculator.py' method='POST' class='calc-form' style='margin-top: 20px;'>")
print("<input type='number' step='any' name='num1' placeholder='Number 1' required>")
print("<select name='op' style='padding: 10px;'>")
print("<option value='add'>+</option><option value='sub'>-</option><option value='mul'>*</option><option value='div'>/</option>")
print("</select>")
print("<input type='number' step='any' name='num2' placeholder='Number 2' required>")
print("<br><button type='submit' class='btn'>Calculate</button>")
print("</form>")
print("</div>")

print("<footer><p>WebServ Project &copy; 2026</p></footer>")
print("</div></body></html>")
