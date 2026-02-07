/*
 * MT25048_Part_A3_Server.c - Zero-Copy TCP Server (Receiver)
 * Roll Number: MT25048
 *
 * This server RECEIVES data from zero-copy clients.
 * Uses standard recv() as MSG_ZEROCOPY is primarily beneficial for send
 * operations.
 *
 * ZERO-COPY EXPLANATION:
 * ----------------------
 * The zero-copy optimization applies on the SEND side (client).
 * On the receive side, we use standard recv() with aligned buffers
 * for optimal performance.
 */

#include "MT25048_Part_A_common.h"
#include <netinet/tcp.h>

#define DEFAULT_PORT 9002 /* Different port from A1/A2 */
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
 * client_handler - Thread function to receive data from zero-copy client
 */
void *client_handler(void *arg) {
  client_thread_args_t *args = (client_thread_args_t *)arg;
  int client_fd = args->client_fd;
  size_t msg_size = args->msg_size;
  int duration_sec = args->duration_sec;
  int thread_id = args->thread_id;

  printf("[Thread %d] Client connected (ZERO-COPY mode receiver)\n", thread_id);

  /* Allocate page-aligned receive buffer for optimal performance */
  void *aligned_buffer;
  size_t buffer_size = sizeof(size_t) + msg_size;
  size_t aligned_size = ((buffer_size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

  if (posix_memalign(&aligned_buffer, PAGE_SIZE, aligned_size) != 0) {
    perror("posix_memalign failed");
    close(client_fd);
    free(args);
    return NULL;
  }

  stats_t stats;
  stats_init(&stats);

  double start_time = get_timestamp_sec();
  double end_time = start_time + duration_sec;

  /* Receive loop */
  while (get_timestamp_sec() < end_time && !shutdown_flag) {
    double recv_start = get_timestamp_us();

    /* Standard recv - zerocopy benefit is on send side */
    ssize_t received = recv(client_fd, aligned_buffer, aligned_size, 0);

    double recv_end = get_timestamp_us();

    if (received < 0) {
      if (errno == EINTR || errno == EAGAIN)
        continue;
      perror("recv failed");
      break;
    }

    if (received == 0) {
      printf("[Thread %d] Client closed connection\n", thread_id);
      break;
    }

    stats_update(&stats, received, recv_end - recv_start);
  }

  stats_print(&stats, "Server Thread (ZERO-COPY)");
  stats_destroy(&stats);

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

  printf("=== MT25048 Part A3: Zero-Copy Server (Receiver) ===\n");
  printf("Port: %d\n", port);
  printf("Message Size: %zu bytes\n", msg_size);
  printf("Duration: %d seconds\n\n", duration);

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

  /* Bind */
  struct sockaddr_in server_addr = {0};
  server_addr.sin_family = AF_INET;
  server_addr.sin_addr.s_addr = INADDR_ANY;
  server_addr.sin_port = htons(port);

  if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) <
      0) {
    perror("bind failed");
    close(server_fd);
    return 1;
  }

  /* Listen */
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

    int nodelay = 1;
    setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

    printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    client_thread_args_t *thread_args =
        (client_thread_args_t *)malloc(sizeof(client_thread_args_t));
    if (!thread_args) {
      close(client_fd);
      continue;
    }

    thread_args->client_fd = client_fd;
    thread_args->msg_size = msg_size;
    thread_args->duration_sec = duration;
    thread_args->thread_id = thread_counter++;

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
