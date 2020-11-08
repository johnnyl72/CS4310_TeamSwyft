#include <strings.h>
#include "common.h"

int main(int argc, char **argv){

    int listenfd, connfd, n;
    struct sockaddr_in servaddr;
    uint8_t buff[MAXLINE+1];
    uint8_t recvline[MAXLINE+1];

    // Create socket
    if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) // Internet, TCP stream socket
        err_n_die("socket error");

    // setting up address that we will be listening on, accepting connections!
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    // htonl converts a long integer (e.g. address) to a network representation
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY); // INDDR_ANY = responding to any internet address
    // htons converts a short integer (e.g. port) to a network representation
    servaddr.sin_port = htons(SERVER_PORT); // 1800 is fine because  port 80 might require reconfig or superuser

    // listen & bind, this socket will listen to this address
    if ((bind(listenfd, (SA *) &servaddr, sizeof(servaddr))) < 0) 
        err_n_die("bind error");
    if ((listen(listenfd, 10)) < 0)
        err_n_die("listen error"); 

    // infinite loop to accept connections
    for (;;){
        struct sockaddr_in addr;
        socklen_t addr_len;
        char client_address[MAXLINE+1];
        // accept blocks until an incoming connection arrives
        // it returns a "file descriptor" to the connection
        printf("Waiting for a connection on port %d\n", SERVER_PORT);
        fflush(stdout);
        // connfd is another socket that we use to talk to the connected client
        connfd = accept(listenfd, (SA *) &addr, &addr_len); // pass into listening file descriptor, Both null to accept any connections

        inet_ntop(AF_INET, &addr, client_address, MAXLINE); // network to presentation format, since address will be a struct (network type) full of binary data
        printf("Client connection: %s\n", client_address);

        // zero out the receive buffer to make sure it ends up null- terminated
        memset(recvline, 0, MAXLINE);

        //read client's message
        while ((n = read(connfd, recvline, MAXLINE-1)) > 0){
            fprintf(stdout, "\n%s\n\n%s", bin2hex(recvline, n), recvline); // printin the binary and text version of message

            if (recvline[n-1] == '\n'){
                break;
            }

            memset(recvline, 0, MAXLINE); // zero out to read next time so we know its null-terminated
        }

        if (n < 0)
        err_n_die("read error");

        //send a response
        snprintf((char*)buff, sizeof(buff), "HTTP/1.0 200 OK\r\n\r\nHello"); // Normally the Hello part would be our response page

        write(connfd, (char*)buff, strlen((char*)buff)); //Write the response (normally our web page) into this socket
        close(connfd);
    }
    return 0;
}