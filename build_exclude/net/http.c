#include "http.h"
#include "../vga.h"

int http_request(http_method_t method, const char* url, const char* headers, const char* body, http_response_t* response) {
    (void)method;
    (void)url;
    (void)headers;
    (void)body;
    (void)response;
    vga_puts("Performing HTTP request (placeholder).\n");
    return -1; // Not implemented
}

