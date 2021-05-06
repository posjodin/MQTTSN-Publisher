#include "timex.h"
#ifdef MODULE_SNTP
#include "net/sntp.h"
#endif /* MODULE_SNTP */
#ifdef MODULE_SIM7020
#include "net/sock/udp.h"
#include "net/sim7020.h"
#endif /* MODULE_SIM7020 */
#include "mqttsn_publisher.h"
#include "sync_timestamp.h"

#ifdef MODULE_SNTP
static char *ntp_hosts[] = {"0.se.pool.ntp.org", "::ffff:5bd1:0014"};
static char **current_ntp_hostp = NULL;

/* 
 * How long to wait for response from NTP server 
 */
#define SYNC_SNTP_TIMEOUT 20000000
/*
 * Number of failed attempts before switching to other NTP server
 */
#define SYNC_SNTP_MAXATTEMPTS 5

/*
 * Time between sync refresh
 */
#define SYNC_INTERVAL_SECONDS 300

/* 
 * For how long a sync is considered valid 
 */
#define SYNC_VALID_SECONDS 24*60*1
/*
 * Time of last sync 
 */
static timex_t last_sync;
#endif /* MODULE_SNTP */

static timex_t basetime;

void sync_init(void) {
    xtimer_now_timex(&basetime);
}

/*
 * Get current basetime
 * Basetime is absolute unix time if there is a valid
 * NTP synchronization, otherwise local clock time.
 */
uint64_t sync_basetime(void) {
    uint64_t bt = timex_uint64(basetime);
    if (sync_has_sync()) {
        bt = sync_get_unix_ticks64(bt);
        assert(SYNC_GLOBAL_TIMESTAMP(bt));    
    }
    else {
        assert(!SYNC_GLOBAL_TIMESTAMP(bt));
    }
    return bt;
}

/*
 * Get offset of a 64-bit timestamp relative to basetime
 */
uint64_t sync_basetime_offset(uint64_t timestamp) {
    timex_t stamp, offset;
    stamp = timex_from_uint64(timestamp);
    if (timex_cmp(stamp, basetime) < 0) {
        /* No negative offset */
        return 0;
    }
    offset = timex_sub(stamp, basetime);
    return timex_uint64(offset);
}

#ifdef MODULE_SNTP

/*
 * We have a valid synchronization. 
 * Remember when this was
 */
static void sync_done(void) {
    xtimer_now_timex(&last_sync);
}

/*
 * Do we have a valid sync?
 */

int sync_has_sync(void) {
    timex_t now;
    if (last_sync.seconds == 0 && last_sync.microseconds == 0)
        return 0;
    xtimer_now_timex(&now);
    timex_t sync_valid_until = timex_add(last_sync, timex_set(SYNC_VALID_SECONDS, 0));
    return timex_cmp(now, sync_valid_until) <= 0;
}

/*
 * Time to sync with ntp server?
 */
static int timetosync(void) {
#ifdef MODULE_SIM7020
    if (!sim7020_active()) {
        return 0;
    }
#endif
    if (!sync_has_sync())
        return 1;

    timex_t now;
    xtimer_now_timex(&now);
    timex_t sync_due = timex_add(last_sync, timex_set(SYNC_INTERVAL_SECONDS, 0));
    return timex_cmp(now, sync_due) >= 0;
}

/*
 * Call ntp server 
 */
static int sync_with_server(char *host) {

    sock_udp_ep_t server = { .port = NTP_PORT, .family = AF_INET6 };
    int res;

    res = dns_resolve_inetaddr(host, (ipv6_addr_t *) &server.addr);
    if (res !=0) {
        printf("resolve failed\n");
        return res;
    }
    if ((res = sntp_sync(&server, SYNC_SNTP_TIMEOUT)) < 0) {
        printf("Error in synchronization: %d\n", res);
        return 1;
    }
    printf("Sync OK!\n");
    return 0;
}

/*
 * Sync clock with NTP server, if it is time
 */
void sync_periodic(void) {
    static unsigned attempts = 0;

    if (!timetosync())
        return;
    printf("Time to sync\n");
    if (current_ntp_hostp == NULL || *current_ntp_hostp == NULL)
        current_ntp_hostp = &ntp_hosts[0];
    int res = sync_with_server(*current_ntp_hostp);
    if (res == 0) {
        sync_done();
        return;
    }
    else {
        attempts++;
        if (attempts >= SYNC_SNTP_MAXATTEMPTS) {
            attempts = 0;
            current_ntp_hostp++;
            if (*current_ntp_hostp == NULL)
                current_ntp_hostp = &ntp_hosts[0];
        }
    }
}

/*
 * Get UTC unix time in microseconds
 */
uint64_t sync_get_unix_usec(void) {
  
    if (!sync_has_sync())
        return 0;
    union {
        struct {
            uint32_t lower;
            uint32_t upper;
        } hexval;
        uint64_t intval;
    }ts1;
    uint64_t ts = sntp_get_unix_usec();
    return ts;
    ts1.intval = ts;
    printf("Unix 64-bit: %" PRIx32 "%" PRIx32 "\n", ts1.hexval.lower, ts1.hexval.upper);

    uint32_t utime_sec = ts/1000000;
    uint32_t utime_msec = (ts/1000) % 1000;
    printf("UNix time: %" PRIu32 ".%03" PRIu32 " msec\n", utime_sec, utime_msec);
}
#else
/*
 * Periodic sync – nothing to do
 */
void sync_periodic(void) {
    return;
}

int sync_has_sync(void) {
    return 0;
}
#endif /* MODULE_SNTP */
