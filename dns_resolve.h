#ifndef DNS_RESOLVE_H
#define DNS_RESOLVE_H

#define DNS_CACHE_SIZE 4

#define DNS_CACHE_REFRESH
#define DNS_CACHE_REFRESH_PERIOD (2UL*3600*1000*1000) /* 2 hours in microseconds */

void dns_resolve_init(void);
int dns_resolve_inetaddr(char *host, ipv6_addr_t *result);
void dns_resolve_refresh(void);

#endif /* DNS_RESOLVE_H */
