#ifndef APP_WATCHDOG_H

#ifndef APP_WATCHDOG_MAX_CONSEC_FAILS
#define APP_WATCHDOG_MAX_CONSEC_FAILS 8
#endif /* APP_WATCHDOG_MAX_CONSEC_FAILS */

#ifndef APP_WATCHDOG_MIN_RECOVERY_INTERVAL_SEC
#define APP_WATCHDOG_MIN_RECOVERY_INTERVAL_SEC 600
#endif /* APP_WATCHDOG_MIN_RECOVERY_INTERVAL_SEC */

void app_watchdog_update(int progress);
int app_watchdog_report(uint8_t *buf, size_t len, uint8_t *finished, char **topicp, char **basenamep);

#endif /* APP_WATCHDOG_H */
