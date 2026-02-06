/*
 * common.h - Common definitions and utilities for PA02 Network I/O Analysis
 * Roll Number: MT25048
 *
 * This header defines the message structure with 8 heap-allocated string fields
 * and utility functions for timing, allocation, and throughput calculation.
 */

#ifndef COMMON_H
#define COMMON_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <errno.h>
#include <pthread.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <signal.h>

/* Message structure with 8 dynamically allocated string fields */
#define NUM_FIELDS 8

typedef struct {
    char *field[NUM_FIELDS];  /* 8 heap-allocated string pointers */
    size_t field_size;         /* Size of each field in bytes */
} message_t;

/* Function declarations */

/* Message allocation and deallocation */
message_t* msg_alloc(size_t field_size);
void msg_free(message_t *msg);
int msg_serialize(const message_t *msg, char *buffer, size_t buffer_size);
int msg_deserialize(message_t *msg, const char *buffer, size_t buffer_size);

/* Timing utilities */
double get_timestamp_us(void);  /* Returns current time in microseconds */
double get_timestamp_sec(void); /* Returns current time in seconds */

/* Throughput calculation */
double calculate_throughput_gbps(size_t bytes_transferred, double duration_sec);

/* Command-line argument parsing helpers */
int parse_int_arg(const char *arg, const char *prefix, int default_value);
const char* parse_string_arg(const char *arg, const char *prefix, const char *default_value);

/* Statistics tracking */
typedef struct {
    size_t total_bytes;
    size_t total_messages;
    double total_latency_us;
    double start_time_sec;
    double end_time_sec;
    pthread_mutex_t lock;
} stats_t;

void stats_init(stats_t *stats);
void stats_update(stats_t *stats, size_t bytes, double latency_us);
void stats_print(stats_t *stats, const char *label);
void stats_destroy(stats_t *stats);

/* Global shutdown flag for clean termination */
extern volatile sig_atomic_t shutdown_flag;
void setup_signal_handlers(void);

#endif /* COMMON_H */
