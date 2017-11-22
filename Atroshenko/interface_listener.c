#include "interface_listener.h"
#include "err_handling.h"
#include "signal.h"

#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#ifndef _INTERFACE_LISTENER_BACKLOG
#define _INTERFACE_LISTENER_BACKLOG 5
#endif

static void create_server_activity(
		int cli_sfd,
		const struct listener_startup_info *info);

static void *listen_routine(
		int serv_sfd,
		const struct listener_startup_info *startup_info);

static void sigint_handler(int signum, void *data)
{
	int serv_sfd = *((int *) data);

	printf("Closing server socket\n");
	close(serv_sfd);
}

void start_listen_connections(const struct listener_startup_info *startup_info)
{
	int serv_sfd;
	struct sockaddr_in serv_addr;

	serv_sfd = socket(AF_INET, SOCK_STREAM, 0);

	if (serv_sfd == -1) {
		perror("socket");
		exit(-1);
	}

	memset(&serv_addr, 0, sizeof(serv_addr));
	serv_addr.sin_family = AF_INET;
	serv_addr.sin_port = htons(startup_info->port_number);
	serv_addr.sin_addr.s_addr = INADDR_ANY;

	if (bind(serv_sfd, (struct sockaddr *) &serv_addr,
			sizeof(serv_addr)) == -1) {
		perror("bind");
		exit(-1);
	}

	if (-1 == listen(serv_sfd, _INTERFACE_LISTENER_BACKLOG)) {
		perror("listen");
		exit(-1);
	}

	printf("Listening port %d\n", startup_info->port_number);
	listen_routine(serv_sfd, startup_info);
}

void *listen_routine(
		int serv_sfd,
		const struct listener_startup_info *startup_info)
{
	int cli_sfd;
	socklen_t cli_addr_len;
	struct sockaddr_in cli_addr;

	add_signal_handler(SIGINT, sigint_handler, &serv_sfd);

	while (1) {
		cli_addr_len = sizeof(struct sockaddr_in);
		cli_sfd = accept(serv_sfd, (struct sockaddr *) &cli_addr,
				&cli_addr_len);

		if (cli_sfd == -1) {
			perror("accept");
			break;
		}
		
		fprintf(stderr, "Client connected!\n");
		create_server_activity(
				cli_sfd,
				startup_info);
	}

	return NULL;
}

struct connection_handler_args {
	struct interface_listener_ctx *ctx;
	server_action action;
};

void *connection_handler_start_routine(void *arg)
{
	struct connection_handler_args *args
			= (struct connection_handler_args *) arg;
	struct interface_listener_ctx *iface_ctx = args->ctx;
	int cli_sfd = iface_ctx->cli_sfd;

	printf("Before action\n");	
	args->action(iface_ctx);
	printf("After action\n");

	/* free resources allocated after trhead start */
	printf("Closing client fd\n");
	close(iface_ctx->cli_sfd);
	free(iface_ctx);
	free(args);

	return NULL;
}

void create_server_activity(
		int cli_sfd,
		const struct listener_startup_info *info)
{
	pthread_t tid;
	struct connection_handler_args *args = calloc(1, sizeof *args);
	struct interface_listener_ctx *iface_ctx = calloc(1, sizeof *iface_ctx);

	args->ctx = iface_ctx;
	args->action = info->action;
	iface_ctx->action_ctx = info->action_ctx;
	iface_ctx->cli_sfd = cli_sfd;

	pthread_create(&tid, NULL, connection_handler_start_routine, args);
}
