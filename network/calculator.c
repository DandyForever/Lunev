#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <pthread.h>
#include <errno.h>
#include <sched.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <setjmp.h>
#include <limits.h>
#include <netinet/tcp.h>
#include <signal.h>
#include "./net.h"

struct sockaddr_in baddr;
struct net_msg msg;
int sock, bytes;
struct sockaddr_in addr;
socklen_t addr_len = sizeof(struct sockaddr_in);
long double h, h2;
long double* data;
pthread_t* threads;

void recv_broadcast();
void wait_for_job();
void* simpson(void* args);
void sys_info(unsigned n, SysInfo * sysInfo);
void* calculate(void* args);

unsigned N;

void handler (int sig){
    printf ("MASTER IS DEAD\n");
    sig++;
    exit (EXIT_FAILURE);
}

long long parse_n (char* str){
    char* endptr = 0;
    long long val = 0;
    errno = 0;

    val = strtoll(str, &endptr, 10);

    CHECK(*endptr != '\0' || endptr == str, parse_n, EINVAL);

    CHECK((errno == ERANGE && (val == LONG_MAX || val == LONG_MIN))  ||
          (errno != 0 && val == 0), parse_n, 0);

    return val;
}


int main(int argc, char** argv) {
    struct sigaction act_to = {
            .sa_handler = handler
    };
    sigaction (SIGIO, &act_to, NULL);

    CHECK(argc != 2, arguments, EINVAL);
    N = parse_n(argv[1]);

    recv_broadcast();
    wait_for_job();

    fcntl(sock, F_SETFL, O_ASYNC);

    SysInfo sysInfo;
    sysInfo.use_hyper = 0;
    sys_info(N, &sysInfo);

    pthread_t threads[10000];
    CalculateData data[10000];

    if (!sysInfo.use_hyper) {
        double  interval_start = msg.interval_start,
                interval_len = (double)(msg.interval_end - msg.interval_start) / (double) msg.cores;
        unsigned j = 0, i = 0;

        for (i = 0; i < sysInfo.online_cpus_num; ++i) {

            while(!sysInfo.is_cpu_online[j]) ++j;

            data[i] = (CalculateData) {
                    interval_start,
                    interval_start + interval_len,
                    0,
                    sysInfo.cpu_sets[j],
            };

            interval_start += interval_len;
            j++;

        }

        for (i = 0; i < sysInfo.online_cpus_num; i++)
            CHECK(pthread_create(&threads[i], NULL, &calculate, &data[i]), pthread_create, 0);

        double sum = 0;
        for (i = 0; i < sysInfo.online_cpus_num; ++i) {
            CHECK(pthread_join(threads[i], NULL), pthread_join, 0);
            if (i < N)
                sum += data[i].result;
        }

        printf ("%.5f\n", sum);
        msg.h = sum;
    }
    else {
        double  interval_start = msg.interval_start,
                interval_len = (double)(msg.interval_end - msg.interval_start) / (double) msg.cores;
        unsigned i;

        for (i = 0; i < N; ++i) {

            data[i] = (CalculateData) {
                    interval_start,
                    interval_start + interval_len,
                    0,
                    sysInfo.virtual_cpu_sets[i % sysInfo.online_virtual_cpus_num]
            };
            interval_start += interval_len;
        }

        for (i = 0; i < N; i++)
            CHECK(pthread_create(&threads[i], NULL, &calculate, &data[i]), pthread_create, 0);

        double sum = 0;
        for (i = 0; i < N; ++i) {
            CHECK(pthread_join(threads[i], NULL), pthread_join, 0);
            sum += data[i].result;
        }

        printf ("%.5f\n", sum);
        msg.h = sum;
    }

    fcntl(sock, F_SETFL, 0);

    bytes = write(sock, &msg, sizeof(struct net_msg));
    CHECK(bytes == -1, write, 0);
    CHECK(bytes != sizeof(struct net_msg), message send, 0);

    printf ("Ready\n");
    shutdown(sock, SHUT_RDWR);
    close(sock);
    exit(EXIT_SUCCESS);

}


void recv_broadcast() {
    int bsock, recv_bytes;
    socklen_t baddr_len = sizeof(struct sockaddr_in);

    bsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    CHECK(bsock == -1, socket, 0);
    baddr.sin_addr.s_addr = htonl(INADDR_ANY);
    baddr.sin_port = htons(BROADCAST_PORT);
    baddr.sin_family = AF_INET;

    int ld1 = 1;
    CHECK(setsockopt(bsock, SOL_SOCKET, SO_REUSEPORT, &ld1, sizeof(ld1)) == -1, setsockopt, 0);
    CHECK(bind(bsock, (struct sockaddr*)&baddr, baddr_len) == -1, bind, 0);

    printf ("Waiting for the server...\n");

    recv_bytes = recvfrom(bsock, &msg, sizeof(struct net_msg), 0, (struct sockaddr *)&baddr, &baddr_len);
    CHECK(recv_bytes == -1, recvfrom, 0);
    CHECK(recv_bytes != sizeof(struct net_msg), message receiving, 0);
    printf ("Received %d bytes; server port %d\n", recv_bytes, msg.tcp_port);
    baddr.sin_port = msg.tcp_port;

    shutdown(bsock, SHUT_RDWR);
    close(bsock);
}


void wait_for_job() {
    sock = socket(PF_INET, SOCK_STREAM, 0);
    CHECK(sock == -1, socket, 0);
    memset(&addr, 0, addr_len);
    addr.sin_addr.s_addr = baddr.sin_addr.s_addr;
    addr.sin_port = baddr.sin_port;
    addr.sin_family = AF_INET;

    int ld1 = 1;
    CHECK(setsockopt(sock, SOL_SOCKET, SO_REUSEPORT, &ld1, sizeof(ld1)) == -1, setsockopt, 0);

    CHECK(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &ld1, sizeof(ld1)) == -1, setsockopt, 0);
    CHECK(setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &ld1, sizeof(ld1)) == -1, setsockopt, 0);
    CHECK(setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &ld1, sizeof(ld1)) == -1, setsockopt, 0);
    CHECK(setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &ld1, sizeof(ld1)) == -1, setsockopt, 0);

    CHECK(connect(sock, (struct sockaddr*)&addr, addr_len) == -1, connect, 0);

    fcntl(sock, F_SETOWN, getpid());

    getsockname(sock, (struct sockaddr*)&baddr, &addr_len);
    printf ("Connected (port %d)\n", ntohs(baddr.sin_port));

    msg.cores = N;

    bytes = write(sock, &msg, sizeof(struct net_msg));
    CHECK(bytes == -1 || bytes != sizeof (struct net_msg), write, 0);
    printf ("Waiting for job...\n");

    bytes = read(sock, &msg, sizeof(struct net_msg));
    CHECK(bytes != sizeof(struct net_msg), read, 0);
}

void sys_info(unsigned n, SysInfo * sysInfo) {
    memset(&(sysInfo -> is_virtual_cpu_online), 0, sizeof(sysInfo -> is_virtual_cpu_online));

    int fd = open("/sys/devices/system/cpu/online", O_RDONLY);
    CHECK(fd == -1, open, 0);
    char online_str[MAX_SYS_FILE_LENGTH];
    int bytes_read = read(fd, online_str, MAX_SYS_FILE_LENGTH-1);
    CHECK(bytes_read == -1, read, 0);
    close(fd);
    online_str[bytes_read] = '\0';

    unsigned left_bound, right_bound, i;
    char *saveptr1 = NULL,
            *saveptr2 = NULL,
            *str = strtok_r(online_str, ",", &saveptr1);

    do {
        str = strtok_r(str, "-", &saveptr2);
        CHECK(!sscanf(str, "%u", &left_bound), sscanf, 0);
        right_bound = left_bound;

        str = strtok_r(NULL, "-", &saveptr2);
        if (str)
            CHECK(!sscanf(str, "%u", &right_bound), sscanf, 0);

        for (i = left_bound; i <= right_bound; i++)
            sysInfo -> is_virtual_cpu_online[i] = 1;
    } while ((str = strtok_r(NULL, ",", &saveptr1)));


    memset(&(sysInfo -> is_cpu_online), 0, sizeof(sysInfo -> is_cpu_online));
    for (i = 0; i < MAX_CPUS; i++) {
        CPU_ZERO(&(sysInfo -> cpu_sets[i]));
        CPU_ZERO(&(sysInfo -> virtual_cpu_sets[i]));
    }

    char filename[256],
            core_str[11];
    int  core_id;

    for (i = 0; i < MAX_CPUS; i++) {
        if (!sysInfo -> is_virtual_cpu_online[i])
            continue;

        CPU_SET(i, &(sysInfo -> virtual_cpu_sets[sysInfo -> online_virtual_cpus_num]));
        sysInfo -> online_virtual_cpus_num++;

        sprintf(filename, "/sys/devices/system/cpu/cpu%d/topology/core_id", i);
        fd = open(filename, O_RDONLY);
        CHECK(fd == -1, open, 0);
        int bytes_read = read(fd, core_str, 10);
        CHECK(bytes_read == -1, read, 0);
        close(fd);
        core_str[bytes_read] = '\0';

        CHECK(!sscanf(core_str, "%d", &core_id), sscanf, 0);

        if (!sysInfo -> is_cpu_online[core_id]) {
            sysInfo -> is_cpu_online[core_id] = 1;
            sysInfo -> online_cpus_num++;

            CPU_SET(i, &(sysInfo -> cpu_sets[core_id]));
        }
    }

    if (sysInfo -> online_cpus_num < n) sysInfo -> use_hyper = 1;
}


void* calculate(void* args) {
    cpu_set_t cpuset = ((CalculateData*)args)->cpuset;
    pthread_t current_thread = pthread_self();
    CHECK(pthread_setaffinity_np(current_thread, sizeof(cpu_set_t), &cpuset), pthread_setaffinity_np, 0);

    double start = ((CalculateData*)args) -> start,
            finish = ((CalculateData*)args) -> finish,
            dx = 0.000000001,
            result = 0;

    while (start < finish){
        result += FUNC(start) * dx;
        start += dx;
    }

    ((CalculateData*)args)->result = result;
    return NULL;
}
