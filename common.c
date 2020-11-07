#include "common.h"


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


char *bin2hex(const unsigned char *input, size_t len){
    char *result;
    char *hexits = "0123456789ABCDEF";

    if (input == NULL || len <= 0)
        return NULL;   

    //(2 hexits+space)/char + NULL
    int resultlength = (len*3)+1;

    result = malloc(resultlength);
    bzero(result, resultlength);
    for(int i = 0; i < len; i++){
        result[i*3] = hexits[input[i] >> 4];
        result[(i*3)+1] = hexits[input[i] & 0X0F];
        result[(i*3)+2] = ' '; // for readability
    }

    return result;
}