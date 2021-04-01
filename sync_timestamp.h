#ifndef SYNC_TIMESTAMP_H
#include "net/sntp.h"

#define NTPP_SYNC_INTERVAL 120 /* seconds */
#define NTPTIMEOUT 10000000

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
