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

#include "net/sim7020.h"

#ifdef BOARD_AVR_RSS2
#include "pstr_print.h"
#endif

typedef enum {
    s_traffic, s_delay,
} sim7020_report_state_t;

static int stats(char *str, size_t len, uint8_t *finished) {
    char *s = str;
    size_t l = len;
    static sim7020_report_state_t state = s_traffic;
    sim7020_netstats_t *ns;
    int nread = 0;
  
    *finished = 0;
    
    switch (state) {
    case s_traffic:
        ns = sim7020_get_netstats();

        RECORD_START(s + nread, l - nread);
        PUTFMT(",{\"n\":\"sim7020;stats;\",\"vj\":[");
        PUTFMT("{\"n\":\"tx\",\"u\":\"count\",\"v\":%" PRIu32 "},", ns->tx_success);
        PUTFMT("{\"n\":\"tx_failed\",\"u\":\"count\",\"v\":%" PRIu32 "},", ns->tx_failed);
        PUTFMT("{\"n\":\"tx\",\"u\":\"byte\",\"v\":%" PRIu32 "},", ns->tx_bytes);
        PUTFMT("{\"n\":\"rx\",\"u\":\"count\",\"v\":%" PRIu32 "},", ns->rx_count);
        PUTFMT("{\"n\":\"rx\",\"u\":\"byte\",\"v\":%" PRIu32 "},", ns->rx_bytes);
        PUTFMT("{\"n\":\"commfail\",\"u\":\"count\",\"v\":%" PRIu32 "},", ns->commfail_count);
        PUTFMT("{\"n\":\"reset\",\"u\":\"count\",\"v\":%" PRIu32 "},", ns->reset_count);
        PUTFMT("{\"n\":\"activation_fail\",\"u\":\"count\",\"v\":%" PRIu32 "}", ns->activation_fail_count);
        PUTFMT("]}");
        RECORD_END(nread);
        state = s_delay;
    case s_delay:
        RECORD_START(s + nread, l - nread);
        PUTFMT(",{\"n\":\"sim7020;delay;\",\"vj\":[");
        int first = 1;
        {
            extern uint32_t sim7020_activation_usecs;
            if (sim7020_active()) {
                if (first)
                    first = 0;
                else
                    PUTFMT(",");
                PUTFMT("{\"n\":\"activation_time\",\"u\":\"msec\",\"v\":%" PRIu32 "}", sim7020_activation_usecs/1000);
            }
        }
        {
            extern uint64_t sim7020_prev_active_duration_usecs;
            if (sim7020_prev_active_duration_usecs != 0) {
                if (first)
                    first = 0;
                else
                    PUTFMT(",");
                PUTFMT("{\"n\":\"prev_duration\",\"u\":\"msec\",\"v\":%" PRIu32 "}", (uint32_t) (sim7020_prev_active_duration_usecs/1000));
            }
        }
        PUTFMT("]}");
        RECORD_END(nread);     
        state = s_traffic;
    }
    *finished = 1;

    return nread;
}


typedef enum {s_stats} sim7020_state_t;

int sim7020_report(uint8_t *buf, size_t len, uint8_t *finished) {
    char *s = (char *) buf;
    size_t l = len;
    int nread = 0, n;
    static sim7020_state_t state = s_stats;

    *finished = 0;

    switch (state) {
    case s_stats:
        n = stats(s + nread, l - nread, finished);
        if (n == 0)
            return (nread);
        nread += n;
        state = s_stats;
    }
    *finished = 1;
    return nread;
}

