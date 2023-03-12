#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>      // for dprintf
#include <string.h>
#include <errno.h>

#include "httplib.h"

// parsing and error handling 
void http_mem_fail_error() {
    perror("Malloc failed");
    exit(ENOBUFS);

}


void http_server_failure(int fd) {

    char message[] = "Server Error\n";
    
    http_response_status_line(fd, 500);
    write(fd, message, sizeof(message) -1);
    http_send_header(fd, "Content-Type", "text/html");
    http_send_header(fd, "Connection", "close");
    http_end_headers(fd); 

}

http_request_t* parse_http_request(int fd) {

    // malloc what needs to be malloced
    http_request_t* request = malloc(sizeof(struct http_request));
    if (!request) {
       http_mem_fail_error(); 
    }

    char* buffer = malloc(HTTP_REQUEST_MAX+1);
    if (!buffer) http_mem_fail_error();

    int bytes_read = read(fd, buffer, HTTP_REQUEST_MAX);
    buffer[bytes_read] = '\0';

    char *start, *end;
    size_t read_size;
    do {
        // get method
        start = end = buffer;
        while (*end >= 'A' && *end <= 'Z') {
            end++;
        }
        read_size = end - start;
        if (read_size == 0) break;

        // sizeof(char) is 1 anyway
        request->method = malloc(read_size + 1);
        if (!request->method) http_mem_fail_error();
        memcpy(request->method, start, read_size);  
        request->method[read_size] = '\0';
        
        // skip space
        start = end;
        if (*end != ' ') break;
        end++;

        // get path
        // REMEMBER: The given path is relative to the server directory
        start = end;
        while (*end != '\0' && *end != ' ' && *end != '\n') end++;
        read_size = end - start;
        if (!read_size) break;
        request->path = malloc(read_size+1);
        if(!request->path) http_mem_fail_error();
        memcpy(request->path, start, read_size);
        request->path[read_size] = '\0';
            
        // go through the rest of the request to ensure it is 
        // properly formatted 
        start = end;
        while (*end != '\0' && *end != '\n') end++;
        if (*end != '\n') break;
        end++;

        free(buffer);
        return request;

    } while(0);

    // error occured, handle with appropriate response
    free(request);
    free(buffer);
    return NULL;
    

}

char* get_http_status_message(int status_code) {
    switch (status_code) {
        case 200:
            return "OK";
        case 400:
            return "Bad Request";
        case 403:
            return "Forbidden";
        case 404:
            return "Not Found";
        default:
            return "Internal Server Error";
    }

}

// http response
// bad response
void get_bad_response(char* msg, int status_code, int fd) {
        char message[100]; 
        sprintf(message, "<h1>%d: %s</h1>\n<p style=\"font-size:24;\">%s</p>\n", status_code, get_http_status_message(status_code), msg);
        http_response_status_line(fd, status_code);
        http_send_header(fd, "Content-Type", "text/html");
        http_send_header(fd, "Connection", "close");
        http_end_headers(fd);    
        write(fd, message, strlen(message)+1); 

}

// status line
void http_response_status_line(int fd, int status_code) {
        
    // dprintf using file descriptor
    dprintf(fd, "HTTP/1.1 %d %s\r\n", status_code, get_http_status_message(status_code));
}
    
// sending header (generic template)
void http_send_header(int fd, char* key, char* value) { dprintf(fd, "%s: %s\r\n", key, value); }

// for sending content length header
void http_send_content_length_header(int fd, int content_length) {
    char length_buffer[31];
    sprintf(length_buffer, "%d", content_length);
    http_send_header(fd, "Content-Length", length_buffer); 

}

void http_end_headers(int fd) {dprintf(fd, "\r\n");}
