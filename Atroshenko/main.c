#include "config.h"
#include "daemonize.h"
#include "err_handling.h"
#include "interface_listener.h"
#include "listen_changes.h"
#include "signal.h"

#include <stdio.h>
#include <stdlib.h>
#include <openssl/md5.h>
#include <string.h>
#include <time.h>

#define DEFAULT_FILE_BUF_SIZE 512

#define CONFIG_ARG "--config"

struct file_change_handling_data {
	FILE *log;
	FILE *target;
};

static struct configuration *resolve_config(int argc, char **argv);

static void log_record(FILE *out, const char *record);
static void file_change_handling_routine(void *data);
static void iface_listener_routine(struct interface_listener_ctx *iface_ctx);

int main(int argc, char **argv)
{
	struct file_change_handling_data routine_data;
	struct listen_ctx *listen_ctx;
	struct listener_startup_info if_startup_info;
	struct configuration *conf = NULL;

	go_background();

	conf = resolve_config(argc, argv);

	routine_data.log = fopen(conf->log_path, "at");
	routine_data.target = fopen(conf->target_path, "rt");

	/* log status when work started */
	file_change_handling_routine(&routine_data);

	if (routine_data.log == NULL || routine_data.target == NULL) {
		fprintf(stderr, "Config data null\n");
		log_error();
		return errno;
	}

	fprintf(stderr, "Started listening file changes\n");

	listen_ctx = start_listen_changes(
			conf->target_path,
			file_change_handling_routine,
			&routine_data);

	if_startup_info.port_number = conf->listen_port;
	if_startup_info.action = iface_listener_routine;
	if_startup_info.action_ctx = (void *) 0x00000042;

	start_listen_connections(&if_startup_info);

	fprintf(stderr, "Stopping listen\n");
	stop_listen_changes(listen_ctx);
	free_config(conf);

	fprintf(stderr, "Exiting\n");

	return 0;
}

struct configuration *resolve_config(int argc, char **argv)
{
	size_t i;
	size_t config_arg = argc;
	struct configuration *conf;

	for (i = 0; i < argc; i++) {
		fprintf(stderr, "Argument '%s'\n", argv[i]);
		if (!strncmp(CONFIG_ARG, argv[i], strlen(CONFIG_ARG)))
			config_arg = i + 1;
	}

	fprintf(stderr, "Config arg: %lu\n", config_arg);

	if (config_arg < argc) {
		fprintf(stderr, "loading config %s\n", argv[config_arg]);
		conf = load_config(argv[config_arg]);	
	} else {
		fprintf(stderr, "loading default config\n");
		conf = load_default_config();
	}

	if (conf == NULL || conf->log_path == NULL
			|| conf->target_path == NULL) {

		log_error();
		fprintf(stderr, "Failed to load configuration\n");
		exit(errno);
	}

	fprintf(stderr, "Loaded\n");
	fprintf(stderr,
		"Read config:\n"
		"\tlog_path=%s\n"
		"\ttarget_path=%s\n"
		"\tlisten_port=%u\n",
		conf->log_path, conf->target_path, conf->listen_port);

	return conf;
}

int get_file_md5(FILE *in, unsigned char *digest)
{
	MD5_CTX context;
	size_t bytes;
	char buf[DEFAULT_FILE_BUF_SIZE];
	int success_call;

	success_call = MD5_Init(&context);

	if (!success_call)
		return 0;

	bytes = fread(buf, sizeof *buf, DEFAULT_FILE_BUF_SIZE, in);

	while (bytes > 0) {
		success_call = MD5_Update(&context, buf, bytes);

		if (!success_call) 
			return 0;

		bytes = fread(buf, sizeof *buf, DEFAULT_FILE_BUF_SIZE, in);
	}

	success_call = MD5_Final(digest, &context);

	return success_call;
}

void bytes_to_hex_str(unsigned char *bytes, size_t len, char *buf)
{
	size_t i;

	for (i = 0; i < len; i++) {
		sprintf(buf, "%02x", bytes[i]);
		buf += 2;
	}
}

void log_record(FILE *out, const char *record)
{
	time_t rawtime;
	struct tm *tm;

	time(&rawtime);
	tm = localtime(&rawtime);

	fprintf(out, "[%4d.%02d.%02d %02d:%02d:%02d] %s\n",
			tm->tm_year + 1900,
			tm->tm_mon + 1,
			tm->tm_mday,
			tm->tm_hour,
			tm->tm_min,
			tm->tm_sec,
			record);
}

void file_change_handling_routine(void *data)
{
	struct file_change_handling_data *routine_data = data;
	unsigned char digest[MD5_DIGEST_LENGTH];
	char digest_str[2 * MD5_DIGEST_LENGTH + 1];

	if (!get_file_md5(routine_data->target, digest))
		log_error();

	bytes_to_hex_str(digest, MD5_DIGEST_LENGTH, digest_str);
	log_record(routine_data->log, digest_str);

	fflush(routine_data->log);
}

void iface_listener_routine(struct interface_listener_ctx *iface_ctx)
{
	printf("Hello! new connection %p\n", iface_ctx->action_ctx);
}