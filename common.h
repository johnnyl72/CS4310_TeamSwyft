#ifndef _COMMON_H_ // header guard
#define _COMMON_H_

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

// useful constants
#define SERVER_PORT 1800 // We pick 1800 because 

#define MAXLINE 4096
#define SA struct sockaddr

void err_n_die(const char *fmt, ...);
char *bin2hex(const unsigned char *input, size_t len);

#endif