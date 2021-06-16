#include <stdio.h>
#include <stdlib.h>

#include "periph/pm.h"

#include "mqttsn_publisher.h"
#ifdef MODULE_SIM7020
#include "net/sim7020.h"
#endif /* MODULE_SIM7020 */

#include "watchdog.h"

static uint16_t fails; /* Consecutive fails */
static uint16_t reset_fails; /* Consecutive fails since last reset */

void _watchdog_fail(void) {
    fails++;
    reset_fails++;
    printf("WD fail %u %u\n", reset_fails, fails);
    if (fails > WATCHDOG_REBOOT_FAILS) {
        printf("Reboot after %u fails\n", fails);
        pm_reboot();
    }
    if (reset_fails > WATCHDOG_RESET_FAILS) {
        mqttsn_stats.commreset++;
        printf("Reset comm after %u fails\n", reset_fails);
        reset_fails = 0;
#ifdef MODULE_SIM7020
        sim7020_reset();
#endif /* MODULE_SIM7020 */
    }
}

void _watchdog_update(void) {
    printf("WD clear\n");
    fails = 0;
    reset_fails = 0;
}
