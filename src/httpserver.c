#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <string.h>
#include <signal.h>
#include <errno.h>
#include <stdio.h>    // for stderr, sscanf
#include <netinet/in.h>  // for sockaddr_in structs
#include <stdlib.h>      // for exit(.)
#include <unistd.h>      // for closing sockets, access(.)
#include <arpa/inet.h>   // for inet_ntoa
#include <regex.h>       // for regex
#include <fcntl.h>       // for open(.)
#include <limits.h>
#include <stdbool.h>

#include "httplib.h"
#include "httpserver.h"

// global vars
const int BACKLOG = 1024;

bool is_html(char* path) {

    char* file_extension = strchr(path, '.');
    if (!file_extension) return false;
    return strcmp(file_extension, ".html") == 0 ||
            strcmp(file_extension, ".htm") == 0;

}

// source: https://web.stanford.edu/class/archive/cs/cs110/cs110.1196/static/lectures/07-Signals/lecture-07-signals.pdf
static void reapChild(int unused) {

    while(true) {
        // WNOHANG allows to return without blocking even if there are more 
        // children still running
        pid_t pid = waitpid(-1, NULL, WNOHANG);
        if (pid <= 0) break;

    }
} 

// runs forever until SIG_INT
void server_start(int* socket_number, void (* request_handler)(int), int* server_port,
                char* conf_addr) {

    struct sockaddr_in server_address, client_address;
    size_t client_address_length = sizeof(client_address);
    int client_socket_number;

    // create a socket for ipv4 and tcp
    *socket_number = socket(PF_INET, SOCK_STREAM, 0);
    if (*socket_number == -1) {
        perror("Failed to create a new socket");
        exit(errno);
    }

    int socket_option = 1;
    if (setsockopt(*socket_number, SOL_SOCKET, SO_REUSEADDR, &socket_option, sizeof(socket_option)) == -1) {

        perror("Failed to set socket options");
        exit(errno);
    }    
     
    // setup arguments for bind()
    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(conf_addr);
    server_address.sin_port = htons(*server_port);

    if (bind(*socket_number, (struct sockaddr *) &server_address, sizeof(server_address)) == -1) {
        perror("Failed to bind");
        exit(errno);
    }

    if (listen(*socket_number, BACKLOG) == -1) {
        perror("Failed to listen");
        exit(errno);
    }
    printf("Listening on port %d (socket %d) ... \n", *server_port, *socket_number);

    while (1) {
        client_socket_number = accept(*socket_number, (struct sockaddr*) &client_address,
                (socklen_t*) &client_address_length);

        if (client_socket_number < 0) {
            perror("Error accepting socket");
            continue;
        } 

        printf("Accepted connection from %s on port %d\n", inet_ntoa(client_address.sin_addr),
                client_address.sin_port);
        // TODO: the fork thingy
        signal(SIGCHLD, reapChild);
        
        if (fork() == 0) {              // This is the child process
            close(*socket_number);      // child doesn't need the listener
            request_handler(client_socket_number);
            exit(EXIT_SUCCESS);
        } else {

            close(client_socket_number);
        }

        //request_handler(client_socket_number);

    }
    shutdown(*socket_number, SHUT_RDWR);
    close(*socket_number);


}


void get_file(int fd, char* path, int file_size) {
    
    int rfd = open(path, O_RDONLY);
    if (rfd < 0) {
        printf("Failed to read a file\n");
        http_server_failure(fd);
        close(rfd);
        return; 

    }
    char* buffer = malloc(file_size);
    int bytes_read = read(rfd, buffer, file_size);

    // check if something went wrong
    if (bytes_read > file_size || bytes_read < 0 || bytes_read < file_size) {

        printf("Failed to read a file\n");
        http_server_failure(fd);
        close(rfd);
        free(buffer);
        return;
    }

    http_response_status_line(fd, 200);
    http_send_header(fd, "Content-Type", "text/html");
    http_send_content_length_header(fd, bytes_read);
    http_send_header(fd, "Connection", "close");
    http_end_headers(fd);    
   
    
    write(fd, buffer, bytes_read);

    close(rfd);
    free(buffer);

}

void handle_request(int fd) {
    
    // parse http request, pass in fd
    http_request_t* request = parse_http_request(fd);
    // bad request case
    if (!request || request->path[0] != '/') {
        char message[] = "Poorly formatted request.";
        get_bad_response(message, 400, fd);

        close(fd);
        return;
    }

    // if request is for a file outside of the designated folder 
    if (strstr(request->path, "..") != NULL) {
        char message[] = "No access granted to files outside this directory.";
        get_bad_response(message, 403, fd);
        close(fd);
        return;
    }

    // check if the file at the specified path exists
    // NOTE: ///.../ <-> /
    // path formatting (strlen excludes the null terminator)
    char* path = malloc(1 + strlen(request->path) + 1);
    path[0] = '.';
    memcpy(path + 1, request->path, strlen(request->path)+1);
    
    // initiate a response
    struct stat stat_path;
    // NOTE: stat(.) == 0 in case of a directory as well
    if (stat(path, &stat_path) != 0) {
        bool is_html_bool = is_html(path+1);
        int status_code = is_html_bool ? 404 : 400; 
        char message[30];
        if (status_code == 400) { sprintf(message, "Not an html file."); }
        else { sprintf(message, "File not found."); }
        
        get_bad_response(message, status_code, fd);

    } else if (S_ISREG(stat_path.st_mode)){
        if (!is_html(path+1)) {
            char message [] = "Not an html file.";
            
            get_bad_response(message, 400, fd);

        } else {

            get_file(fd, path, stat_path.st_size);
        }       
    } else {
        char message [] = "Not a file request.";
        get_bad_response(message, 400, fd);

    } 
    
    free(path);
    close(fd);
    return;
    
}


