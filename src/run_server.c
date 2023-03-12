#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>
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
int server_fd;

void signal_handler(int signum) {
    
    printf("Signal recieved %d: %s\n", signum, strsignal(signum));
    printf("Closing socket %d\n", server_fd);    
    if (close(server_fd) < 0)
        perror("Failed to close server (ignoring)\n");
    exit(0);

}

char* USAGE = 
    "\nUsage: ./httpserver --server_directory <server_directory> [CONFIG OPTION...]\n"
    "CONFIG OPTIONS:                                                               \n"
    "	--port <port number 1024 or greater>                                       \n"
    "	--address <IP address in dot notation>                                     \n";

void exit_with_usage() {

    fprintf(stderr, "%s", USAGE);
    exit(EXIT_SUCCESS);

}

int main(int argc, char** argv) {

    signal(SIGINT, signal_handler);
    signal(SIGPIPE, SIG_IGN);
    
    // default
    int server_port = 8000;
    char* conf_addr="127.0.0.1";
    void (*request_handler)(int) = handle_request;
    char* server_directory = NULL;
    
    // parse the args
    for (int i = 1; i < argc; i++) {
        if (strcmp("--address", argv[i]) == 0) {
           // check if appropriate address             
            conf_addr=argv[++i];
            if (inet_addr(conf_addr) == (in_addr_t) -1) {
                printf("Invalid adderss.\n"); 
                exit(errno);
            }
        } else if (strcmp("--port", argv[i]) == 0) {
           if (sscanf(argv[++i], "%d", &server_port) != 1) {
                printf("Invalid port\n");
                exit(1);
            }        
        } else if (strcmp("--server_directory", argv[i]) == 0) {
           server_directory = argv[++i];
        } else if (strcmp("--help", argv[i]) == 0) {
            exit_with_usage();
        } else {

            fprintf(stderr, "Unrecognized option: %s\n", argv[i]);
            exit_with_usage();
        }


    }

    if (!server_directory) {
        fprintf(stderr, "no server_directory specified");
        exit(EXIT_FAILURE);
    }

    if (chdir(server_directory) == -1) {
        perror("Invalid server directory.\n");
        exit(errno);
    }

    server_start(&server_fd, request_handler, &server_port, conf_addr);
    return EXIT_SUCCESS;
}
