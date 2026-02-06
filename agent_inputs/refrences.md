This document lists all external references used for theory refresh, implementation guidance, performance analysis, and report justification for PA02: Analysis of Network I/O primitives using perf. 

Each resource is mapped to specific assignment parts (A–E) to ensure traceability and responsible AI usage. an try to extract some references from this md into the refernces foler also

1. Course Lectures & Slides (Primary References)
Principles of Computer System Design

Video: https://www.youtube.com/watch?v=hxcPkVlzbPk

Slides: https://www.cse.iitb.ac.in/~mythili/decs/slides_pdf/lecture2.pdf

Used for:

System design trade-offs

User–kernel boundary concepts

Performance vs abstraction discussion (Part E)

Process & Kernel Mode of Execution

Videos:

https://youtu.be/uK08dU6NIs8

https://youtu.be/glVOi25qSmg

Slides:

https://www.cse.iitb.ac.in/~mythili/decs/slides_pdf/lecture6.pdf

https://www.cse.iitb.ac.in/~mythili/decs/slides_pdf/lecture7.pdf

Used for:

System calls

Context switches

Blocking behavior explanations (Parts A & E)

Threads

Video: https://youtu.be/Y1PF0fE-v9M

Slides: https://www.cse.iitb.ac.in/~mythili/decs/slides_pdf/lecture8.pdf

Used for:

Thread-per-client server design

Cache contention and scalability analysis (Parts A & E)

2. Linux Networking & Socket Programming
Linux Network Stack

Video: https://www.youtube.com/watch?v=MpjlWt7fvrw

Kernel-bypass examples: https://github.com/netsys-iiitd/kernel-bypass/tree/master

Used for:

User space → kernel space path

sk_buff lifecycle

Copy points and cache behavior (Parts A, B, E)

Socket Programming

Beej’s Guide to Network Programming:
https://beej.us/guide/bgnet/

Used for:

TCP client-server implementation

Blocking send() / recv() behavior (Part A1)

Networking via Sockets (Slides)

Slides: https://www.cse.iitb.ac.in/~mythili/decs/slides_pdf/lecture18.pdf

Used for:

Socket API semantics

TCP buffering and flow control explanations

3. Event-Driven & Asynchronous I/O
Event-Driven Network I/O (select, poll, epoll)

Video: https://www.youtube.com/watch?v=dEHZb9JsmOU

Used for:

Understanding blocking vs non-blocking behavior

Explaining context-switch overhead (Part E)

epoll Example Code

https://github.com/eklitzke/epoll-example

Used for:

Conceptual comparison (not directly implemented in PA02)

4. One-Copy and Zero-Copy Networking
Linux sendmsg()

man page: https://man7.org/linux/man-pages/man2/sendmsg.2.html

Used for:

One-copy socket implementation (Part A2)

MSG_ZEROCOPY (Zero-Copy Networking)

LWN article: https://lwn.net/Articles/726917/

Kernel self-test example:
https://github.com/torvalds/linux/blob/master/tools/testing/selftests/net/zerocopy_send.c

Used for:

Zero-copy implementation

Kernel behavior explanation and diagrams (Part A3, Part E)

5. Linux Kernel Internals
Socket Buffers (sk_buff)

Blog (Linux networking internals):
https://arthurchiao.art/blog/linux-net-stack-en/

Used for:

Explaining where copies occur

Cache miss analysis

Metadata overhead discussion (Parts B & E)

6. Performance Profiling (perf)
perf Tool Documentation

Official Wiki: https://perf.wiki.kernel.org/index.php/Main_Page

Used for:

Metric selection

perf stat usage (Part B)

Practical perf Guide

Brendan Gregg: https://brendangregg.com/perf.html

Used for:

Interpreting cycles, cache misses, context switches

Micro-architectural analysis (Part E)

perf Hands-On Video

https://www.youtube.com/watch?v=zIQTWG9rZes

Used for:

Validating perf workflow and counters

7. RPC & Client-Server Models
RPC Conceptual Reference

https://book.systemsapproach.org/e2e/rpc.html

Used for:

Understanding request-response communication

Conceptual comparison with socket-based design

8. Containers & Namespaces
Containers

Video: https://youtu.be/4BG-hE72r_I

Slides: https://www.cse.iitb.ac.in/~mythili/virtcc/slides_pdf/11-containers.pdf

Used for:

Understanding network namespaces

Explaining why VMs are not allowed in PA02 setup

9. Assignment Specification
PA02 Problem Statement

Provided by instructor (PDF)
Used for:

All implementation, measurement, and reporting requirements

10. AI Usage Declaration Support

The above references were used to:

Validate AI-generated code

Cross-check kernel behavior

Ensure correctness of performance explanations

All AI-assisted outputs were:

Reviewed line-by-line

Matched against the above references

Declared explicitly in the report (as per PA02 guidelines)