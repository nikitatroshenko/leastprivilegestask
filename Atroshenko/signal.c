#include "signal.h"
#include "err_handling.h"

#include <signal.h>
#include <stdlib.h>

#define DEBUG 1

#if DEBUG

#include <stdio.h>

#define log(str) printf("%s:%d - %s\n", __FILE__, __LINE__, str)

#else

#define log(str)

#endif

#define LEASTPRIVELEGETASK_MAX_SUPPORTED_SIGNALS 10

struct signal_handler_queue_entry {
	signal_handler handler;
	void *data;
	struct signal_handler_queue_entry *next;
};

static struct signal_queue {
	int signal;
	struct signal_handler_queue_entry *first_entry;

} global_signal_table[LEASTPRIVELEGETASK_MAX_SUPPORTED_SIGNALS] = {};

static struct signal_handler_queue_entry *signal_handler_queue_entry_init()
{

	log("Alive");

	return calloc(1, sizeof (struct signal_handler_queue_entry *));
}

static struct signal_queue *get_for_signal(
		int signum,
		struct signal_queue *signal_table,
		size_t table_size)
{
	size_t i;

	log("Alive");

	for (i = 0; i < table_size && signal_table[i].signal != 0; i++)
		if (signal_table[i].signal == signum)
			break;

	log("Alive");
	printf("%s:%d - i=%lu\n", __FILE__, __LINE__, i);

	if (i == table_size) {
		errno = ENOMEM;
		return NULL;
	}

	return signal_table + i;
} 

static void execute_queue(int signum)
{
	struct signal_queue *queue = get_for_signal(
			signum,
			global_signal_table,
			LEASTPRIVELEGETASK_MAX_SUPPORTED_SIGNALS);
	struct signal_handler_queue_entry *curr = queue->first_entry;

	log("Alive");

	while (curr != NULL) {
		curr->handler(signum, curr->data);
		curr = curr->next;
	}
}

static void set_ansi_handler(int signum)
{
	signal(signum, execute_queue);
}

static void push_handler(
		int signum,
		struct signal_queue *signal_table,
		size_t table_size,
		struct signal_handler_queue_entry *entry)
{
	struct signal_queue *queue = get_for_signal(
			signum, signal_table, table_size);
	struct signal_handler_queue_entry *last = queue->first_entry;

	log("Alive");

	if (queue->first_entry == NULL) {
		queue->first_entry = entry;
		set_ansi_handler(signum);
		return;
	}

	while (last->next != NULL)
		last = last->next;

	log("Alive");

	last->next = entry;

}

void add_signal_handler(int signum, signal_handler signal_handler, void *data)
{
	int i;
	struct signal_handler_queue_entry *new_entry
			= signal_handler_queue_entry_init();

	printf("%s:%d - %s\n", __FILE__, __LINE__, "Alive");
	log("Alive");

	printf("Global Signal Table:\n");
	for (i = 0; i < LEASTPRIVELEGETASK_MAX_SUPPORTED_SIGNALS; i++) {
		printf("%d: %p\n", global_signal_table[i].signal,
			global_signal_table[i].first_entry);
	}

	new_entry->handler = signal_handler;
	new_entry->data = data;

	log("Alive");

	push_handler(
		signum,
		global_signal_table,
		LEASTPRIVELEGETASK_MAX_SUPPORTED_SIGNALS,
		new_entry);
}
