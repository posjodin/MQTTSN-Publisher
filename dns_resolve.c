#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <avr/eeprom.h>

#include "net/ipv6/addr.h"
#if defined(MODULE_SOCK_DNS)
#include "net/sock/dns.h"
#else
#include "net/sock/dns.h"
#endif
#include "xtimer.h"
#include "hashes.h"

#ifdef APP_WATCHDOG
#include "app_watchdog.h"
#endif /* APP_WATCHDOG */
#ifdef BOARD_AVR_RSS2
#include "pstr_print.h"
#endif

#include "dns_resolve.h"

#define MAX_HOSTNAME_LENGTH 64
struct dns_cache {
    char host[MAX_HOSTNAME_LENGTH];
    ipv6_addr_t ipv6addr;
    uint32_t time_usec;
    enum {
        UNUSED,
        RESOLVED
    } state;
};
typedef struct dns_cache dns_cache_t;

static dns_cache_t dns_resolve_cache[DNS_CACHE_SIZE];

static EEMEM dns_cache_t ee_dns_resolve_cache[DNS_CACHE_SIZE];
static EEMEM uint32_t ee_hash;

/*
 * Read DNS cache from EEPROM. Use a hash in EEPROM to verify that the info in the
 * EEPROM is valid. Read info, compute hash, and compare with
 * hash in EEPROM. Return non-zero if hashes match (ie info in EEPROM
 * is valid).
 */
static int read_eeprom(void) {
    eeprom_read_block(&dns_resolve_cache, &ee_dns_resolve_cache, sizeof(dns_resolve_cache));
    uint32_t ehash = eeprom_read_dword(&ee_hash);
    uint32_t hash = dek_hash((uint8_t *) &dns_resolve_cache, sizeof(dns_resolve_cache));
    return ehash == hash;
}

/*
 * Update cache and its hash in EEPROM
 */
static void update_eeprom(void) {
    eeprom_update_block(&dns_resolve_cache, &ee_dns_resolve_cache, sizeof(dns_resolve_cache));
    uint32_t hash = dek_hash((uint8_t *) &dns_resolve_cache, sizeof(dns_resolve_cache));
    eeprom_update_dword(&ee_hash, hash);
}

/*
 * Init cache. Read from EEPROM. If EEPROM info is not valid, mark all places as unused.
 * Cache will be written back to EEPROM when it is updated.
 */
void dns_resolve_init(void) {
    dns_cache_t *dc;
    if (read_eeprom() == 0) {
        for (dc = &dns_resolve_cache[0]; dc <= &dns_resolve_cache[DNS_CACHE_SIZE-1]; dc++) {
            dc->state = UNUSED;
        }
    }
}

/*
 * Is host in cache?
 */
static dns_cache_t *cache_lookup(char *host) {
    dns_cache_t *dc;
    for (dc = &dns_resolve_cache[0]; dc <= &dns_resolve_cache[DNS_CACHE_SIZE-1]; dc++) {
        if ((dc->state == RESOLVED) && (strncmp(dc->host, host, MAX_HOSTNAME_LENGTH) == 0))
            return dc;
    }
    return NULL;
}

/*
 * Find an unused cache entry. If none exists, expire oldest entry and use that.
 */
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

/*
 * Update cache info after lookup, and update EEPROM as well.
 * TODO: use epoch time, not local timestamp.
 */
static void cache_update(char *host, ipv6_addr_t *result) {
    dns_cache_t *cache_entry = cache_lookup(host);
    if (cache_entry == NULL)
        cache_entry = cache_alloc();
    if (cache_entry != NULL) {
        strncpy(cache_entry->host, host, sizeof(cache_entry->host));
        cache_entry->ipv6addr = *result;
        cache_entry->state = RESOLVED;
        cache_entry->time_usec = xtimer_now_usec();
        update_eeprom();
    }
}

/*
 * Resolve host name to IP address, and update cache if successful.
 */
static int _resolve_inetaddr(char *host, ipv6_addr_t *result) {
#if defined(MODULE_SOCK_DNS) || defined(MODULE_SIM7020_SOCK_DNS)
#ifdef DNS_RESOLVER
    sock_dns_server.family = AF_INET6;
    sock_dns_server.port = 53;
    if (ipv6_addr_from_str((ipv6_addr_t *)&sock_dns_server.addr.ipv6, DNS_RESOLVER) == NULL) {
         printf("Bad resolver %s\n", DNS_RESOLVER);
         return -1;
    }
#endif /* DNS_RESOLVER */
    result->u64[0].u64 = 0;
    result->u16[4].u16 = 0;
    result->u16[5].u16 = 0xffff;
    int res = sock_dns_query(host, &result->u32[3].u32, AF_INET);
    if (res >= 0) {
        /* Cache result */
        cache_update(host, result);
    }
#ifdef APP_WATCHDOG
    app_watchdog_update(res >= 0);
#endif /* APP_WATCHDOG */
    return res;
#else
    return -1;
#endif
}

/*
 * Resolve host. If IPv6 address string, convert to IPv6
address and return. Otherwise, check if host is in the
cache already. If so, return address in cache. If it is
not in cache, make a DNS query.
*/
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

/*
 * Cache refresh. Check if info in cache is old and should
 * be updated.
 * We do not want to be stuck in here too long, so only
 * update one entry at a time.
 */
void dns_resolve_refresh(void) {
    dns_cache_t *dc;
    for (dc = &dns_resolve_cache[0]; dc <= &dns_resolve_cache[DNS_CACHE_SIZE-1]; dc++) {
        if (dc->state == RESOLVED) {
            uint32_t now = xtimer_now_usec();
            if (now - dc->time_usec >= DNS_CACHE_REFRESH_PERIOD) {
                ipv6_addr_t dummy;
                _resolve_inetaddr(dc->host, &dummy);
                /* Only one refresh attempt per invocation */
                return;
            }
        }
    }
}

