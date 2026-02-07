# MT25048 - PA02: Network I/O Analysis using perf
# Roll Number: MT25048

CC = gcc
CFLAGS = -O2 -pthread -g -Wall -Wextra
LDFLAGS = -pthread

# All targets
all: a1_server a1_client a2_server a2_client a3_server a3_client

# Part A1: Two-Copy Implementation (Baseline)
a1_server: MT25048_Part_A1_Server.c MT25048_Part_A_common.c MT25048_Part_A_common.h
	$(CC) $(CFLAGS) -o $@ MT25048_Part_A1_Server.c MT25048_Part_A_common.c $(LDFLAGS)

a1_client: MT25048_Part_A1_Client.c MT25048_Part_A_common.c MT25048_Part_A_common.h
	$(CC) $(CFLAGS) -o $@ MT25048_Part_A1_Client.c MT25048_Part_A_common.c $(LDFLAGS)

# Part A2: One-Copy Implementation (sendmsg)
a2_server: MT25048_Part_A2_Server.c MT25048_Part_A_common.c MT25048_Part_A_common.h
	$(CC) $(CFLAGS) -o $@ MT25048_Part_A2_Server.c MT25048_Part_A_common.c $(LDFLAGS)

a2_client: MT25048_Part_A2_Client.c MT25048_Part_A_common.c MT25048_Part_A_common.h
	$(CC) $(CFLAGS) -o $@ MT25048_Part_A2_Client.c MT25048_Part_A_common.c $(LDFLAGS)

# Part A3: Zero-Copy Implementation (MSG_ZEROCOPY)
a3_server: MT25048_Part_A3_Server.c MT25048_Part_A_common.c MT25048_Part_A_common.h
	$(CC) $(CFLAGS) -o $@ MT25048_Part_A3_Server.c MT25048_Part_A_common.c $(LDFLAGS)

a3_client: MT25048_Part_A3_Client.c MT25048_Part_A_common.c MT25048_Part_A_common.h
	$(CC) $(CFLAGS) -o $@ MT25048_Part_A3_Client.c MT25048_Part_A_common.c $(LDFLAGS)

# Clean build artifacts
clean:
	rm -f a1_server a1_client a2_server a2_client a3_server a3_client *.o

# Clean everything including CSV data
cleanall: clean
	rm -f raw_csvs/*.csv

.PHONY: all clean cleanall
