#ifndef HTTPLIB_H
#define HTTPLIB_H

// PATH_MAX is 4096, so HTTP_REQUEST_MAX has to be at least 4096
#define HTTP_REQUEST_MAX 8192
// Struct for storing a parsed http request
typedef struct http_request {
    char* method;
    char* path;
} http_request_t;

// parsing and error handling
void http_mem_fail_error();
void http_server_failure(int fd);
http_request_t* parse_http_request(int fd);


// For sending HTTP response.
void get_bad_response(char* msg, int status_code, int fd);
// sending status line
void http_response_status_line(int fd, int status_code);
// sending response header (content type. content length, date modified)
void http_send_header(int fd, char* key, char* value);
void http_send_content_length_header(int fd, int content_length);
// delimiting new line
void http_end_headers(int fd);


#endif
