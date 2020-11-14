#include <sys/socket.h> // Socket definition
#include <stdio.h> // Input & Output
#include <stdlib.h> // Standard library
#include <string.h> // String library, size_t
#include <unistd.h> // Unix standard
#include <arpa/inet.h> // Integer to network representation
#include <errno.h> // Error Message
#include <pthread.h>
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
        
    }
    return 0;
}
void * process(void* p_connect_socket){
    int connect_socket = *( (int*) p_connect_socket);
    int message_size;
    char buff[MAX_BUFF_SIZE], recvline[MAX_BUFF_SIZE];
    sleep(10);
    memset(recvline, '\0', MAX_BUFF_SIZE);  // Zero out the receive buffer to make sure it ends up null-terminated
    while ((message_size = read(connect_socket, recvline, MAX_BUFF_SIZE)) > 0){ // read(): https://pubs.opengroup.org/onlinepubs/009695399/functions/read.html

            // printf vs sprtinf vs fprintf: https://www.geeksforgeeks.org/difference-printf-sprintf-fprintf/
            printf("\n%s\n", recvline);

            if (recvline[message_size-1] == '\n'){
                break;
            }

            memset(recvline, 0, MAX_BUFF_SIZE); // Zero out to read next time so we know its null-terminated
        }

        if (message_size < 0){
            perror("There was a reading error!");
            exit(1);
        }

        // Send a response back
        snprintf((char*)buff, sizeof(buff), "HTTP/1.0 200 OK\r\n\r\nHello"); // Our next job is to create a method to create a proper HTTP response back to display our game
        // snprintf((char*)buff, sizeof(buff), "HTTP/1.1 200 OK\nContent-Type: text/plain\nContent-Length: 12\n\nHello world!");

        write(connect_socket, buff, strlen((char*)buff)); // Write the response (eventually our web page) into this socket
        close(connect_socket);
        printf("End of connection");

        // return NULL;
}
