/*
 * Copyright (C) 2020 Peter Sjödin, KTH
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#include "net/af.h"
#include "net/ipv4/addr.h"
#include "net/ipv6/addr.h"
#include "net/sock/udp.h"

#include "report.h"

#include "net/netif.h"

#ifdef MODULE_NETSTATS
#include "net/netstats.h"
#endif /* MODULE_NETSTATS */

#ifdef BOARD_AVR_RSS2
#include "pstr_print.h"
#endif

#ifdef MODULE_NETSTATS

static int iface_name(netif_t *iface, char *str, size_t len) {
    char name[CONFIG_NETIF_NAMELENMAX];
    netif_get_name(iface, name);
    strncpy(str, name, len < sizeof(name) ? len : sizeof(name));
    return strlen(str);
}

static int stats(netif_t *iface, char *str, size_t len, __attribute__((unused)) uint8_t *finished) {
    char *s = str;
    size_t l = len;
    int nread = 0;

    netstats_t *netstats;
    int res = netif_get_opt(iface, NETOPT_STATS, NETSTATS_LAYER2, &netstats,
                            sizeof(&netstats));

    RECORD_START(s + nread, l - nread);
    PUTFMT(",{\"n\":\"netif;");
    RECORD_ADD((unsigned) iface_name(iface, RECORD_STR(), RECORD_LEN()));
    PUTFMT(";stats;\",\"vj\":[");
    if (res >= 0) {
        PUTFMT("{\"n\":\"rx_bytes\",\"u\":\"count\",\"v\":%" PRIu32 "},", netstats->rx_bytes);
        PUTFMT("{\"n\":\"rx\",\"u\":\"count\",\"v\":%" PRIu32 "},", netstats->rx_count);
        uint32_t tx_count = netstats->tx_unicast_count + netstats->tx_mcast_count;
        PUTFMT("{\"n\":\"tx\",\"u\":\"count\",\"v\":%" PRIu32 "}", tx_count);
    }
    PUTFMT("]}");
    uint16_t u16;
    res = netif_get_opt(iface, NETOPT_CHANNEL, 0, &u16, sizeof(u16));
    if (res >= 0) {
        PUTFMT(",{\"n\":\"netif;");
        RECORD_ADD((unsigned) iface_name(iface, RECORD_STR(), RECORD_LEN()));
        PUTFMT(";chan;\",\"v\":%" PRIu16 "}", u16);
    }
    int8_t i8;
    res = netif_get_opt(iface, NETOPT_RSSI, 0, &i8, sizeof(i8));
    if (res >= 0) {
        PUTFMT(",{\"n\":\"netif;");
        RECORD_ADD((unsigned) iface_name(iface, RECORD_STR(), RECORD_LEN()));
        PUTFMT(";rssi;\",\"v\":%" PRIi8 "}", i8);
    }
    RECORD_END(nread);

    return nread;
}

int if_report(uint8_t *buf, size_t len, uint8_t *finished, 
                   __attribute__((unused)) char **topicp, __attribute__((unused)) char **basenamep) {
    char *s = (char *) buf;
    size_t l = len;
    int nread = 0, n;

    static netif_t *netif = NULL;
    
    *finished = 0;
    if (l == 0) {
        // Zero data len -- to get topic/basename, just use default
        return 0;
    }
    while ((netif = netif_iter(netif))) {
        n = stats(netif, s + nread, l - nread, finished);
        if (n == 0)
            return (nread);
        nread += n;
    }
    *finished = 1;
    netif = NULL;
    return nread;
}
#endif /* MODULE_NETSTATS */
