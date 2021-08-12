#ifndef SYNC_TIMESTAMP_H
#include "net/sntp.h"

/*
 * How long to wait for response from NTP server
 */
#ifndef SYNC_SNTP_TIMEOUT
#define SYNC_SNTP_TIMEOUT 20000000
#endif /* SYNC_SNTP_TIMEOUT */

/*
 * Number of failed attempts before switching to other NTP server
 */
#ifndef SYNC_SNTP_MAXATTEMPTS
#define SYNC_SNTP_MAXATTEMPTS 5
#endif /* SYNC_SNTP_MAXATTEMPTS */

/*
 * Time between sync refresh
 */
#ifndef SYNC_INTERVAL_SECONDS
#define SYNC_INTERVAL_SECONDS 3600
#endif /* SYNC_INTERVAL_SECONDS */

/*
 * For how long a sync is considered valid
 */
#ifndef SYNC_VALID_SECONDS
#define SYNC_VALID_SECONDS (48*60*60UL)
#endif /* SYNC_VALID_SECONDS */

/*
 * Init sync
 */
void sync_init(void);

/*
 * Sync clock with NTP server, if it is time
 */
void sync_periodic(void);

/*
 * Get UTC unix time in microseconds
 */
uint64_t sync_get_unix_usec(void);

/*
 * Is the clock valid?
 */
int sync_has_sync(void);

/**
 * @brief   Get Unix time from a 64-bit timestamp
 *
 * @return  Unix UTC time in microseconds
 */
static inline uint64_t sync_get_unix_ticks64(uint64_t microseconds)
{
    return (uint64_t)(sntp_get_offset() - (NTP_UNIX_OFFSET * US_PER_SEC) + microseconds);
}

/*
 * Get current basetime as 64-bit timestamp
 */
uint64_t sync_basetime(void);

/*
 * Get offset of a 64-bit timestamp relative to basetime
 */
uint64_t sync_basetime_offset(uint64_t timestamp);

#define SYNC_GLOBAL_TIMESTAMP(T) ((T) >= ((uint32_t) 1 << 28))
#define SYNC_TIMESTAMP_H
#endif /* SYNC_TIMESTAMP_H */
