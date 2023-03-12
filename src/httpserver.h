#ifndef HTTPSERVER_H
#define HTTPSERVER_H

bool is_html(char* path);
void server_start(int* socket_number, void (* request_handler)(int), int* server_port,
                char* conf_addr);
void get_file(int fd, char* path, int file_size);
void handle_request(int fd);

#endif
