#ifndef HTTP_H
#define HTTP_H

#include <stdint.h>
#include "tcpip.h"

// HTTP request method
typedef enum {
    HTTP_METHOD_GET,
    HTTP_METHOD_POST
} http_method_t;

// HTTP response structure (simplified)
typedef struct {
    uint16_t status_code;
    char* body;
    uint32_t body_len;
} http_response_t;

// Perform an HTTP request
int http_request(http_method_t method, const char* url, const char* headers, const char* body, http_response_t* response);

#endif // HTTP_H
