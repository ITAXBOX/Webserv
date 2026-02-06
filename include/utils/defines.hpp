#ifndef DEFINES_HPP
#define DEFINES_HPP

// ============================================================================
// HTTP Status Codes
// ============================================================================

// 2xx Success
#define HTTP_OK 200
#define HTTP_CREATED 201
#define HTTP_NO_CONTENT 204

// 4xx Client Errors
#define HTTP_BAD_REQUEST 400
#define HTTP_FORBIDDEN 403
#define HTTP_NOT_FOUND 404
#define HTTP_METHOD_NOT_ALLOWED 405
#define HTTP_PAYLOAD_TOO_LARGE 413

// 5xx Server Errors
#define HTTP_INTERNAL_SERVER_ERROR 500

// ============================================================================
// Buffer and Limit Constants
// ============================================================================

#define BUFFER_SIZE 1024
#define MAX_BODY_SIZE 1048576  // 1MB (1024 * 1024)

// ============================================================================
// Default Server Configuration
// ============================================================================

// 0.0.0.0 means bind to all interfaces
// This allows the server to accept connections on any network interface
// like 127.0.0.1 (localhost)
// localhost
// 192.168.x.x
// 10.x.x.x
// or any other IP assigned to the machine
#define DEFAULT_HOST "0.0.0.0"
#define DEFAULT_PORT 8080
#define DEFAULT_ROOT "./www"
#define DEFAULT_INDEX "index.html"
#define DEFAULT_BACKLOG 128

#endif
