#include "config.h"
#include "ini.h"
#include "err_handling.h"

#include <errno.h>
#include <limits.h>
#include <stdlib.h>
#include <string.h>

#define CONF_LOG_PATH		"log_path"
#define CONF_TARGET_PATH	"target_path"
#define CONF_LISTEN_PORT	"listen_port"

#ifndef DEFAULT_LISTEN_PORT
#define DEFAULT_LISTEN_PORT 21234
#endif

#define property_is(key) (!strncmp(name, key, strlen(key)))

// static unsigned short strtous(const char *str)
// {
// 	unsigned long l;
// 	unsigned short result;
// 	char *rest;

// 	errno = 0;
// 	l = strtoul(str, &rest, 0);

// 	if (errno == ERANGE) {
// 		perror("strtoul");
// 		printf("%s:%d - illegal long\n", __FILE__, __LINE__);
// 		errno = ERANGE;
// 		return USHRT_MAX;
// 	}

// 	result = (unsigned short) l;

// 	if ((l & ~((unsigned long) result))) {
// 		errno = ERANGE;
// 		perror("strtous");
// 		printf("%s:%d - illegal short\n", __FILE__, __LINE__);
// 		return USHRT_MAX;
// 	}
// }

static int config_prop_handler(
	void *user,
	const char *section,
	const char *name,
	const char *value)
{

	struct configuration *conf = user;

	if (strncmp(section, "", 1))
		return 0; /* Needed configuration is in default section */

	if (property_is(CONF_LOG_PATH))
		conf->log_path = strdup(value);
	else if (property_is(CONF_TARGET_PATH))
		conf->target_path = strdup(value);
	else if (property_is(CONF_LISTEN_PORT))
		conf->listen_port = strtoul(value, NULL, 10);
	else
		return 0;

	return 1;
}

#undef property_is

struct configuration *load_config(const char *path)
{
	struct configuration *conf = calloc(1, sizeof *conf);
	int rc;

	rc = ini_parse(path, config_prop_handler, conf);
	if (rc == -1)
		errno = ENOCONFIG;
	else if (rc == -2)
		errno = ENOMEM;
	else if (rc != 0)
		errno = EFAILINIREAD;

	// todo free allocated conf in erroneous case

	return (rc != 0) ? NULL : conf;
}

struct configuration *load_default_config()
{
	struct configuration *conf = calloc(1, sizeof *conf);
	const char *log_path = getenv(CONF_LOG_PATH);
	const char *target_path = getenv(CONF_TARGET_PATH);
	const char *listen_port = getenv(CONF_LISTEN_PORT);

	perror(log_path);
	perror(target_path);

	conf->log_path = (log_path == NULL) ? NULL : strdup(log_path);
	conf->target_path = (target_path == NULL) ? NULL : strdup(target_path);
	conf->listen_port = (listen_port == NULL)
				? DEFAULT_LISTEN_PORT
				: strtoul(listen_port, NULL, 10);

	return conf;
}

void free_config(struct configuration *conf)
{
	free(conf->log_path);
	free(conf->target_path);
}