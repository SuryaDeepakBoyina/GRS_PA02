/*
 * MT25048_Part_A_common.c - Implementation of common utilities for PA02 Network
 * I/O Analysis Roll Number: MT25048
 */

#include "MT25048_Part_A_common.h"

/* Global shutdown flag */
volatile sig_atomic_t shutdown_flag = 0;

/* Signal handler for clean shutdown */
static void signal_handler(int signum) {
  (void)signum; /* Suppress unused parameter warning */
  shutdown_flag = 1;
}

/* Setup signal handlers for SIGINT and SIGTERM */
void setup_signal_handlers(void) {
  struct sigaction sa;
  memset(&sa, 0, sizeof(sa));
  sa.sa_handler = signal_handler;
  sigemptyset(&sa.sa_mask);
  sa.sa_flags = 0;

  sigaction(SIGINT, &sa, NULL);
  sigaction(SIGTERM, &sa, NULL);
}

/*
 * msg_alloc - Allocate a message structure with 8 heap-allocated fields
 *
 * Each field is malloc'd separately as required by the assignment spec.
 * This ensures proper two-copy behavior in the baseline implementation.
 */
message_t *msg_alloc(size_t field_size) {
  message_t *msg = (message_t *)malloc(sizeof(message_t));
  if (!msg) {
    perror("malloc message_t failed");
    return NULL;
  }

  msg->field_size = field_size;

  /* Allocate each field separately on the heap */
  for (int i = 0; i < NUM_FIELDS; i++) {
    msg->field[i] = (char *)malloc(field_size);
    if (!msg->field[i]) {
      perror("malloc field failed");
      /* Free previously allocated fields */
      for (int j = 0; j < i; j++) {
        free(msg->field[j]);
      }
      free(msg);
      return NULL;
    }
    /* Initialize with pattern data */
    snprintf(msg->field[i], field_size, "Field%d_Data_%zu", i, field_size);
  }

  return msg;
}

/*
 * msg_free - Free a message structure and all its fields
 */
void msg_free(message_t *msg) {
  if (!msg)
    return;

  for (int i = 0; i < NUM_FIELDS; i++) {
    free(msg->field[i]);
  }
  free(msg);
}

/*
 * msg_serialize - Serialize message to buffer for sending
 *
 * Format: [field_size (8 bytes)] [field0] [field1] ... [field7]
 */
int msg_serialize(const message_t *msg, char *buffer, size_t buffer_size) {
  size_t total_size = sizeof(size_t) + (NUM_FIELDS * msg->field_size);

  if (buffer_size < total_size) {
    fprintf(stderr, "Buffer too small for serialization\n");
    return -1;
  }

  /* Write field size */
  memcpy(buffer, &msg->field_size, sizeof(size_t));
  size_t offset = sizeof(size_t);

  /* Write each field */
  for (int i = 0; i < NUM_FIELDS; i++) {
    memcpy(buffer + offset, msg->field[i], msg->field_size);
    offset += msg->field_size;
  }

  return total_size;
}

/*
 * msg_deserialize - Deserialize buffer to message structure
 */
int msg_deserialize(message_t *msg, const char *buffer, size_t buffer_size) {
  if (buffer_size < sizeof(size_t)) {
    fprintf(stderr, "Buffer too small for deserialization\n");
    return -1;
  }

  /* Read field size */
  size_t field_size;
  memcpy(&field_size, buffer, sizeof(size_t));
  size_t total_size = sizeof(size_t) + (NUM_FIELDS * field_size);

  if (buffer_size < total_size) {
    fprintf(stderr, "Buffer incomplete for deserialization\n");
    return -1;
  }

  msg->field_size = field_size;
  size_t offset = sizeof(size_t);

  /* Allocate and read each field */
  for (int i = 0; i < NUM_FIELDS; i++) {
    msg->field[i] = (char *)malloc(field_size);
    if (!msg->field[i]) {
      perror("malloc field failed in deserialize");
      /* Free previously allocated */
      for (int j = 0; j < i; j++) {
        free(msg->field[j]);
      }
      return -1;
    }
    memcpy(msg->field[i], buffer + offset, field_size);
    offset += field_size;
  }

  return total_size;
}

/*
 * get_timestamp_us - Get current timestamp in microseconds
 */
double get_timestamp_us(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec * 1000000.0 + (double)ts.tv_nsec / 1000.0;
}

/*
 * get_timestamp_sec - Get current timestamp in seconds
 */
double get_timestamp_sec(void) {
  struct timespec ts;
  clock_gettime(CLOCK_MONOTONIC, &ts);
  return (double)ts.tv_sec + (double)ts.tv_nsec / 1000000000.0;
}

/*
 * calculate_throughput_gbps - Calculate throughput in Gbps
 */
double calculate_throughput_gbps(size_t bytes_transferred,
                                 double duration_sec) {
  if (duration_sec <= 0)
    return 0.0;
  double bits = (double)bytes_transferred * 8.0;
  return bits / (duration_sec * 1000000000.0); /* Gbps */
}

/*
 * parse_int_arg - Parse integer command-line argument
 */
int parse_int_arg(const char *arg, const char *prefix, int default_value) {
  if (strncmp(arg, prefix, strlen(prefix)) == 0) {
    return atoi(arg + strlen(prefix));
  }
  return default_value;
}

/*
 * parse_string_arg - Parse string command-line argument
 */
const char *parse_string_arg(const char *arg, const char *prefix,
                             const char *default_value) {
  if (strncmp(arg, prefix, strlen(prefix)) == 0) {
    return arg + strlen(prefix);
  }
  return default_value;
}

/*
 * stats_init - Initialize statistics structure
 */
void stats_init(stats_t *stats) {
  memset(stats, 0, sizeof(stats_t));
  pthread_mutex_init(&stats->lock, NULL);
  stats->start_time_sec = get_timestamp_sec();
}

/*
 * stats_update - Update statistics with new measurement
 */
void stats_update(stats_t *stats, size_t bytes, double latency_us) {
  pthread_mutex_lock(&stats->lock);
  stats->total_bytes += bytes;
  stats->total_messages++;
  stats->total_latency_us += latency_us;
  pthread_mutex_unlock(&stats->lock);
}

/*
 * stats_print - Print statistics summary
 */
void stats_print(stats_t *stats, const char *label) {
  pthread_mutex_lock(&stats->lock);
  stats->end_time_sec = get_timestamp_sec();

  double duration_sec = stats->end_time_sec - stats->start_time_sec;
  double throughput_gbps =
      calculate_throughput_gbps(stats->total_bytes, duration_sec);
  double avg_latency_us =
      (stats->total_messages > 0)
          ? (stats->total_latency_us / stats->total_messages)
          : 0.0;

  printf("\n=== %s Statistics ===\n", label);
  printf("Duration: %.2f seconds\n", duration_sec);
  printf("Total Bytes: %zu\n", stats->total_bytes);
  printf("Total Messages: %zu\n", stats->total_messages);
  printf("Throughput: %.3f Gbps\n", throughput_gbps);
  printf("Average Latency: %.2f µs\n", avg_latency_us);
  printf("Messages/sec: %.2f\n", stats->total_messages / duration_sec);

  pthread_mutex_unlock(&stats->lock);
}

/*
 * stats_destroy - Clean up statistics structure
 */
void stats_destroy(stats_t *stats) { pthread_mutex_destroy(&stats->lock); }
