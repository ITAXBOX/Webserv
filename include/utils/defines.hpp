#ifndef DEFINES_HPP
#define DEFINES_HPP

// ============================================================================
// HTTP Status Codes
// ============================================================================

// 2xx Success
#define HTTP_OK 200

// 4xx Client Errors
#define HTTP_BAD_REQUEST 400
#define HTTP_FORBIDDEN 403
#define HTTP_NOT_FOUND 404
#define HTTP_METHOD_NOT_ALLOWED 405

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

#define DEFAULT_HOST "0.0.0.0"
#define DEFAULT_PORT 8080
#define DEFAULT_ROOT "./www"
#define DEFAULT_INDEX "index.html"
#define DEFAULT_BACKLOG 128

#endif
