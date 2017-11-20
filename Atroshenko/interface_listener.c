#include <pthread.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include "interface_listener.h"
#include "err_handling.h"

#ifndef _INTERFACE_LISTENER_BACKLOG
#define _INTERFACE_LISTENER_BACKLOG 5
#endif

static void create_server_activity(
		int cli_sfd,
		const struct listener_startup_info *info);

static void *listen_routine(
		int serv_sfd,
		const struct listener_startup_info *startup_info);

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

	while (1) {
		cli_addr_len = sizeof(struct sockaddr_in);
		cli_sfd = accept(serv_sfd, (struct sockaddr *) &cli_addr,
				&cli_addr_len);
		if (cli_sfd == -1) {
			perror("accept");
			return NULL;
		}
		
		fprintf(stderr, "CLient connected!\n");
		create_server_activity(
				cli_sfd,
				startup_info);
		/* do_server_activity(cli_sfd); */
	}
}

void *connection_handler_start_routine(void *arg)
{
	struct interface_listener_ctx iface_ctx =
			*(struct interface_listener_ctx *) arg;


	
	close(iface_ctx.cli_sfd);

	return NULL;
}

void create_server_activity(
		int cli_sfd,
		const struct listener_startup_info *info)
{
	pthread_t tid;
	struct interface_listener_ctx iface_ctx;

	iface_ctx.action_ctx = info->action_ctx;
	iface_ctx.cli_sfd = cli_sfd;

	pthread_create(&tid, NULL, connection_handler_start_routine,
			NULL);
}
