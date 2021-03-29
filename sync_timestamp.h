#ifndef SMTP_TIMESTAMP_H

#define NTPP_SYNC_INTERVAL 120 /* seconds */
#define NTPTIMEOUT 10000000

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

#define SMTP_TIMESTAMP_H
#endif /* SMTP_TIMESTAMP_H */
