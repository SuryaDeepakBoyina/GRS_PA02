/*
 * MT25048_Part_A1_Client.c - Two-Copy TCP Client (Baseline)
 * Roll Number: MT25048
 *
 * This client SENDS data continuously to the server using send() (two-copy).
 * Multiple threads can be spawned to test concurrent connections.
 *
 * TWO-COPY EXPLANATION:
 * ---------------------
 * Copy 1: User space buffer → Kernel socket buffer (via send() syscall)
 * Copy 2: Kernel socket buffer → NIC hardware buffer (via DMA/network stack)
 *
 * Measures throughput (Gbps) and latency (µs) for performance analysis.
 */

#include "MT25048_Part_A_common.h"
#include <netinet/tcp.h>

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 9000
#define DEFAULT_MSGSIZE 1024
#define DEFAULT_DURATION 10
#define DEFAULT_THREADS 1
#define SEND_BUFFER_SIZE (256 * 1024) /* 256 KB */

typedef struct {
  const char *host;
  int port;
  size_t msg_size;
  int duration_sec;
  int thread_id;
  stats_t *global_stats;
} client_thread_args_t;

/*
 * sender_thread - Thread function to send messages to server
 */
void *sender_thread(void *arg) {
  client_thread_args_t *args = (client_thread_args_t *)arg;
  const char *host = args->host;
  int port = args->port;
  size_t msg_size = args->msg_size;
  int duration_sec = args->duration_sec;
  int thread_id = args->thread_id;

  printf("[Thread %d] Connecting to %s:%d\n", thread_id, host, port);

  /* Create socket */
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    perror("socket failed");
    return NULL;
  }

  /* Configure send buffer */
  int sendbuf = SEND_BUFFER_SIZE;
  setsockopt(sock_fd, SOL_SOCKET, SO_SNDBUF, &sendbuf, sizeof(sendbuf));

  /* Connect to server */
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_port = htons(port);

  if (inet_pton(AF_INET, host, &server_addr.sin_addr) <= 0) {
    fprintf(stderr, "[Thread %d] Invalid host address\n", thread_id);
    close(sock_fd);
    return NULL;
  }

  if (connect(sock_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("connect failed");
    close(sock_fd);
    return NULL;
  }

  /* Disable Nagle's algorithm */
  int nodelay = 1;
  setsockopt(sock_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

  printf("[Thread %d] Connected to server\n", thread_id);

  /* Allocate message with 8 heap-allocated fields */
  message_t *msg = msg_alloc(msg_size / NUM_FIELDS);
  if (!msg) {
    fprintf(stderr, "[Thread %d] Failed to allocate message\n", thread_id);
    close(sock_fd);
    return NULL;
  }

  /* Allocate send buffer */
  size_t buffer_size = sizeof(size_t) + msg_size;
  char *send_buffer = (char *)malloc(buffer_size);
  if (!send_buffer) {
    perror("malloc send_buffer failed");
    msg_free(msg);
    close(sock_fd);
    return NULL;
  }

  /* Serialize message once */
  int serialized_size = msg_serialize(msg, send_buffer, buffer_size);
  if (serialized_size < 0) {
    fprintf(stderr, "[Thread %d] Serialization failed\n", thread_id);
    free(send_buffer);
    msg_free(msg);
    close(sock_fd);
    return NULL;
  }

  /* Local statistics */
  stats_t local_stats;
  stats_init(&local_stats);

  double start_time = get_timestamp_sec();
  double end_time = start_time + duration_sec;

  /* Send loop */
  while (get_timestamp_sec() < end_time && !shutdown_flag) {
    double send_start = get_timestamp_us();

    /* TWO-COPY SEND: Data copied from user buffer to kernel socket buffer */
    ssize_t sent = send(sock_fd, send_buffer, serialized_size, 0);

    double send_end = get_timestamp_us();

    if (sent < 0) {
      if (errno == EINTR || errno == EAGAIN)
        continue;
      perror("send failed");
      break;
    }

    if (sent != serialized_size) {
      fprintf(stderr, "[Thread %d] Partial send: %zd/%d\n", thread_id, sent,
              serialized_size);
      break;
    }

    stats_update(&local_stats, serialized_size, send_end - send_start);
  }

  /* Update global statistics */
  pthread_mutex_lock(&args->global_stats->lock);
  args->global_stats->total_bytes += local_stats.total_bytes;
  args->global_stats->total_messages += local_stats.total_messages;
  args->global_stats->total_latency_us += local_stats.total_latency_us;
  pthread_mutex_unlock(&args->global_stats->lock);

  stats_print(&local_stats, "Client Thread (TWO-COPY)");
  stats_destroy(&local_stats);

  free(send_buffer);
  msg_free(msg);
  close(sock_fd);

  printf("[Thread %d] Disconnected\n", thread_id);
  return NULL;
}

int main(int argc, char *argv[]) {
  const char *host = DEFAULT_HOST;
  int port = DEFAULT_PORT;
  size_t msg_size = DEFAULT_MSGSIZE;
  int duration = DEFAULT_DURATION;
  int num_threads = DEFAULT_THREADS;

  /* Parse command-line arguments */
  for (int i = 1; i < argc; i++) {
    host = parse_string_arg(argv[i], "--host=", host);
    port = parse_int_arg(argv[i], "--port=", port);
    msg_size = (size_t)parse_int_arg(argv[i], "--msgsize=", (int)msg_size);
    duration = parse_int_arg(argv[i], "--duration=", duration);
    num_threads = parse_int_arg(argv[i], "--threads=", num_threads);
  }

  printf("=== MT25048 Part A1: Two-Copy Client (Sender) ===\n");
  printf("Server: %s:%d\n", host, port);
  printf("Message Size: %zu bytes\n", msg_size);
  printf("Duration: %d seconds\n", duration);
  printf("Threads: %d\n\n", num_threads);

  setup_signal_handlers();

  /* Global statistics */
  stats_t global_stats;
  stats_init(&global_stats);

  /* Create client threads */
  pthread_t *threads = (pthread_t *)malloc(num_threads * sizeof(pthread_t));
  client_thread_args_t *thread_args = (client_thread_args_t *)malloc(
      num_threads * sizeof(client_thread_args_t));

  if (!threads || !thread_args) {
    perror("malloc failed");
    return 1;
  }

  /* Spawn threads */
  for (int i = 0; i < num_threads; i++) {
    thread_args[i].host = host;
    thread_args[i].port = port;
    thread_args[i].msg_size = msg_size;
    thread_args[i].duration_sec = duration;
    thread_args[i].thread_id = i;
    thread_args[i].global_stats = &global_stats;

    if (pthread_create(&threads[i], NULL, sender_thread, &thread_args[i]) !=
        0) {
      perror("pthread_create failed");
      num_threads = i; /* Adjust to actual number created */
      break;
    }
  }

  /* Wait for all threads to complete */
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }

  /* Print aggregate statistics */
  stats_print(&global_stats, "Overall Client (TWO-COPY)");
  stats_destroy(&global_stats);

  free(threads);
  free(thread_args);

  printf("\nClient finished\n");
  return 0;
}
