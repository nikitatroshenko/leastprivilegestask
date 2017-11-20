#ifndef INTERFACE_LISTENER
#define INTERFACE_LISTENER 

struct interface_listener_ctx {
	void *action_ctx;
	int cli_sfd;
};

typedef void(*server_action)(struct interface_listener_ctx *);

struct listener_startup_info {
	short port_number;
	server_action action;
	void *action_ctx;
};

void start_listen_connections(const struct listener_startup_info *startup_info);

#endif /* INTERFACE_LISTENER */