# WebServ Test Suite

## Test 1: Basic GET Request (Root)
```bash
curl -v http://localhost:8080/
```
**Expected:** 200 OK with index.html content (2170 bytes)

---

## Test 2: Specific File Request
```bash
curl -v http://localhost:8080/test.html
```
**Expected:** 200 OK with test.html content

---

## Test 3: CSS File Request (MIME Type Test)
```bash
curl -v http://localhost:8080/style.css
```
**Expected:** 200 OK with Content-Type: text/css

---

## Test 4: HEAD Request (No Body)
```bash
curl -I http://localhost:8080/
```
**Expected:** 200 OK with headers only, no body content

---

## Test 5: 404 Not Found
```bash
curl -v http://localhost:8080/nonexistent.html
```
**Expected:** 404 Not Found with error page

---

## Test 6: 405 Method Not Allowed
```bash
curl -X DELETE http://localhost:8080/
```
**Expected:** 405 Method Not Allowed with error page

---

## Test 7: Directory Traversal Security Test
```bash
# Try to access Makefile outside tests directory
curl -v --path-as-is "http://localhost:8080/../Makefile"

# Try double slashes
curl -v "http://localhost:8080//test.html"

# Try backslashes (Windows-style)
curl -v --path-as-is "http://localhost:8080/..\\Makefile"
```
**Expected:** 404 Not Found (security check blocks suspicious patterns)
**Note:** Returns 404 instead of 403 to prevent information disclosure about file existence

---

## Test 8: Browser Test
Open in browser:
```
http://localhost:8080/
```
**Expected:** Beautiful gradient page with team name displayed

---

## Test 9: Query String Test
```bash
curl -v "http://localhost:8080/?name=test&value=123"
```
**Expected:** 200 OK (query strings should be stripped and ignored for now)

---

## Test 10: Multiple Concurrent Connections
```bash
# Terminal 1
curl -v http://localhost:8080/ &

# Terminal 2
curl -v http://localhost:8080/test.html &

# Terminal 3
curl -v http://localhost:8080/style.css &
```
**Expected:** All three requests succeed independently

---

## Test 11: Keep-Alive Test
```bash
curl -v -H "Connection: keep-alive" http://localhost:8080/
```
**Expected:** 200 OK, parser should reset for next request

---

## Test 12: Large Headers Test
```bash
curl -v -H "X-Custom-Header: $(printf 'A%.0s' {1..1000})" http://localhost:8080/
```
**Expected:** 200 OK (handles large headers)

---

## Test 13: Empty URI Test
```bash
curl -v http://localhost:8080
```
**Expected:** 200 OK with index.html (empty URI defaults to /)

---

## Test 14: Case Sensitivity Test
```bash
curl -v http://localhost:8080/INDEX.HTML
```
**Expected:** 404 Not Found (file system is case-sensitive)

---

## Test 15: Graceful Shutdown Test
1. Start server: `./webserv`
2. Send request: `curl http://localhost:8080/` (in another terminal)
3. Press **Ctrl+C** in server terminal
4. Try **Ctrl+Z** (should be ignored now)

**Expected:** 
- Ctrl+C: Server logs "Received interrupt signal" and exits cleanly
- Ctrl+Z: Server ignores the signal and continues running

---

## Test 16: Rapid Requests Test
```bash
for i in {1..10}; do curl -s http://localhost:8080/ > /dev/null & done; wait
```
**Expected:** All 10 requests complete successfully

---

## Test 17: Invalid HTTP Version
```bash
telnet localhost 8080
GET / HTTP/2.0
Host: localhost

```
**Expected:** 400 Bad Request (only HTTP/1.0 and HTTP/1.1 supported)

---

## Test 18: Missing Host Header
```bash
printf "GET / HTTP/1.1\r\n\r\n" | nc localhost 8080
```
**Expected:** 400 Bad Request (Host header required in HTTP/1.1)

---

## Test 19: Malformed Request
```bash
printf "INVALID REQUEST\r\n\r\n" | nc localhost 8080
```
**Expected:** 400 Bad Request

---

## Test 20: Connection Persistence
```bash
(echo -e "GET / HTTP/1.1\r\nHost: localhost\r\n\r\n"; sleep 1; echo -e "GET /test.html HTTP/1.1\r\nHost: localhost\r\n\r\n") | nc localhost 8080
```
**Expected:** Two responses on same connection (keep-alive working)

---

## Notes:
- All tests should show proper logging in server terminal
- Memory should be freed after each request (check logs for "ClientConnection destroyed")
- No memory leaks (can verify with valgrind if needed)
- Port 8080 should be released after Ctrl+C
