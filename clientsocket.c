#include <sys/socket.h> // for socket definition
#include <sys/types.h>
#include <signal.h>
#include <stdio.h> // for input/output
#include <stdlib.h> // for standard library
#include <string.h> // for string library
#include <unistd.h> // for unix standard
#include <arpa/inet.h>
#include <stdarg.h> // for varargs
#include <errno.h>
#include <fcntl.h>
#include <sys/time.h>
#include <sys/ioctl.h>
#include <netdb.h>

#define SERVER_PORT 80

#define MAXLINE 4096
#define SA struct sockaddr

void err_n_die(const char *fmt, ...);

int main(int argc, char **argv) {

    int sockfd, n;
    int sendbytes;
    struct sockaddr_in servaddr;
    char sendline[MAXLINE];
    char recvline[MAXLINE];

    // Usage check
    if (argc != 2)
        err_n_die("usage: %s <server address>", argv[0]); // IP address is the server on the internet that we will find

    // Create socket
    // AF_INET = Address Family - Internet socket
    // SOCK_STREAM = Stream socket (other one is datagram socket, two main variety of sockets)
    // 0 = Use protocol number, default is 0, which is TCP for Stream sockets
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0)
        err_n_die("Error while creating the socket!");

    bzero(&servaddr, sizeof(servaddr)); // zero out the address
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERVER_PORT); // host to network, short - convert to network standard byte order to ensure two different computers of different byte orders

    if (inet_pton(AF_INET, argv[1], &servaddr.sin_addr) <= 0) // inet_pton converting string representation of ip address to binary equivalent 
        err_n_die("inet_pton error for %s ", argv[1]);

    if (connect(sockfd, (SA *) &servaddr, sizeof(servaddr)) < 0) // connect to server
        err_n_die("connect failed!");

    // we are connected, prepare the message
    sprintf(sendline, "GET / HTTP/1.1\r\n\r\n"); // get root page with / and then use HTTP1.1
    sendbytes = strlen(sendline);

    // send the request -- making sure you send it all
    // this code a bit fragile, since it bails if only some of the bytes are sent.
    // ideally, you would want to retry, unless the return value was -1
    if (write(sockfd, sendline, sendbytes) != sendbytes)
        err_n_die("write error");

    // Read server's response
    while( (n = read(sockfd, recvline, MAXLINE-1)) > 0){ // read socket
        printf("%s", recvline); // will render this html to show web pages later, for now we will just show html
        memset(recvline, 0, MAXLINE); // zero out each time
    }
    if (n < 0)
    err_n_die("read error");

    return 0;
}

void err_n_die(const char *fmt, ...){
    int errno_save;
    va_list ap; // va_list is a variable capable of storing a variable-length argument list

    // errno is set from any system or library calls, we save it for now
    errno_save = errno;

    va_start(ap, fmt); //var_start takes the va_list, and the variable that precedes the ellipsis

    // print out the fmt+args to standard out
    vfprintf(stdout, fmt, ap);
    fprintf(stdout, "\n");
    fflush(stdout);

    // print out error message if errno was set
    if (errno_save != 0){
        fprintf(stdout, "(errno = %d) : %s\n", errno_save, strerror(errno_save));
        fprintf(stdout, "\n");
        fflush(stdout);
    }

    va_end(ap);

    // terminate with error
    exit(1);
}