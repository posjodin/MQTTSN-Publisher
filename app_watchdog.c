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

#include <avr/eeprom.h>

#include "xtimer.h"
#include "timex.h"

#ifdef MODULE_SIM7020
#include "net/af.h"
#include "net/ipv4/addr.h"
#include "net/ipv6/addr.h"
#include "net/sock/udp.h"

#include "net/sim7020.h"
#endif /* MODULE_SIM7020 */

#include "hashes.h"

#ifdef BOARD_AVR_RSS2
#include "pstr_print.h"
#endif
#include "periph/pm.h"
#ifdef WDT_WATCHDOG
#include "periph/wdt.h"
#endif
#include "report.h"
#include "sync_timestamp.h"

#include "app_watchdog.h"

static uint16_t consec_fails;
static uint32_t last_recovery;

static struct {
    uint32_t noprogress;                 /* No. of lack-of-progress events */
    uint32_t recovery;                   /* No. of recoveries */
} awd_stats;

typedef struct {
    uint32_t restarts;
    uint64_t last_timestamp;
} perm_awd_stats_t;

static perm_awd_stats_t perm_awd_stats;
static EEMEM perm_awd_stats_t ee_perm_awd_stats;
static EEMEM uint32_t ee_hash;

#ifdef APP_WATCHDOG_THREAD
#define APPWD_THREAD_PERIOD_SEC 2
#define APPWD_UPDATE_INTERVAL_SEC (5*SEC_PER_MIN)
#define APPWD_PRIO  (THREAD_PRIORITY_MIN - \
                     (SCHED_PRIO_LEVELS - 1))
#define APPWD_STACK THREAD_STACKSIZE_SMALL

static char appwd_stack[APPWD_STACK];
static void *appwd_thread(void *arg);

#endif /* APPWD_THREAD */

#ifdef WDT_WATCHDOG
#define WDT_MAX_MSEC 8000
#define WDT_PERIOD_MSEC 1000
#endif

/*
 * Read DNS cache from EEPROM. Use a hash in EEPROM to verify that the info in the
 * EEPROM is valid. Read info, compute hash, and compare with
 * hash in EEPROM. Return non-zero if hashes match (ie info in EEPROM
 * is valid).
 */

static int read_eeprom(void) {
    eeprom_read_block(&perm_awd_stats, &ee_perm_awd_stats, sizeof(perm_awd_stats));
    uint32_t eehash = eeprom_read_dword(&ee_hash);
    uint32_t hash = dek_hash((uint8_t *) &perm_awd_stats, sizeof(perm_awd_stats));
    return eehash == hash;
}

/*
 * Update cache and its hash in EEPROM
 */
static void update_eeprom(void) {
    eeprom_update_block(&perm_awd_stats, &ee_perm_awd_stats, sizeof(perm_awd_stats));
    uint32_t hash = dek_hash((uint8_t *) &perm_awd_stats, sizeof(perm_awd_stats));
    eeprom_update_dword(&ee_hash, hash);
}

void app_watchdog_init(void) {
    if (read_eeprom() == 0) {
        perm_awd_stats.restarts = 0;
        perm_awd_stats.last_timestamp = 0;        
    }
    else {
      uint32_t utime_sec = perm_awd_stats.last_timestamp/US_PER_SEC;
      uint32_t utime_msec = (perm_awd_stats.last_timestamp/MS_PER_SEC) % MS_PER_SEC;

      printf("Read AWD. Restarts %" PRIu32 " tstamp %" PRIu32 ".%" PRIu32 "\n",
             perm_awd_stats.restarts,
             utime_sec, utime_msec);
    }
#ifdef APP_WATCHDOG_THREAD
    kernel_pid_t appwd_pid = thread_create(appwd_stack, sizeof(appwd_stack), APPWD_PRIO, THREAD_CREATE_STACKTEST,
                              appwd_thread, NULL, "appwd");
    printf("start appdw: pid %d\n", appwd_pid);
#endif /* APP_WATCHDOG_THREAD */
#ifdef WDT_WATCHDOG
    wdt_setup_reboot(0, WDT_MAX_MSEC);
    wdt_start();
#endif /* WDT_WATCHDOG */
}

/*
 * Time for recovery action? Too many consecutive failures, and 
 * sufficiently long since last recovery?
 */
static int awd_should_recover(void) {
    if (consec_fails > APP_WATCHDOG_CONSEC_FAILS) {
        uint32_t now = xtimer_now_usec();
        if ((now - last_recovery)/US_PER_SEC >= APP_WATCHDOG_MIN_RECOVERY_INTERVAL_SEC) {
            return 1;
        }
    }
    return 0;
}

static void awd_reboot(void) {
#ifdef WDT_WATCHDOG
    wdt_setup_reboot(0, 500 /* msec */);
    wdt_start();
    irq_disable();
    while(1);
#endif /* WDT_WATCHDOG */
}

static void awd_restart(void) {
    perm_awd_stats.restarts++;
    uint64_t timestamp = xtimer_now_usec64();
    if (sync_has_sync())
        timestamp = sync_get_unix_ticks64(timestamp);
    perm_awd_stats.last_timestamp = timestamp;
    update_eeprom();
    awd_reboot(); /* bye */
}

/*
 * Recovery - reboot if too many recoveries already, otherwise
 * do hardware recovery 
 */
static void awd_recovery(void) {
    awd_stats.recovery++;
#ifdef APP_WATCHDOG_MAX_RECOVERIES
    if (awd_stats.recovery >= APP_WATCHDOG_MAX_RECOVERIES) {
        printf("Rebooting...\n"); xtimer_sleep(3);
        awd_restart(); /* No return */
    }
#endif /* APP_WATCHDOG_REBOOT_RECOVERIES */
    last_recovery = xtimer_now_usec();
#ifdef MODULE_SIM7020

    uint64_t tstamp = xtimer_now_usec64();
    if (sync_has_sync()) {
        tstamp = sync_get_unix_ticks64(tstamp);
    }
    printf("Restart SIM7020, recovery %d\n", awd_stats.recovery);
    /* Restart module */
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

#ifdef APP_WATCHDOG_THREAD
static void *appwd_thread(__attribute__((unused)) void *arg)
{
  unsigned int periods = 0;
    while (1) {
        xtimer_sleep(APPWD_THREAD_PERIOD_SEC);

#ifdef WDT_WATCHDOG
        wdt_kick();
#endif /* WDT_WATCHDOG */
        if (periods++ >= APPWD_UPDATE_INTERVAL_SEC/APPWD_THREAD_PERIOD_SEC) {
          periods = 0;
          if (consec_fails > 0)
            printf("APPWD_THREAD: %d fails\n", consec_fails+1);
          app_watchdog_update(0);
          printf("APPWD %d\n", consec_fails);
        }
    }
    return NULL;
}
#endif /* APPWD_THREAD */
 
int app_watchdog_report(uint8_t *buf, size_t len, uint8_t *finished, 
                        __attribute__((unused)) char **topicp, __attribute__((unused)) char **basenamep) {
     char *s = (char *) buf;
     size_t l = len;
     int nread = 0;
     
     *finished = 0;
     RECORD_START(s + nread, l - nread);
     PUTFMT(",{\"n\":\"appwd;stats;\",\"vj\":[");
     PUTFMT("{\"n\":\"noprogress\",\"u\":\"count\",\"v\":%" PRIu32 "},", awd_stats.noprogress);
     PUTFMT("{\"n\":\"recovery\",\"u\":\"count\",\"v\":%" PRIu32 "},", awd_stats.recovery);
     PUTFMT("{\"n\":\"restarts\",\"u\":\"count\",\"v\":%" PRIu32 "},", perm_awd_stats.restarts);
     uint32_t utime_sec = perm_awd_stats.last_timestamp/US_PER_SEC;
     uint32_t utime_msec = (perm_awd_stats.last_timestamp/MS_PER_SEC) % MS_PER_SEC;
     PUTFMT("{\"n\":\"restart_time\",\"v\":%" PRIu32 ".%03" PRIu32 "}", utime_sec, utime_msec);
     PUTFMT("]}");
     RECORD_END(nread);
     *finished = 1;

     return nread;
}
