#include <string.h>

#include "net/sock/udp.h"
#include "xtimer.h"
#include "mutex.h"
#include "byteorder.h"

#include "dns_resolve.h"

#define UPING_THREAD

#define ENABLE_DEBUG    (0)
#include "debug.h"
#define UPING_DATA 256

static sock_udp_t _uping_sock;
typedef struct uping_packet {
  uint16_t seqno;
  uint8_t data[UPING_DATA];
} uping_packet_t;

static uping_packet_t _uping_packet;
static uint16_t seqno;

int uping(sock_udp_ep_t *server, uint32_t timeout)
{
    int result;

    if ((result = sock_udp_create(&_uping_sock,
                                  NULL,
                                  server,
                                  0)) < 0) {
        DEBUG("Error creating UDP sock\n");
        return result;
    }
    unsigned int i;
    _uping_packet.seqno = seqno++;
    for (i = 0; i < UPING_DATA; i++)
        _uping_packet.data[i] = 'a' + (i % ('z'-'a'));


    if ((result = (int)sock_udp_send(&_uping_sock,
                                     &_uping_packet,
                                     sizeof(_uping_packet),
                                     NULL)) < 0) {
        DEBUG("Error sending message\n");
        sock_udp_close(&_uping_sock);
        return result;
    }
    if ((result = (int)sock_udp_recv(&_uping_sock,
                                     &_uping_packet,
                                     sizeof(_uping_packet),
                                     timeout,
                                     NULL)) < 0) {
        DEBUG("Error receiving message\n");
        sock_udp_close(&_uping_sock);
        return result;
    }
    sock_udp_close(&_uping_sock);
    return 0;
}

static sock_udp_ep_t server;
static int count = 1;

#ifdef UPING_THREAD
static int doping;
static void uping_thread_start(void);
#endif /* UPING_THREAD */

int cmd_uping(int argc, char **argv) {
    uint32_t timeout = 5*US_PER_SEC;
    
#ifdef UPING_THREAD
    if (argc == 2) {
        if (strcmp(argv[1], "start") == 0) {
            doping = 1;
            uping_thread_start();
            return 0;
        }
        else if (strcmp(argv[1], "stop") == 0) {
            doping = 0;
            return 0;
        }
    }
#endif /* UPING_THREAD */

    if (argc < 3)
        goto usage;

    server.family = AF_INET6;
    if (sscanf(argv[2], "%" SCNu16, &server.port) != 1)
        goto usage;
    if (argc == 4) {
        if (sscanf(argv[3], "%d", &count) != 1)
            goto usage;
    }
    if (sscanf(argv[2], "%" SCNu16, &server.port) != 1)
        goto usage;

    char *host = argv[1];
    int res = dns_resolve_inetaddr(host, (ipv6_addr_t *) &server.addr);
    if (res !=0) {
        printf("resolve failed\n");
        return res;
    }
    int no;
    for (no = 1; no <= count; no++) {
        res = uping(&server, timeout);
        if (res == 0)
            printf("%d: ok\n", no);
        else
            printf("%d: fail\n");
    }
    return 0;
usage:
    printf("argc %d\n", argc);
    printf("Usage: uping <host> <port> [<count>]\n");
    printf("or: uping start|stop\n");
    return -1;  
}

#ifdef UPING_THREAD
#define UPING_INTERVAL (30)
#define UPING_TIMEOUT (5*US_PER_SEC)

#define UPING_STACK THREAD_STACKSIZE_MAIN
#define UPING_PRIORITY (THREAD_PRIORITY_MAIN-1)
static char uping_thread_stack[UPING_STACK];
static kernel_pid_t uping_pid = KERNEL_PID_UNDEF;

static void *uping_thread( __attribute__((unused)) void *arg) {
    printf("Here is uping thread\n");
    while (1) {
        if (doping) {
            int i;
            for (i = 0; doping && i < count; i++) {
                int res = uping(&server, UPING_TIMEOUT);
                if (res != 0)
                    printf("%d: ---\n", seqno);
                else
                    printf("%d: ping\n", seqno);
            }
        }
        xtimer_sleep(UPING_INTERVAL);
    }
    return NULL;
}

static void uping_thread_start(void) {
    if (uping_pid == KERNEL_PID_UNDEF) {
        uping_pid = thread_create(uping_thread_stack, sizeof(uping_thread_stack),
                                  UPING_PRIORITY, THREAD_CREATE_STACKTEST,
                                  uping_thread, NULL, "uping");
    }
}
#endif /* UPING_THREAD */
