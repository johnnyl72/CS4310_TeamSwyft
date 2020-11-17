#include <sys/socket.h> // Socket definition
#include <stdio.h> // Input & Output
#include <stdlib.h> // Standard library
#include <string.h> // String library, size_t
#include <unistd.h> // Unix standard
#include <arpa/inet.h> // Integer to network representation
#include <errno.h> // Error Message
#include <pthread.h> // Multithreading

#include <limits.h> // Path limit
#include <sys/types.h> // open, recv
#include <sys/stat.h> //open
#include <fcntl.h> //open


#define SERVER_PORT 1234
#define MAX_BUFF_SIZE 4096

void * process(void* connect_socket);

int main(int argc, char **argv){

    int listen_socket, connect_socket;
    struct sockaddr_in server_address;
    // Create socket
    if ( (listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0 ){
        perror("Socket Creation Failed!");
        exit(1);
    }
    printf("PRINT SOCKET IS: %d", listen_socket);
    // Setting up the address that will be used for listening (accepting connections)
    memset((char *) &server_address, '\0', sizeof(server_address)); // Zero out server_address first
    server_address.sin_family = AF_INET; 
    // htonl converts a long integer (e.g. address) to a network representation
    server_address.sin_addr.s_addr = htonl(INADDR_ANY); // INDDR_ANY = Responding to any of our available interfaces (e.g. localhost, ethernet, wifi)
    // htons converts a short integer (e.g. port) to a network representation
    server_address.sin_port = htons(SERVER_PORT); // Arbitrary port number

    // Listen & Bind our listen_socket to our server_address (i.e. INADDR_ANY)
    if ( (bind(listen_socket, (struct sockaddr *) &server_address, sizeof(server_address))) < 0 ) {
        perror("Binding Error!");
        exit(1);
    }
    // Queue of 2 to connect, any more will throw error
    if ( (listen(listen_socket, 2)) < 0 ){
        perror("Listen Error!");
        exit(1);
    }

    // Infinite loop to accept connections
    while(1){
        struct sockaddr_in client_address;
        socklen_t addr_len;
        char client_ip[MAX_BUFF_SIZE];

        printf("Waiting for a connection on port %d\n", SERVER_PORT);
        fflush(stdout); // Force flush mainly for debugging purposes: https://www.youtube.com/watch?v=T9adOz_EHnQ

        // connect_socket is another socket that is used to talk with the connected client
        if ( (connect_socket = accept(listen_socket, (struct sockaddr *) &client_address, &addr_len)) < 0) { // pass into listening file descriptor
            perror("Accept failed!");
            exit(1);
        } 

        // Print the client's ip address 
        inet_ntop(AF_INET, &client_address, client_ip, MAX_BUFF_SIZE); // Network to presentation format (ntop) due to client_address being a struct (network type) full of binary data
        printf("Client's IP address: %s\n", client_ip); // Bug here on first connection

        // Process client's message
        pthread_t thread;
        // process(connect_socket);
        pthread_create(&thread, NULL, process, &connect_socket);
        pthread_join(thread, NULL);
    }
    return 0;
}
// Accepts a null pointer because we used p_thread
void * process(void* p_connect_socket){
    int connect_socket = *( (int*) p_connect_socket );
    char *req_head_tokens[3], msg_req[MAX_BUFF_SIZE], msg_resp[MAX_BUFF_SIZE], path[PATH_MAX];
    int message_size, write_fd, read_fd;

    memset(msg_req, '\0', MAX_BUFF_SIZE);
    read_fd = read(connect_socket, msg_req, MAX_BUFF_SIZE);
    char *ROOT = getenv("PWD");

    if( read_fd > 0){
        printf("\n%s", msg_req);
        req_head_tokens[0] = strtok(msg_req, " ");
        if(strncmp(req_head_tokens[0], "GET", sizeof(req_head_tokens[0])) == 0){
            // Example: GET / HTTP/1.1
            req_head_tokens[1] = strtok(NULL, " ");
            req_head_tokens[2] = strtok(NULL, " \n");
            if( strncmp( req_head_tokens[1], "/\0", 2) == 0)
                req_head_tokens[1] = "/index.html";

            if( (strncmp( req_head_tokens[2], "HTTP/1.1", 8) == 0) || (strncmp( req_head_tokens[2], "HTTP/1.0", 8) == 0)){
                strcpy(path, ROOT);
                strcpy(&path[strlen(ROOT)], req_head_tokens[1]);
                printf("Request file: %s\n", path);
                //O_RDONLY = READ ONLY
                if( (write_fd = open(path, O_RDONLY)) > 0 ){
                    send(connect_socket, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
                    while( (message_size=read(write_fd, msg_resp, MAX_BUFF_SIZE)) > 0 ){
                        write(connect_socket, msg_resp, message_size);
                    }
                }
                else{
                    write(connect_socket, "HTTP/1.1 404 Not Found\r\n", 24);
                }
            }
            else{
                write(connect_socket, "HTTP/1.1 400 Bad Request\r\n", 26);
            }
        }
    }

    close(connect_socket);
    printf("End of connection\n----------------------------------------------------------------------------------\n");

    // return NULL;
}
