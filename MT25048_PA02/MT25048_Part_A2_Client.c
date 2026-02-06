/*
 * MT25048_Part_A2_Client.c - One-Copy TCP Client (sendmsg optimization)
 * Roll Number: MT25048
 *
 * This client SENDS data continuously using sendmsg() with pre-registered
 * buffers and scatter-gather I/O via iovec structures.
 *
 * ONE-COPY EXPLANATION:
 * ---------------------
 * Copy Eliminated: User space → Kernel space copy is REDUCED/OPTIMIZED
 *
 * How it works:
 * 1. Use posix_memalign() to create page-aligned buffers
 * 2. Use sendmsg() with iovec structures for scatter-gather I/O
 * 3. Kernel can reference user buffers directly or use optimized copy path
 */

#include "common.h"
#include <netinet/tcp.h>
#include <sys/uio.h>

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 9001 /* Matches A2 server */
#define DEFAULT_MSGSIZE 1024
#define DEFAULT_DURATION 10
#define DEFAULT_THREADS 1
#define PAGE_SIZE 4096

typedef struct {
  const char *host;
  int port;
  size_t msg_size;
  int duration_sec;
  int thread_id;
  stats_t *global_stats;
} client_thread_args_t;

/*
 * sender_thread - Thread function using sendmsg() for one-copy transmission
 */
void *sender_thread(void *arg) {
  client_thread_args_t *args = (client_thread_args_t *)arg;
  const char *host = args->host;
  int port = args->port;
  size_t msg_size = args->msg_size;
  int duration_sec = args->duration_sec;
  int thread_id = args->thread_id;

  printf("[Thread %d] Connecting to %s:%d (ONE-COPY mode)\n", thread_id, host,
         port);

  /* Create socket */
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    perror("socket failed");
    return NULL;
  }

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

  /* Allocate PAGE-ALIGNED buffer for DMA optimization */
  void *aligned_buffer;
  size_t buffer_size = sizeof(size_t) + msg_size;
  size_t aligned_size = ((buffer_size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

  if (posix_memalign(&aligned_buffer, PAGE_SIZE, aligned_size) != 0) {
    perror("posix_memalign failed");
    close(sock_fd);
    return NULL;
  }

  printf("[Thread %d] Allocated %zu-byte aligned buffer at %p\n", thread_id,
         aligned_size, aligned_buffer);

  /* Create message in aligned buffer */
  message_t *msg = msg_alloc(msg_size / NUM_FIELDS);
  if (!msg) {
    fprintf(stderr, "[Thread %d] Failed to allocate message\n", thread_id);
    free(aligned_buffer);
    close(sock_fd);
    return NULL;
  }

  /* Serialize into aligned buffer */
  int serialized_size =
      msg_serialize(msg, (char *)aligned_buffer, aligned_size);
  if (serialized_size < 0) {
    fprintf(stderr, "[Thread %d] Serialization failed\n", thread_id);
    msg_free(msg);
    free(aligned_buffer);
    close(sock_fd);
    return NULL;
  }

  /* Prepare iovec structure for scatter-gather I/O */
  struct iovec iov[1];
  iov[0].iov_base = aligned_buffer;
  iov[0].iov_len = serialized_size;

  struct msghdr msghdr;
  memset(&msghdr, 0, sizeof(msghdr));
  msghdr.msg_iov = iov;
  msghdr.msg_iovlen = 1;

  stats_t local_stats;
  stats_init(&local_stats);

  double start_time = get_timestamp_sec();
  double end_time = start_time + duration_sec;

  /* Send loop using sendmsg() - ONE-COPY path */
  while (get_timestamp_sec() < end_time && !shutdown_flag) {
    double send_start = get_timestamp_us();

    /*
     * ONE-COPY SENDMSG:
     * Kernel can reference aligned_buffer directly or use optimized copy path.
     * The iovec structure allows scatter-gather DMA, reducing intermediate
     * copies.
     */
    ssize_t sent = sendmsg(sock_fd, &msghdr, 0);

    double send_end = get_timestamp_us();

    if (sent < 0) {
      if (errno == EINTR || errno == EAGAIN)
        continue;
      perror("sendmsg failed");
      break;
    }

    if (sent != serialized_size) {
      fprintf(stderr, "[Thread %d] Partial sendmsg: %zd/%d\n", thread_id, sent,
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

  stats_print(&local_stats, "Client Thread (ONE-COPY)");
  stats_destroy(&local_stats);

  msg_free(msg);
  free(aligned_buffer);
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

  printf("=== MT25048 Part A2: One-Copy Client (sendmsg Sender) ===\n");
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
      num_threads = i;
      break;
    }
  }

  /* Wait for all threads */
  for (int i = 0; i < num_threads; i++) {
    pthread_join(threads[i], NULL);
  }

  /* Print aggregate statistics */
  stats_print(&global_stats, "Overall Client (ONE-COPY)");
  stats_destroy(&global_stats);

  free(threads);
  free(thread_args);

  printf("\nClient finished\n");
  return 0;
}
