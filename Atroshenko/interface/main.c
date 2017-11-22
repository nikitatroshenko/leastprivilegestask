#include "err_handling.h"

#include <limits.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

struct interface_args {
	unsigned int portno;
	char *host;
};

int parse_arguments(int argc, char **argv, struct interface_args *args)
{
	char *port_arg_ptr;

	if (argc < 3) {
		errno = EBADARGS;
		return -1;
	}

	args->host = strdup(argv[1]);

	errno = 0;
	port_arg_ptr = argv[2];
	args->portno = strtoul(argv[2], &port_arg_ptr, 10);

	/* if invalid number passed */
	if (*port_arg_ptr != '\0' || errno == EINVAL) {
		errno = EINVAL;
		return -1;
	}

	if (args->portno > USHRT_MAX || errno == ERANGE) {
		errno = ERANGE;
		return -1;
	}

	return 0;
}

int connect_by_args(struct interface_args *args)
{
	int mysfd;
	struct sockaddr_in serv_addr;
	struct hostent *server;


	server = gethostbyname(args->host);
	if (server == NULL)
		return -1;

	mysfd = socket(AF_INET, SOCK_STREAM, 0);
	if (mysfd == -1)
		return -1;

	memset(&serv_addr, 0, sizeof serv_addr);
	serv_addr.sin_family = AF_INET;
	memcpy(&serv_addr.sin_addr, server->h_addr, server->h_length);
	serv_addr.sin_port = htons(args->portno);

	if (connect(mysfd, (struct sockaddr *) &serv_addr,
			sizeof serv_addr) == -1) {
		int errnum = errno;

		close(mysfd);
		errno = errnum;
		return -1;
	}

	return mysfd;
}

int main(int argc, char const **argv)
{
	struct interface_args args;
	int mysfd;

	if (parse_arguments(argc, argv, &args)) {
		log_error();
		exit(-1);
	}

	mysfd = connect_by_args(&args);
	if (mysfd == -1) {
		log_error();
		exit(-1);
	}

	write(mysfd, "hello", strlen("hello"));
	close(mysfd);
	
	return 0;
}