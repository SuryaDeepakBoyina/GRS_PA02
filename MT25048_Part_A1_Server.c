/*
 * MT25048_Part_A1_Server.c - Two-Copy TCP Server (Baseline)
 * Roll Number: MT25048
 *
 * This server RECEIVES data from clients using recv() (traditional two-copy).
 *
 * TWO-COPY EXPLANATION:
 * ---------------------
 * Copy 1: NIC hardware buffer → Kernel socket buffer (via DMA/network stack)
 * Copy 2: Kernel socket buffer → User space buffer (via recv() syscall)
 *
 * Architecture:
 * - Main thread: Accepts connections
 * - Worker threads: One thread per client, continuously receives messages
 */

#include "MT25048_Part_A_common.h"
#include <netinet/tcp.h>

#define DEFAULT_PORT 9000
#define DEFAULT_MSGSIZE 1024
#define DEFAULT_DURATION 10
#define BACKLOG 100
#define RECV_BUFFER_SIZE (256 * 1024) /* 256 KB */

typedef struct {
  int client_fd;
  size_t msg_size;
  int duration_sec;
  int thread_id;
} client_thread_args_t;

/*
 * client_handler - Thread function to handle one client connection
 *
 * Continuously receives messages using recv() (two-copy path) until duration
 * expires or client disconnects.
 */
void *client_handler(void *arg) {
  client_thread_args_t *args = (client_thread_args_t *)arg;
  int client_fd = args->client_fd;
  size_t msg_size = args->msg_size;
  int duration_sec = args->duration_sec;
  int thread_id = args->thread_id;

  printf("[Thread %d] Client connected, ready to receive (msgsize=%zu, "
         "duration=%d)\n",
         thread_id, msg_size, duration_sec);

  /* Allocate receive buffer */
  size_t buffer_size = sizeof(size_t) + msg_size;
  char *recv_buffer = (char *)malloc(buffer_size);
  if (!recv_buffer) {
    perror("malloc recv_buffer failed");
    close(client_fd);
    free(args);
    return NULL;
  }

  /* Statistics tracking */
  stats_t stats;
  stats_init(&stats);

  double start_time = get_timestamp_sec();
  double end_time = start_time + duration_sec;

  /* Receive loop */
  while (get_timestamp_sec() < end_time && !shutdown_flag) {
    double recv_start = get_timestamp_us();

    /* TWO-COPY RECV: Data copied from kernel socket buffer to user buffer */
    ssize_t received = recv(client_fd, recv_buffer, buffer_size, 0);

    double recv_end = get_timestamp_us();

    if (received < 0) {
      if (errno == EINTR || errno == EAGAIN)
        continue;
      perror("recv failed");
      break;
    }

    if (received == 0) {
      /* Connection closed by client */
      printf("[Thread %d] Client closed connection\n", thread_id);
      break;
    }

    stats_update(&stats, received, recv_end - recv_start);
  }

  stats_print(&stats, "Server Thread (TWO-COPY)");
  stats_destroy(&stats);

  free(recv_buffer);
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

  printf("=== MT25048 Part A1: Two-Copy Server (Receiver) ===\n");
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

  /* Configure receive buffer size */
  int recvbuf = RECV_BUFFER_SIZE;
  setsockopt(server_fd, SOL_SOCKET, SO_RCVBUF, &recvbuf, sizeof(recvbuf));

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
        continue; /* Interrupted by signal */
      perror("accept failed");
      continue;
    }

    /* Disable Nagle's algorithm for lower latency */
    int nodelay = 1;
    setsockopt(client_fd, IPPROTO_TCP, TCP_NODELAY, &nodelay, sizeof(nodelay));

    printf("Accepted connection from %s:%d\n", inet_ntoa(client_addr.sin_addr),
           ntohs(client_addr.sin_port));

    /* Create thread args */
    client_thread_args_t *args =
        (client_thread_args_t *)malloc(sizeof(client_thread_args_t));
    if (!args) {
      perror("malloc args failed");
      close(client_fd);
      continue;
    }

    args->client_fd = client_fd;
    args->msg_size = msg_size;
    args->duration_sec = duration;
    args->thread_id = thread_counter++;

    /* Spawn thread to handle client */
    pthread_t thread;
    if (pthread_create(&thread, NULL, client_handler, args) != 0) {
      perror("pthread_create failed");
      free(args);
      close(client_fd);
      continue;
    }

    /* Detach thread - it will clean up itself */
    pthread_detach(thread);
  }

  printf("\nShutting down server...\n");
  close(server_fd);

  return 0;
}
