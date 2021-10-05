/*
 * Copyright (C) 2020 Peter Sjödin, KTH
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "xtimer.h"
#include "timex.h"

#ifdef MODULE_SIM7020
#include "net/af.h"
#include "net/ipv4/addr.h"
#include "net/ipv6/addr.h"
#include "net/sock/udp.h"

#include "net/sim7020.h"
#endif /* MODULE_SIM7020 */

#include "report.h"

#ifdef BOARD_AVR_RSS2
#include "pstr_print.h"
#endif

#include "app_watchdog.h"

static uint16_t consec_fails;
static uint32_t last_recovery;

static struct {
    uint32_t noprogress;                 /* No. of lack-of-progress events */
    uint32_t recovery;                   /* No. of recoveries */
} awd_stats;

static int awd_should_recover(void) {
    if (consec_fails > APP_WATCHDOG_MAX_CONSEC_FAILS) {
        uint32_t now = xtimer_now_usec();
        if ((now - last_recovery)/US_PER_SEC >= APP_WATCHDOG_MIN_RECOVERY_INTERVAL_SEC) {
            return 1;
        }
    }
    return 0;
}

static void awd_recovery(void) {
    awd_stats.recovery++;
    consec_fails = 0;
    last_recovery = xtimer_now_usec();
#ifdef MODULE_SIM7020
    sim7020_reset();
#endif
}

void app_watchdog_update(int progress) {
    if (progress) 
        consec_fails = 0;
    else {
        consec_fails++;
        awd_stats.noprogress++;
        if (awd_should_recover()) {
            awd_recovery();
        }
    }
}

int app_watchdog_report(uint8_t *buf, size_t len, uint8_t *finished, 
                        __attribute__((unused)) char **topicp, __attribute__((unused)) char **basenamep) {
     char *s = (char *) buf;
     size_t l = len;
     int nread = 0;
     
     *finished = 0;
     RECORD_START(s + nread, l - nread);
     PUTFMT(",{\"n\":\"appwd;stats;\",\"vj\":[");
     PUTFMT("{\"n\":\"noprogress\",\"u\":\"count\",\"v\":%d},", awd_stats.noprogress);
     PUTFMT("{\"n\":\"recovery\",\"u\":\"count\",\"v\":%d}", awd_stats.recovery);
     PUTFMT("]}");
     RECORD_END(nread);
     *finished = 1;

     return nread;
}
