/*
 * MT25048_Part_A3_Client.c - Zero-Copy TCP Client (MSG_ZEROCOPY)
 * Roll Number: MT25048
 *
 * This client SENDS data continuously using sendmsg() with the MSG_ZEROCOPY
 * flag (requires Linux 4.14+).
 *
 * ZERO-COPY EXPLANATION:
 * ----------------------
 * Both traditional copies are eliminated:
 *
 * Traditional Path (Two-Copy):
 *   User Buffer → Kernel Socket Buffer (Copy 1)
 *   Kernel Socket Buffer → NIC (Copy 2)
 *
 * Zero-Copy Path (MSG_ZEROCOPY):
 *   User Buffer ← Kernel Pins Pages
 *   User Buffer → NIC via DMA (Direct Memory Access)
 *
 * ASCII Diagram:
 * +-------------+
 * | User Buffer |-----(DMA)----> [ NIC ]
 * +-------------+      ^
 *       |              |
 *       +--[Page Pin]--+
 *      (Kernel pins user pages, NIC reads directly)
 *
 * COMPLETION NOTIFICATION:
 * ------------------------
 * Because the kernel pins user pages, we must wait for transmission to complete
 * before reusing the buffer. Completion is signaled via MSG_ERRQUEUE.
 *
 * Process:
 * 1. sendmsg(..., MSG_ZEROCOPY)
 * 2. Kernel pins pages and queues for DMA
 * 3. When NIC completes transmission, kernel sends notification to MSG_ERRQUEUE
 * 4. Application calls recvmsg(..., MSG_ERRQUEUE) to get completion
 * 5. Only then can buffer be safely reused or freed
 *
 * LIMITATIONS:
 * - Only effective for large messages (>~10KB typically)
 * - Requires kernel 4.14+
 * - Page pinning overhead for small messages
 * - Completion notification latency
 */

#include "MT25048_Part_A_common.h"
#include <linux/errqueue.h>
#include <netinet/tcp.h>
#include <sys/uio.h>

#define DEFAULT_HOST "127.0.0.1"
#define DEFAULT_PORT 9002 /* Matches A3 server */
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
 * check_zerocopy_support - Verify kernel supports MSG_ZEROCOPY
 */
int check_zerocopy_support(int sock_fd) {
  int zerocopy = 1;
  if (setsockopt(sock_fd, SOL_SOCKET, SO_ZEROCOPY, &zerocopy,
                 sizeof(zerocopy)) < 0) {
    if (errno == ENOPROTOOPT) {
      fprintf(stderr, "MSG_ZEROCOPY not supported (requires Linux 4.14+)\n");
      fprintf(stderr, "Falling back to regular sendmsg()\n");
      return 0;
    }
    perror("setsockopt SO_ZEROCOPY failed");
    return -1;
  }
  return 1; /* Supported */
}

/*
 * handle_zerocopy_completions - Process completion notifications from
 * MSG_ERRQUEUE
 */
int handle_zerocopy_completions(int sock_fd, int *completed_count) {
  char control[100];
  struct msghdr msg = {0};
  struct iovec iov = {0};

  msg.msg_iov = &iov;
  msg.msg_iovlen = 1;
  msg.msg_control = control;
  msg.msg_controllen = sizeof(control);

  /* Receive from error queue (non-blocking if available) */
  int ret = recvmsg(sock_fd, &msg, MSG_ERRQUEUE | MSG_DONTWAIT);
  if (ret < 0) {
    if (errno == EAGAIN || errno == EWOULDBLOCK) {
      return 0; /* No notifications available */
    }
    perror("recvmsg MSG_ERRQUEUE failed");
    return -1;
  }

  /* Parse control messages for zerocopy completion */
  struct cmsghdr *cm;
  for (cm = CMSG_FIRSTHDR(&msg); cm; cm = CMSG_NXTHDR(&msg, cm)) {
    if (cm->cmsg_level == SOL_IP && cm->cmsg_type == IP_RECVERR) {
      struct sock_extended_err *serr;
      serr = (struct sock_extended_err *)CMSG_DATA(cm);

      if (serr->ee_origin == SO_EE_ORIGIN_ZEROCOPY) {
        /* Zerocopy completion notification */
        uint32_t lo = serr->ee_info; /* First notification ID */
        uint32_t hi = serr->ee_data; /* Last notification ID */

        *completed_count += (hi - lo + 1);

        /* ee_code indicates status:
         * 0 = success (zerocopy)
         * 1 = fallback (copied)
         */
        if (serr->ee_code != 0) {
          /* Kernel fell back to copy */
          // This is not an error, just means zerocopy wasn't used
        }
      }
    }
  }

  return 1;
}

/*
 * sender_thread - Thread function using MSG_ZEROCOPY for zero-copy
 * transmission
 */
void *sender_thread(void *arg) {
  client_thread_args_t *args = (client_thread_args_t *)arg;
  const char *host = args->host;
  int port = args->port;
  size_t msg_size = args->msg_size;
  int duration_sec = args->duration_sec;
  int thread_id = args->thread_id;

  printf("[Thread %d] Connecting to %s:%d (ZERO-COPY mode)\n", thread_id, host,
         port);

  /* Create socket */
  int sock_fd = socket(AF_INET, SOCK_STREAM, 0);
  if (sock_fd < 0) {
    perror("socket failed");
    return NULL;
  }

  /* Check zerocopy support */
  int zerocopy_enabled = check_zerocopy_support(sock_fd);
  if (zerocopy_enabled < 0) {
    close(sock_fd);
    return NULL;
  }

  if (zerocopy_enabled == 0) {
    printf(
        "[Thread %d] WARNING: Zerocopy not supported, using regular sendmsg\n",
        thread_id);
  } else {
    printf("[Thread %d] Zerocopy enabled\n", thread_id);
  }

  /* Connect to server */
  struct sockaddr_in server_addr = {0};
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

  /* Allocate page-aligned buffer (required for zerocopy) */
  void *aligned_buffer;
  size_t buffer_size = sizeof(size_t) + msg_size;
  size_t aligned_size = ((buffer_size + PAGE_SIZE - 1) / PAGE_SIZE) * PAGE_SIZE;

  if (posix_memalign(&aligned_buffer, PAGE_SIZE, aligned_size) != 0) {
    perror("posix_memalign failed");
    close(sock_fd);
    return NULL;
  }

  /* Create and serialize message */
  message_t *msg = msg_alloc(msg_size / NUM_FIELDS);
  if (!msg) {
    free(aligned_buffer);
    close(sock_fd);
    return NULL;
  }

  int serialized_size =
      msg_serialize(msg, (char *)aligned_buffer, aligned_size);
  if (serialized_size < 0) {
    msg_free(msg);
    free(aligned_buffer);
    close(sock_fd);
    return NULL;
  }

  /* Prepare iovec for sendmsg */
  struct iovec iov[1];
  iov[0].iov_base = aligned_buffer;
  iov[0].iov_len = serialized_size;

  struct msghdr msghdr = {0};
  msghdr.msg_iov = iov;
  msghdr.msg_iovlen = 1;

  stats_t local_stats;
  stats_init(&local_stats);

  double start_time = get_timestamp_sec();
  double end_time = start_time + duration_sec;

  int send_count = 0;
  int completion_count = 0;

  /* Send loop */
  while (get_timestamp_sec() < end_time && !shutdown_flag) {
    double send_start = get_timestamp_us();

    /*
     * ZERO-COPY SENDMSG with MSG_ZEROCOPY:
     * - Kernel pins user pages
     * - NIC performs DMA directly from user buffer
     * - Completion notification sent to MSG_ERRQUEUE when done
     */
    int flags = zerocopy_enabled ? MSG_ZEROCOPY : 0;
    ssize_t sent = sendmsg(sock_fd, &msghdr, flags);

    double send_end = get_timestamp_us();

    if (sent < 0) {
      if (errno == EINTR || errno == EAGAIN || errno == ENOBUFS) {
        /* Handle completions to free up buffers */
        handle_zerocopy_completions(sock_fd, &completion_count);
        continue;
      }
      perror("sendmsg MSG_ZEROCOPY failed");
      break;
    }

    if (sent != serialized_size) {
      fprintf(stderr, "[Thread %d] Partial sendmsg: %zd/%d\n", thread_id, sent,
              serialized_size);
      break;
    }

    send_count++;
    stats_update(&local_stats, serialized_size, send_end - send_start);

    /* Periodically check for completions */
    if (send_count % 100 == 0) {
      handle_zerocopy_completions(sock_fd, &completion_count);
    }
  }

  /* Drain remaining completions */
  if (zerocopy_enabled) {
    printf("[Thread %d] Draining completions (%d sent)...\n", thread_id,
           send_count);
    while (completion_count < send_count) {
      if (handle_zerocopy_completions(sock_fd, &completion_count) == 0) {
        usleep(1000); /* Wait 1ms */
      }
    }
    printf("[Thread %d] All completions received (%d)\n", thread_id,
           completion_count);
  }

  /* Update global statistics */
  pthread_mutex_lock(&args->global_stats->lock);
  args->global_stats->total_bytes += local_stats.total_bytes;
  args->global_stats->total_messages += local_stats.total_messages;
  args->global_stats->total_latency_us += local_stats.total_latency_us;
  pthread_mutex_unlock(&args->global_stats->lock);

  stats_print(&local_stats, "Client Thread (ZERO-COPY)");
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

  printf("=== MT25048 Part A3: Zero-Copy Client (MSG_ZEROCOPY Sender) ===\n");
  printf("Server: %s:%d\n", host, port);
  printf("Message Size: %zu bytes\n", msg_size);
  printf("Duration: %d seconds\n", duration);
  printf("Threads: %d\n", num_threads);
  printf("Requires: Linux kernel 4.14+\n\n");

  setup_signal_handlers();

  /* Global statistics */
  stats_t global_stats;
  stats_init(&global_stats);

  /* Create threads */
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
  stats_print(&global_stats, "Overall Client (ZERO-COPY)");
  stats_destroy(&global_stats);

  free(threads);
  free(thread_args);

  printf("\nClient finished\n");
  return 0;
}
