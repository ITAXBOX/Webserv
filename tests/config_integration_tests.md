# Config Integration Tests

## Overview
These tests verify the config file parser integration with the WebServer.

---

## Test 1: No Config (Default Server)

### Command:
```bash
./webserv
```

### Expected Output:
```
[WARN] No configuration file provided. Using defaults.
[INFO] Default server configured: 0.0.0.0:8080
```

### Verification:
```bash
curl -v http://localhost:8080/
# Should return 200 OK with index.html from ./tests/
```

---

## Test 2: With Config File (Multiple Servers)

### Command:
```bash
./webserv conf/default.conf
```

### Expected Output:
```
[INFO] Loading configuration from: conf/default.conf
[INFO] Config parsed successfully: 2 server(s)
[INFO] Server configured: 0.0.0.0:8080 (localhost)
[INFO] Server configured: 0.0.0.0:8081 (example.com, www.example.com)
```

### Verification:
```bash
# Test server 1 (port 8080)
curl -v http://localhost:8080/
# Should return 200 OK

# Test server 2 (port 8081)
curl -v http://localhost:8081/
# Should return 404 (no ./site2 directory) or 200 if directory exists
```

---

## Test 3: Invalid Config File

### Command:
```bash
./webserv nonexistent.conf
```

### Expected Output:
```
[ERROR] Failed to open config file: nonexistent.conf
[ERROR] Failed to load configuration
[ERROR] WebServer initialization failed
```

### Exit Code:
`1`

---

## Test 4: Config with Syntax Error

### Setup:
```bash
cat > /tmp/bad.conf << 'EOF'
server {
    listen 8080
    # Missing semicolon ^
    root ./tests;
}
EOF
```

### Command:
```bash
./webserv /tmp/bad.conf
```

### Expected Output:
```
[ERROR] Config parsing failed: Line X: Expected ';'
[ERROR] Failed to load configuration
[ERROR] WebServer initialization failed
```

---

## Test 5: Config with Multiple Locations

### Setup:
```bash
cat > /tmp/full.conf << 'EOF'
server {
    listen 9090;
    server_name myserver.local;
    root ./tests;
    index index.html test.html;
    
    location / {
        allowed_methods GET HEAD POST;
    }
    
    location /api {
        allowed_methods GET POST DELETE;
    }
}
EOF
```

### Command:
```bash
./webserv /tmp/full.conf
```

### Expected Output:
```
[INFO] Server configured: 0.0.0.0:9090 (myserver.local)
```

### Verification:
```bash
curl http://localhost:9090/
curl http://localhost:9090/api
```

---

## Test 6: Port Conflict

### Setup:
**Terminal 1:**
```bash
./webserv conf/default.conf
# Servers on 8080 and 8081
```

**Terminal 2:**
```bash
./webserv conf/default.conf
```

### Expected Output (Terminal 2):
```
[ERROR] bind(0.0.0.0:8080) failed: Address already in use
[ERROR] Failed to initialize server on 0.0.0.0:8080
[ERROR] Failed to setup servers
[ERROR] WebServer initialization failed
```

---

## Test 7: Config Parser Standalone Test

### Compile Test Program:
```bash
c++ -Wall -Wextra -Werror -std=c++98 -Iinclude -o test_parser \
    src/config/test_parser.cpp \
    src/config/Tokenizer.cpp \
    src/config/ConfigParser.cpp \
    src/config/ConfigDirectives.cpp \
    src/config/ServerConfig.cpp \
    src/config/LocationConfig.cpp \
    src/utils/utils.cpp \
    src/utils/Logger.cpp
```

### Command:
```bash
./test_parser conf/default.conf
```

### Expected Output:
```
=== Parsing Config: conf/default.conf ===
✓ Tokenization successful (49 tokens)
✓ Parsing successful

=== Configuration ===
Found 2 server(s):

Server #1:
  Host: 
  Port: 8080
  Server Names: localhost
  Root: ./tests
  Index: index.html
  Client Max Body Size: 1048576 bytes
  Locations (1):
    - Path: /
      Allowed Methods: GET, HEAD

Server #2:
  Host: 
  Port: 8081
  Server Names: example.com, www.example.com
  Root: ./site2
  Index: home.html
  Client Max Body Size: 1048576 bytes
  Locations (1):
    - Path: /api
      Allowed Methods: DELETE, GET, POST

✅ Configuration parsed successfully!
```

---

## Test 8: Server Name Matching

### Command:
```bash
./webserv conf/default.conf
```

### Verification:
```bash
# Test with different Host headers
curl -H "Host: localhost:8080" http://localhost:8080/
# Should use server 1 (root: ./tests)

curl -H "Host: example.com:8081" http://localhost:8081/
# Should use server 2 (root: ./site2)

curl -H "Host: www.example.com:8081" http://localhost:8081/
# Should also use server 2
```

---

## Test 9: Clean Shutdown

### Command:
```bash
./webserv conf/default.conf
```

### Verification:
```bash
# In another terminal, check both ports are listening
netstat -tuln | grep -E '8080|8081'
# Should show both ports LISTEN

# Press Ctrl+C in server terminal
```

### Expected Output:
```
[INFO] Received interrupt signal (Ctrl+C)
[INFO] Stopping WebServer...
[INFO] Event loop stopped
[INFO] Server shutdown complete
```

### Verify ports released:
```bash
netstat -tuln | grep -E '8080|8081'
# Should be empty
```

---

## Test 10: Config Validation (Empty Server Block)

### Setup:
```bash
cat > /tmp/empty.conf << 'EOF'
server {
}
EOF
```

### Command:
```bash
./webserv /tmp/empty.conf
```

### Expected Behavior:
Server should start with default values:
- Port: 8080 (default)
- Host: 0.0.0.0 (default)
- Root: ./tests (default)

---

## Test 11: Tokenizer Standalone Test

### Compile Test Program:
```bash
c++ -Wall -Wextra -Werror -std=c++98 -Iinclude -o test_tokenizer \
    src/config/test_tokenizer.cpp \
    src/config/Tokenizer.cpp \
    src/utils/utils.cpp
```

### Command:
```bash
./test_tokenizer conf/default.conf
```

### Expected Output:
```
=== Tokenizing: conf/default.conf ===
File content (339 bytes):
[... config file content ...]

=== Tokens ===
[0] Line 4: WORD ("server")
[1] Line 4: OPEN_BRACE ("{")
[2] Line 5: WORD ("listen")
[3] Line 5: WORD ("8080")
[4] Line 5: SEMICOLON (";")
...
[48] Line 25: EOF

✅ Tokenization successful! 49 tokens generated.
```

---

## Automated Test Script

Save as `test_config_integration.sh`:

```bash
#!/bin/bash

RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo "========================================"
echo "   Config Integration Test Suite"
echo "========================================"

# Test 1: Default server
echo -e "\n${YELLOW}[Test 1]${NC} Default server (no config)..."
timeout 2 ./webserv > /tmp/test1.log 2>&1 &
PID=$!
sleep 1
RESULT=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:8080/ 2>/dev/null)
kill $PID 2>/dev/null
wait $PID 2>/dev/null
if [ "$RESULT" = "200" ]; then
    echo -e "${GREEN}✓ PASS${NC} - Default server responds on port 8080"
else
    echo -e "${RED}✗ FAIL${NC} - Expected 200, got $RESULT"
fi
sleep 1

# Test 2: Config file with 2 servers
echo -e "\n${YELLOW}[Test 2]${NC} Config file with multiple servers..."
timeout 2 ./webserv conf/default.conf > /tmp/test2.log 2>&1 &
PID=$!
sleep 1
RESULT1=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:8080/ 2>/dev/null)
RESULT2=$(curl -s -o /dev/null -w "%{http_code}" http://localhost:8081/ 2>/dev/null)
kill $PID 2>/dev/null
wait $PID 2>/dev/null
if [ "$RESULT1" = "200" ] && [ "$RESULT2" != "000" ]; then
    echo -e "${GREEN}✓ PASS${NC} - Both servers respond (8080: $RESULT1, 8081: $RESULT2)"
else
    echo -e "${RED}✗ FAIL${NC} - Port 8080: $RESULT1, Port 8081: $RESULT2"
fi
sleep 1

# Test 3: Invalid config file
echo -e "\n${YELLOW}[Test 3]${NC} Invalid config file..."
./webserv nonexistent.conf > /tmp/test3.log 2>&1
EXIT_CODE=$?
if [ $EXIT_CODE -eq 1 ] && grep -q "Failed to open config file" /tmp/test3.log; then
    echo -e "${GREEN}✓ PASS${NC} - Correctly rejected invalid config"
else
    echo -e "${RED}✗ FAIL${NC} - Should have failed with error"
fi

# Test 4: Parser standalone
echo -e "\n${YELLOW}[Test 4]${NC} Config parser standalone..."
if [ -f ./test_parser ]; then
    ./test_parser conf/default.conf > /tmp/test4.log 2>&1
    if grep -q "Configuration parsed successfully" /tmp/test4.log; then
        SERVERS=$(grep -o "Found [0-9]* server" /tmp/test4.log | grep -o "[0-9]*")
        echo -e "${GREEN}✓ PASS${NC} - Parser works ($SERVERS servers found)"
    else
        echo -e "${RED}✗ FAIL${NC} - Parser failed"
    fi
else
    echo -e "${YELLOW}⊘ SKIP${NC} - test_parser not compiled"
fi

echo -e "\n========================================"
echo "   Tests Complete!"
echo "========================================"
```

### Run the automated tests:
```bash
chmod +x test_config_integration.sh
./test_config_integration.sh
```

---

## Notes

- All config files are in `conf/` directory
- Test files are served from `./tests/` directory
- Default port is 8080 if not specified in config
- Default host is 0.0.0.0 (all interfaces)
- Ctrl+Z is now ignored (only Ctrl+C stops the server)
- Server supports multiple virtual hosts on different ports
