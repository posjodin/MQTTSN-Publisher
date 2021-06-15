#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "net/ipv6/addr.h"
#if defined(MODULE_SOCK_DNS)
#include "net/sock/dns.h"
#else
#include "net/sock/dns.h"
#endif
#include "xtimer.h"

#ifdef BOARD_AVR_RSS2
#include "pstr_print.h"
#endif

#include "dns_resolve.h"

#define MAX_HOSTNAME_LENGTH 64
struct dns_cache {
    char *host;
    ipv6_addr_t ipv6addr;
    uint32_t time_usec;
    enum {
        UNUSED,
        RESOLVED
    } state;
    
};
typedef struct dns_cache dns_cache_t;

static dns_cache_t dns_resolve_cache[DNS_CACHE_SIZE];

void dns_resolve_init(void) {
    dns_cache_t *dc;
    for (dc = &dns_resolve_cache[0]; dc <= &dns_resolve_cache[DNS_CACHE_SIZE-1]; dc++)
        dc->state = UNUSED;
}

static dns_cache_t *cache_lookup(char *host) {
    dns_cache_t *dc;
    for (dc = &dns_resolve_cache[0]; dc <= &dns_resolve_cache[DNS_CACHE_SIZE-1]; dc++) {
        if ((dc->state == RESOLVED) && (strncmp(dc->host, host, MAX_HOSTNAME_LENGTH) == 0))
            return dc;
    }
    return NULL;
}

static dns_cache_t *cache_alloc(void) {
    dns_cache_t *dc, *oldest;
    oldest = NULL;
    for (dc = &dns_resolve_cache[0]; dc <= &dns_resolve_cache[DNS_CACHE_SIZE-1]; dc++) {
        if (dc->state == UNUSED) {
            return dc;
        }
        if (oldest == NULL || dc->time_usec < oldest->time_usec)
            oldest = dc;
    }
    return oldest;
}

static int _resolve_inetaddr(char *host, ipv6_addr_t *result) {
#if defined(MODULE_SOCK_DNS) || defined(MODULE_SIM7020_SOCK_DNS) 
#ifdef DNS_RESOLVER
    sock_dns_server.family = AF_INET6;
    sock_dns_server.port = 53;    
    if (ipv6_addr_from_str((ipv6_addr_t *)&sock_dns_server.addr.ipv6, DNS_RESOLVER) == NULL) {
         printf("Bad resolver address %s\n", DNS_RESOLVER);
         return -1;
    }
#endif /* DNS_RESOLVER */
    result->u64[0].u64 = 0;
    result->u16[4].u16 = 0;
    result->u16[5].u16 = 0xffff;

    int res = sock_dns_query(host, &result->u32[3].u32, AF_INET);
    if (res >= 0) {
        /* Cache result */
        dns_cache_t *cache_entry = cache_lookup(host);
        if (cache_entry == NULL)
            cache_entry = cache_alloc();
        if (cache_entry != NULL) {
            cache_entry->host = host;
            cache_entry->ipv6addr = *result;
            cache_entry->state = RESOLVED;
            cache_entry->time_usec = xtimer_now_usec();
        }
    }
    return res;
#else
    return -1;
#endif    
}

int dns_resolve_inetaddr(char *host, ipv6_addr_t *result) {
    /* Is host a v6 address? */
    if (ipv6_addr_from_str(result, host) != NULL) {
        return 0;
    }
    /* Is the result in cache? */
    dns_cache_t *cache_entry = cache_lookup(host);
    if (cache_entry != NULL) {
        *result = cache_entry->ipv6addr;
        return 0;
    }
    /* Resolve the name */
    return _resolve_inetaddr(host, result);
}

void dns_resolve_refresh(void) {
    dns_cache_t *dc;
    for (dc = &dns_resolve_cache[0]; dc <= &dns_resolve_cache[DNS_CACHE_SIZE-1]; dc++) {
        if (dc->state == RESOLVED) {
            uint32_t now = xtimer_now_usec();
            if (now - dc->time_usec >= DNS_CACHE_REFRESH_PERIOD) {
                ipv6_addr_t dummy;
                _resolve_inetaddr(dc->host, &dummy);
                /* Only one refresh per invocation */
                return;
            }
        }
    }
}

