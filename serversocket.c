#include <sys/socket.h> // Socket definition
#include <stdio.h>		// Input & Output
#include <stdlib.h>		// Standard library
#include <string.h>		// String library, size_t
#include <unistd.h>		// Unix standard
#include <arpa/inet.h>	// Integer to network representation
#include <errno.h>		// Error Message
#include <pthread.h>	// Multithreading

#include <limits.h>	   // Path limit
#include <sys/types.h> // open, recv
#include <sys/stat.h>  //open
#include <fcntl.h>	   //open

#define SERVER_PORT 1234
#define MAX_BUFF_SIZE 4096

void *process(void *connect_socket);
char* replace_char(char* str, char find, char replace);
int main(int argc, char **argv)
{

	int listen_socket, connect_socket;
	struct sockaddr_in server_address;
	// Create socket
	if ((listen_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0)
	{
		perror("Socket Creation Failed!");
		exit(1);
	}
	// printf("PRINT SOCKET IS: %d", listen_socket);
	// Setting up the address that will be used for listening (accepting connections)
	memset((char *)&server_address, '\0', sizeof(server_address)); // Zero out server_address first
	server_address.sin_family = AF_INET;
	// htonl converts a long integer (e.g. address) to a network representation
	server_address.sin_addr.s_addr = htonl(INADDR_ANY); // INDDR_ANY = Responding to any of our available interfaces (e.g. localhost, ethernet, wifi)
	// htons converts a short integer (e.g. port) to a network representation
	server_address.sin_port = htons(SERVER_PORT); // Arbitrary port number

	// Listen & Bind our listen_socket to our server_address (i.e. INADDR_ANY)
	if ((bind(listen_socket, (struct sockaddr *)&server_address, sizeof(server_address))) < 0)
	{
		perror("Binding Error!");
		exit(1);
	}
	// Queue of 2 to connect, any more will throw error
	if ((listen(listen_socket, 10)) < 0)
	{
		perror("Listen Error!");
		exit(1);
	}

	// Infinite loop to accept connections
	while (1)
	{
		struct sockaddr_in client_address;
		socklen_t addr_len;
		char client_ip[MAX_BUFF_SIZE];

		printf("Waiting for a connection on port %d\n", SERVER_PORT);
		fflush(stdout); // Force flush mainly for debugging purposes: https://www.youtube.com/watch?v=T9adOz_EHnQ

		// connect_socket is another socket that is used to talk with the connected client
		if ((connect_socket = accept(listen_socket, (struct sockaddr *)&client_address, &addr_len)) < 0)
		{ // pass into listening file descriptor
			perror("Accept failed!");
			exit(1);
		}

		// Print the client's ip address
		inet_ntop(AF_INET, &client_address, client_ip, MAX_BUFF_SIZE); // Network to presentation format (ntop) due to client_address being a struct (network type) full of binary data
		printf("Client's IP address: %s\n", client_ip);				   // Bug here on first connection

		// Process client's message
		pthread_t thread;
		pthread_create(&thread, NULL, process, &connect_socket);
		pthread_join(thread, NULL);
	}
	return 0;
}
// Accepts a null pointer because we used p_thread
void *process(void *p_connect_socket)
{
	int connect_socket = *((int *)p_connect_socket);
	char *req_head_tokens[3], msg_req[MAX_BUFF_SIZE], msg_resp[MAX_BUFF_SIZE], path[PATH_MAX];
	int message_size, write_fd, read_fd;

	memset(msg_req, '\0', MAX_BUFF_SIZE);
	read_fd = read(connect_socket, msg_req, MAX_BUFF_SIZE);
	char *ROOT = getenv("PWD");

	if (read_fd > 0)
	{
		printf("\n%s", msg_req);
		req_head_tokens[0] = strtok(msg_req, " ");
		if (strncmp(req_head_tokens[0], "GET", sizeof(req_head_tokens[0])) == 0)
		{
			// Example: GET / HTTP/1.1
			req_head_tokens[1] = strtok(NULL, " ");
			req_head_tokens[2] = strtok(NULL, " \n");
			if (strncmp(req_head_tokens[1], "/\0", 2) == 0)
				req_head_tokens[1] = "/index.html";

			if ((strncmp(req_head_tokens[2], "HTTP/1.1", 8) == 0) || (strncmp(req_head_tokens[2], "HTTP/1.0", 8) == 0))
			{
				strcpy(path, ROOT);
				strcpy(&path[strlen(ROOT)], req_head_tokens[1]);
				printf("Request file: %s\n", path);
				//O_RDONLY = READ ONLY

				if (fork() == 0)
				{
					if ((write_fd = open(path, O_RDONLY)) > 0)
					{
						send(connect_socket, "HTTP/1.1 200 OK\r\n\r\n", 19, 0);
						while ((message_size = read(write_fd, msg_resp, sizeof(msg_resp))) > 0)
						{
							write(connect_socket, msg_resp, message_size);
						}
					}
					else
					{
						write(connect_socket, "HTTP/1.1 404 Not Found\r\n", 24);
					}
				}
			}
			else
			{
				write(connect_socket, "HTTP/1.1 400 Bad Request\r\n", 26);
			}
		}
		else if (strncmp(req_head_tokens[0], "POST", sizeof(req_head_tokens[0])) == 0)
		{

			// Parses the POST request body data but only does the basic characters not ASCII Encoding Reference i.e. '%' is %25 or ' ' is %20
			// https://www.w3schools.com/tags/ref_urlencode.ASP
			char *pch;
			char *resp[4];
			pch = strtok(NULL, " \r\n");
			char lastLine[1024];
			while ( pch != NULL){
				strcpy(lastLine, pch);
				pch = strtok(NULL, " \r\n");
			}
			printf("%s", lastLine);
			resp[0] = strtok(lastLine, "=");
			resp[1] = strtok(NULL, "&");
			resp[2] = strtok(NULL, "=");
			resp[3] = strtok(NULL, "\n");

			char *text[2];
			text[1] = replace_char(resp[1], '+', ' ');
			printf("\nName: %s", text[1]);
			text[2] = replace_char(resp[3], '+', ' ');
			printf("\nComment: %s", resp[3]);
			// printf("\nComment: %s\n", resp[1]);

			// BUG: Due how to the design of our program, sometimes it prints twice

			strcpy(path, ROOT);
			strcpy(&path[strlen(ROOT)], "/reviews.txt");
			printf("\nAppended to: %s\n", path);

			FILE * fpointer = fopen(path, "a+");
			fprintf(fpointer, text[1]);
			fprintf(fpointer, "\n");
			fprintf(fpointer, text[2]);
			fprintf(fpointer, "\n");
			fprintf(fpointer, "-------------------------------------------------------");
			fprintf(fpointer, "\n");
			fclose(fpointer);

			// https://en.wikipedia.org/wiki/HTTP_301
			char postresp[] = "HTTP/1.1 301 Moved Permanently\nLocation: index.html\nContent-Type: text/html; charset=UTF-8\nContent-Length: 134\n\n!";
			send(connect_socket, postresp, sizeof(postresp), 0);
		}
	}
	// if(fork() != 0){
	close(connect_socket);
	printf("\nEnd of connection\n----------------------------------------------------------------------------------\n");
	// }

	// return NULL;
}

char* replace_char(char* str, char find, char replace){
    char *current_pos = strchr(str,find);
    while (current_pos){
        *current_pos = replace;
        current_pos = strchr(current_pos,find);
    }
    return str;
}
