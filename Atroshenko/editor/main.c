#include "err_handling.h"

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

struct editor_args;

typedef int(*editor_func)(struct editor_args *);

struct editor_args {
	editor_func func;
	FILE *file;
	char *data;
	size_t data_len;
	int offset;
};

static int binary_append(struct editor_args *args);
static int binary_write(struct editor_args *args);
static int text_append(struct editor_args *args);
static int text_write(struct editor_args *args);

static int is_dec_char(char c)
{
	return (c >= '0') && (c <= '9');
}

static int is_hex_char(char c)
{
	if (is_dec_char(c))
		return 1;

	c |= 32; /* to lower case */
	return (c <= 'f') && (c >= 'a');
}

static int is_hex_string(const char *string)
{
	const char *p;

	for (p = string; *p != '\0'; p++) {
		if (!is_hex_char(*p))
			return 0;
	}

	return (p - string) % 2 == 0;
}

static int parse_arguments(int argc, char **argv, struct editor_args *args)
{
	char file_mode[3] = {};
	int is_binary = 0;

	if (argc < 4) {
		errno = EBADARGS;
		return -1;
	}

	if (strlen(argv[2]) != 2) {
		errno = EINVAL;
		return -1;
	}

	if (argv[2][1] == 'b') {
		file_mode[1] = 'b';
	} else if (argv[2][1] == 't') {
		file_mode[1] = 't';
	} else {
		errno = EINVAL;
		return -1;
	}
	if (argv[2][0] == 'a') {
		file_mode[0] = 'a';
		args->func = (file_mode[1] == 'b')
					? binary_append
					: text_append;
	} else if (argv[2][0] == 'w') {
		file_mode[0] = 'w';
		args->func = (file_mode[1] == 'b')
					? binary_write
					: text_write;
	} else {
		errno = EINVAL;
		return -1;
	}

	args->file = fopen(argv[1], file_mode);
	if (args->file == NULL) {
		return -1;
	}

	if ((file_mode[1] == 'b') && !is_hex_string(argv[3])) {
		errno = EINVAL;
		return -1;
	}
	args->data = strdup(argv[3]);
	args->data_len = strlen(argv[3]);

	if (!(file_mode[0] == 'a')) {
		char *arg_ptr = argv[4];

		if (argc < 5) {
			errno = EBADARGS;
			return -1;
		}

		errno = 0;
		args->offset = strtoul(argv[4], &arg_ptr, 10);
		if (errno)
			return -1;
		if (*arg_ptr != '\0') {
			errno = EINVAL;
			return -1;
		}
	}

	return 0;
}

/**
 * Usage:
 * editor <path> {a|w}{t|b} <data> [offset]
 *
 * if binary, data should be a hexadecimal string with even number of digits
 * if append, offset is ignored, else required
 */
int main(int argc, char **argv)
{
	struct editor_args args;

	if (parse_arguments(argc, argv, &args) == -1) {
		int errsv = errno;

		log_error();

		if (errsv == EACCES) {
			printf("EACCES\n");
		}
		return errsv;
	}

	args.func(&args);

	return 0;
}

static char to_byte(char two_hexes[2])
{
	char result = 0;

	if (is_dec_char(two_hexes[0]))
		result |= (two_hexes[0] - '0') << 4;
	else 
		result |= ((two_hexes[0] | 32) - 'a' + 10) << 4;
	if (is_dec_char(two_hexes[1]))
		result |= two_hexes[1] - '0';
	else 
		result |= (two_hexes[1] | 32) - 'a' + 10;

	return result;
}

void prepare_binary_data(struct editor_args *args)
{
	size_t i;
	size_t bin_data_len = args->data_len / 2;
	char *bin_data = calloc(bin_data_len, sizeof *bin_data);

	for (i = 0; i < bin_data_len; i++) {
		bin_data[i] = to_byte(args->data + i * 2);
	}

	free(args->data);
	args->data = bin_data;
	args->data_len = bin_data_len;
}

int binary_append(struct editor_args *args)
{
	printf("binary_append\n");
	return 0;
}

int binary_write(struct editor_args *args)
{
	printf("binary_write\n");
	return 0;
}

int text_append(struct editor_args *args)
{
	printf("text_append\n");
	return 0;
}

int text_write(struct editor_args *args)
{
	printf("text_write\n");
	return 0;
}
