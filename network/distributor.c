#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include "./net.h"
#include <netinet/in.h>
#include <netinet/tcp.h>


int bsock, clients_max, master, *client, *cores, bytes, cores_total;
struct sockaddr_in addr;
socklen_t addr_len = sizeof(struct sockaddr_in);
struct net_msg broadcast_msg;
long double a, b;

void arg_process(int argc, char** argv);
void* broadcast(void* args);
void wait_for_clients();


void enable_keepalive(int sock) {
    int yes = 1;
    CHECK(setsockopt(sock, SOL_SOCKET, SO_KEEPALIVE, &yes, sizeof(int)) == -1, setsockopt, 0);

    int idle = 1;

#ifdef LINUX
    CHECK(setsockopt(sock, IPPROTO_TCP, TCP_KEEPIDLE, &idle, sizeof(int)) == -1, setsockopt, 0);
#else
    CHECK(setsockopt(sock, IPPROTO_TCP, TCP_KEEPALIVE, &idle, sizeof(int)) == -1, setsockopt, 0);
#endif

    int interval = 1;
    CHECK(setsockopt(sock, IPPROTO_TCP, TCP_KEEPINTVL, &interval, sizeof(int)) == -1, setsockopt, 0);

    int maxpkt = 1;
    CHECK(setsockopt(sock, IPPROTO_TCP, TCP_KEEPCNT, &maxpkt, sizeof(int)) == -1, setsockopt, 0);
}



int main(int argc, char** argv)
{
    arg_process(argc, argv);

    client = calloc(clients_max, sizeof(int));
    cores = calloc(clients_max, sizeof(int));
    CHECK(client == 0 || cores == 0, calloc, 0);

    // Setup networking
    master = socket(PF_INET, SOCK_STREAM, 0);
    CHECK(master == -1, socket, 0);
    memset(&addr, 0, addr_len);
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(0);
    addr.sin_family = AF_INET;

    CHECK(bind(master, (struct sockaddr*)&addr, addr_len) == -1, bind, 0);
    CHECK(listen(master, clients_max) == -1, listen , 0);
    CHECK(getsockname(master, (struct sockaddr*)&addr, &addr_len) == -1, getsockname, 0);

    memset(&broadcast_msg, 0, sizeof(struct net_msg));
    broadcast_msg.tcp_port = addr.sin_port;

    // Start the broadcast
    pthread_t bthread;
    CHECK(pthread_create(&bthread, NULL, &broadcast, &broadcast_msg) != 0, pthread_create, 0);

    wait_for_clients();

    // Finish the broadcast
    pthread_cancel(bthread);
    pthread_join(bthread, NULL);
    shutdown(bsock, SHUT_RDWR);
    close(bsock);

    // Jobs distribution
    struct net_msg request;
    long double h = (b - a)/SPLIT;
    unsigned cores_count = 0;
    for (int i = 0; i < clients_max; ++i) {
        request = (struct net_msg) {
            0,
            cores[i],
            SPLIT/cores_total*cores[i],
            a + (b - a)/cores_total*cores_count,
            a + (b - a)/cores_total*(cores_count + cores[i]),
            h
        };

        bytes = write(client[i], &request, sizeof(struct net_msg));
        CHECK(bytes == -1, write, 0);
        CHECK(bytes != sizeof(struct net_msg), message sending, EINVAL);
        cores_count += cores[i];
    }

    printf ("Request sent\n");

    // Results collection
    for (int i = 0; i < clients_max; ++i) {
        enable_keepalive(client[i]);
    }

    // Filling fd_set
    fd_set fds;
    int maxsd = 0;
    long double S = 0;

//    FD_ZERO(&fds);
//    for (int i = 0; i < clients_max; ++i) {
//            if (client[i]) {
//                FD_SET(client[i], &fds);
//                if (client[i] > maxsd)
//                    maxsd = client[i];
//            }
//            else CHECK(0, some of fds is NULL, EINVAL);
//    }


    int ready = 0;
    while (ready < clients_max) {
        FD_ZERO(&fds);
        for (int i = 0; i < clients_max; ++i) {
            if (client[i]) {
                FD_SET(client[i], &fds);
                if (client[i] > maxsd)
                    maxsd = client[i];
            }
            else CHECK(0, some of fds is NULL, EINVAL);
        }

        CHECK((select(maxsd+1, &fds, NULL, NULL, NULL) == -1) && (errno != EINTR), select, 0);
        printf ("Select triggered\n");
        printf ("\tSlaves: %d\n", clients_max);

        for (int i = 0; i < clients_max; ++i) {
            if (FD_ISSET(client[i], &fds)) {
                printf ("Event #%d\n", client[i]);
                bytes = read(client[i], &request, sizeof(struct net_msg));
                printf ("%d\n", bytes);
                CHECK(bytes == -1, read, 0);
                CHECK(bytes != sizeof(struct net_msg), message failed, EINVAL);
                S += request.h;
                ++ready;
                client[i] = 0;

                printf ("client (%d) == %Lf\n", i, request.h);
//                if (bytes == sizeof(struct net_msg)){
//                    S += request.h;
//                    ++ready;
//
//                    printf ("client (%d) == %Lf\n", i, request.h);
//                }
            }
        }
    }

    printf("\n%.6Lf\n\n", S);

    free(client);
    free(cores);
    return 0;
}


void arg_process(int argc, char** argv) {
    CHECK(argc != 2, [NUM of CLIENTS], EINVAL);

    a = 3;
    b = 9;

    CHECK(!sscanf(argv[1], "%d", &clients_max), sscanf, 0);
}


void* broadcast(void* args) {
    struct sockaddr_in addr;

    int sent_bytes;
    socklen_t addr_len = sizeof(struct sockaddr_in);

    bsock = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP);
    CHECK(bsock == -1, socket, 0);
    memset(&addr, 0, addr_len);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    addr.sin_port = htons(0);
    addr.sin_family = AF_INET;

    CHECK(bind(bsock, (struct sockaddr*)&addr, addr_len) == -1, bind, 0);
    int ld1 = 1;
    CHECK(setsockopt(bsock, SOL_SOCKET, SO_BROADCAST, &ld1, sizeof(ld1)) == -1, setsockopt, 0);
    addr.sin_addr.s_addr = htonl(-1);
    addr.sin_port = htons(BROADCAST_PORT);

    while (1) {
        sent_bytes = sendto(bsock, args, sizeof(struct net_msg), 0, (struct sockaddr*)&addr, addr_len);
        CHECK(sent_bytes == -1, sendto, 0);
        CHECK(sent_bytes != sizeof(struct net_msg), message sending, EINVAL);
        printf ("Sent %d bytes\n", sent_bytes);
        sleep(1);
    }

    args = NULL;
}


void wait_for_clients() {
    fd_set fds;
    int clients_count = 0, cores_info = 0, maxsd;
    struct net_msg recv_msg;

    printf ("Waiting for clients (port %d)\n", broadcast_msg.tcp_port);
    cores_total = 0;

    while (cores_info < clients_max) {
        FD_ZERO(&fds);
        FD_SET(master, &fds);
        maxsd = master;
        for (int i = 0; i < clients_max; ++i) {
            if (client[i]) {
                FD_SET(client[i], &fds);
                if (client[i] > maxsd)
                    maxsd = client[i];
            }
        }

        if (clients_count == clients_max) {
            FD_CLR(master, &fds);
            shutdown(master, SHUT_RDWR);
            close(master);
        }

        CHECK ((select(maxsd+1, &fds, NULL, NULL, NULL) == -1) && (errno != EINTR), select, 0);

        if (FD_ISSET(master, &fds)) {
            printf ("Event #%d\n", master);
            int new = accept(master, (struct sockaddr*)&addr, &addr_len);
            CHECK(new == -1, accept, 0);
            printf ("New connection (%d) [%s:%d]\n", clients_count, inet_ntoa(addr.sin_addr), ntohs(addr.sin_port));

            for (int i = 0; i < clients_max; ++i)
                if (!client[i]) {
                    client[i] = new;
                    break;
                }
            clients_count += 1;
        }

        for (int i = 0; i < clients_max; ++i)
            if (FD_ISSET(client[i], &fds)) {
                printf ("Event @%d\n", client[i]);
                bytes = read(client[i], &recv_msg, sizeof(struct net_msg));
                CHECK(bytes == -1, read, 0);
                CHECK (!bytes || bytes != sizeof(struct net_msg), message receiving, EINVAL);
                cores[i] = recv_msg.cores;
                cores_info += 1;
                cores_total += recv_msg.cores;
                printf ("Client %d has %d core%s\n", i, cores[i], ((cores[i] > 1) ? "s" : ""));
            }
    }

    printf ("Ready for calculation\n");
}

