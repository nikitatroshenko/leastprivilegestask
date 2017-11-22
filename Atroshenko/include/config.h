#ifndef SERVICETASK_CONFIG
#define SERVICETASK_CONFIG

struct configuration {
	unsigned int listen_port;
	char *target_path;
	char *log_path;
};

struct configuration *load_default_config();

struct configuration *load_config(const char *path);

void free_config(struct configuration *conf);

#endif