#include "err_handling.h"

#include <limits.h>
#include <netdb.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef PATH_MAX
#define PATH_MAX 4096
#endif

#ifndef _EDITOR_EXECUTABLE
#define _EDITOR_EXECUTABLE "/usr/bin/atroshenko_editor"
#endif

struct interface_args {
	unsigned int portno;
	char *host;
};

static int parse_arguments(int argc, char **argv, struct interface_args *args);
static int connect_by_args(struct interface_args *args);

struct editor_args {
	char *path;
	int is_binary : 1;
	int is_append : 1;
	char *data;
	size_t offset;
};

static int call_editor(struct editor_args *args, int force);

int main(int argc, char **argv)
{
	struct interface_args args;
	struct editor_args editor_args;
	int mysfd;
	char single_char_choice;

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

	memset(&editor_args, 0, sizeof editor_args);

	printf("Enter filename:\n");
	scanf("%ms", &editor_args.path);
	printf("Append?\n");
	scanf("%1s", &single_char_choice);
	if (single_char_choice == 'y') {
		editor_args.is_append = 1;
	}
	printf("Binary?\n");
	scanf("%1s", &single_char_choice);
	if (single_char_choice == 'y') {
		editor_args.is_binary = 1;
	}
	printf("Data:\n");
	scanf("%ms", &editor_args.data);
	printf("Offset:\n");
	scanf("%lu", &editor_args.offset);

	if (call_editor(&editor_args, 0)) {
		if ((errno == EACCES) && call_editor(&editor_args, 1)) {
			if (errno == EACCES)
				printf("No permission\n");
		}
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

#if unix || __unix || __unix__

int call_editor(struct editor_args *args, int force)
{
	char *command_line = calloc(PATH_MAX, sizeof *command_line);
	int errsv;

	if (snprintf(command_line, PATH_MAX, "%s '%s' '%s' '%c%c' '%s' %lu",
			force ? "sudo" : "",
			_EDITOR_EXECUTABLE,
			args->path,
			args->is_append ? 'a' : 'w',
			args->is_binary ? 'b' : 't',
			args->data,
			args->offset) >= PATH_MAX) {

		errno = ENAMETOOLONG;
		return -1;
	}

	errno = system(command_line);
	errsv = errno;
	perror("call_editor");
	errno = errsv;
	return errno ? -1 : 0;
}

#else
#error implemented for UNIX systems only
#endif