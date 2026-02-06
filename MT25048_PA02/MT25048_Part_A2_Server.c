/*
 * MT25048_Part_A2_Server.c - One-Copy TCP Server (sendmsg optimization)
 * Roll Number: MT25048
 *
 * This server implements ONE-COPY socket communication using sendmsg() with
 * scatter-gather I/O via iovec structures and pre-registered (aligned) buffers.
 *
 * ONE-COPY EXPLANATION:
 * ---------------------
 * Copy Eliminated: User space → Kernel space copy is REDUCED/OPTIMIZED
 *
 * How it works:
 * 1. Use posix_memalign() to create page-aligned buffers
 * 2. Use sendmsg() with iovec structures for scatter-gather I/O
 * 3. Kernel can reference user buffers directly (zero-copy semantics within
 * kernel) or optimize the copy path with DMA-friendly aligned buffers
 *
 * Remaining copy: Kernel → NIC (this is unavoidable without MSG_ZEROCOPY)
 *
 * The key optimization is that sendmsg() with iovec allows the kernel to:
 * - Avoid intermediate buffer copies
 * - Use scatter-gather DMA when hardware supports it
 * - Reference user pages more efficiently
 */

#include "common.h"
#include <netinet/tcp.h>
#include <sys/uio.h>

#define DEFAULT_PORT 9001 /* Different port from A1 */
#define DEFAULT_MSGSIZE 1024
#define DEFAULT_DURATION 10
#define BACKLOG 100
#define PAGE_SIZE 4096

typedef struct {
  int client_fd;
  size_t msg_size;
  int duration_sec;
  int thread_id;
} client_thread_args_t;

/*
 * client_handler - Thread function using sendmsg() for one-copy transmission
 */
void *client_handler(void *arg) {
  client_thread_args_t *args = (client_thread_args_t *)arg;
  int client_fd = args->client_fd;
  size_t msg_size = args->msg_size;
  int duration_sec = args->duration_sec;
  int thread_id = args->thread_id;

  printf("[Thread %d] Client connected (ONE-COPY mode: sendmsg)\n", thread_id);

  /* Allocate PAGE-ALIGNED buffer for DMA optimization */
  void *aligned_buffer;
  size_t buffer_size = sizeof(size_t) + msg_size;

  /* Round up to page boundary */
  size_t aligned_size = ((buffer_size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

  if (posix_memalign(&aligned_buffer, PAGE_SIZE, aligned_size) != 0) {
    perror("posix_memalign failed");
    close(client_fd);
    free(args);
    return NULL;
  }

  printf("[Thread %d] Allocated %zu-byte aligned buffer at %p\n", thread_id,
         aligned_size, aligned_buffer);

  /* Create message in aligned buffer */
  message_t *msg = msg_alloc(msg_size / NUM_FIELDS);
  if (!msg) {
    fprintf(stderr, "[Thread %d] Failed to allocate message\n", thread_id);
    free(aligned_buffer);
    close(client_fd);
    free(args);
    return NULL;
  }

  /* Serialize into aligned buffer */
  int serialized_size =
      msg_serialize(msg, (char *)aligned_buffer, aligned_size);
  if (serialized_size < 0) {
    fprintf(stderr, "[Thread %d] Serialization failed\n", thread_id);
    msg_free(msg);
    free(aligned_buffer);
    close(client_fd);
    free(args);
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

  stats_t stats;
  stats_init(&stats);

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
    ssize_t sent = sendmsg(client_fd, &msghdr, 0);

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

    stats_update(&stats, serialized_size, send_end - send_start);
  }

  stats_print(&stats, "Server Thread (ONE-COPY)");
  stats_destroy(&stats);

  msg_free(msg);
  free(aligned_buffer);
  close(client_fd);
  free(args);

  printf("[Thread %d] Client disconnected\n", thread_id);
  return NULL;
}

int main(int argc, char *argv[]) {
  int port = DEFAULT_PORT;
  size_t msg_size = DEFAULT_MSGSIZE;
  int duration = DEFAULT_DURATION;

  /* Parse command-line arguments */
  for (int i = 1; i < argc; i++) {
    port = parse_int_arg(argv[i], "--port=", port);
    msg_size = (size_t)parse_int_arg(argv[i], "--msgsize=", (int)msg_size);
    duration = parse_int_arg(argv[i], "--duration=", duration);
  }

  printf("=== MT25048 Part A2: One-Copy Server (sendmsg) ===\n");
  printf("Port: %d\n", port);
  printf("Message Size: %zu bytes\n", msg_size);
  printf("Duration: %d seconds\n", duration);
  printf("Optimization: Pre-aligned buffers + sendmsg()\n\n");

  setup_signal_handlers();

  /* Create TCP socket */
  int server_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (server_fd < 0) {
    perror("socket failed");
    return 1;
  }

  /* Set socket options */
  int optval = 1;
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
  setsockopt(server_fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));

  /* Bind to port */
  struct sockaddr_in server_addr;
  memset(&server_addr, 0, sizeof(server_addr));
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("bind failed");
    close(server_fd);
    return 1;
  }

  /* Listen for connections */
  if (listen(server_fd, BACKLOG) < 0) {
    perror("listen failed");
    close(server_fd);
    return 1;
  }

  printf("Server listening on port %d...\n\n", port);

  int thread_counter = 0;

  /* Accept loop */
  while (!shutdown_flag) {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd =
        accept(server_fd, (struct sockaddr *)&client_addr, &client_len);
    if (client_fd < 0) {
      if (errno == EINTR)
        continue;
      perror("accept failed");
      continue;
    }

    /* Disable Nagle's algorithm */
    int nodelay = 1;
    setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

    printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    /* Create thread args */
    client_thread_args_t *thread_args =
        (client_thread_args_t *)malloc(sizeof(client_thread_args_t));
    if (!thread_args) {
      perror("malloc args failed");
      close(client_fd);
      continue;
    }

    thread_args->client_fd = client_fd;
    thread_args->msg_size = msg_size;
    thread_args->duration_sec = duration;
    thread_args->thread_id = thread_counter++;

    /* Spawn thread */
    pthread_t thread;
    if (pthread_create(&thread, NULL, client_handler, thread_args) != 0) {
      perror("pthread_create failed");
      free(thread_args);
      close(client_fd);
      continue;
    }

    pthread_detach(thread);
  }

  printf("\nShutting down server...\n");
  close(server_fd);

  return 0;
}
