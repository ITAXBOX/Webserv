*This project has been created as part of the 42 curriculum by Unknown.*

# Webserv

## Description
This project is an implementation of a non-blocking HTTP server in C++ 98. It is designed to handle multiple client connections simultaneously using a single event loop and `epoll` (or equivalent). The server supports hosting static websites, file uploads, and executing CGI scripts (Python, PHP, etc.) compliant with the HTTP/1.1 protocol.

Key features:
- **Non-blocking I/O**: Uses a single reactor pattern with `epoll` to handle all sockets and pipes without blocking.
- **HTTP Methods**: Supports GET, POST, and DELETE.
- **CGI Support**: Asynchronous CGI execution (Process & Pipe management) without blocking the main event loop.
- **Configuration**: robust configuration file support (inspired by NGINX).
- **Multipart Uploads**: Handles large file uploads via `multipart/form-data`.

## Instructions

### Compilation
To compile the project, run:
```bash
make
```
This will generate the `webserv` executable.

### Execution
Run the server with a configuration file:
```bash
./webserv conf/default.conf
```
If no configuration file is provided, it may default to hardcoded values or fail depending on strictness.

### Usage
- Open a web browser and navigate to `http://localhost:8080` (or the port defined in your config).
- Use `curl` to test specific methods:
  ```bash
  curl -v http://localhost:8080/
  curl -X POST -F "file=@test.txt" http://localhost:8080/upload
  ```

## Resources

### References
- [RFC 2616 (HTTP/1.1)](https://tools.ietf.org/html/rfc2616)
- [Beej's Guide to Network Programming](https://beej.us/guide/bgnet/)
- [NGINX Documentation](https://nginx.org/en/docs/)

### AI Usage
AI tools (GitHub Copilot) were used to:
- Refactor the CGI execution engine to be fully asynchronous and non-blocking, ensuring strict compliance with the project requirements.
- Fix the `Poller` implementation to correctly use Level-Triggered mode for safer I/O handling.
- specific bug fixes and code structure improvements.
