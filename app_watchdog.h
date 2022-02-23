#ifndef APP_WATCHDOG_H


/* Number of failed communications before recovery */
#ifndef APP_WATCHDOG_CONSEC_FAILS
#define APP_WATCHDOG_CONSEC_FAILS 16
#endif /* APP_WATCHDOG_CONSEC_FAILS */

/* Min interval between recoveries */
#ifndef APP_WATCHDOG_MIN_RECOVERY_INTERVAL_SEC
#define APP_WATCHDOG_MIN_RECOVERY_INTERVAL_SEC 600
#endif /* APP_WATCHDOG_MIN_RECOVERY_INTERVAL_SEC */

/* Number of recoveries before rebooting */
#ifndef APP_WATCHDOG_MAX_RECOVERIES
#define APP_WATCHDOG_MAX_RECOVERIES 9
#endif /* APP_WATCHDOG_MAX_RECOVERIES */

void app_watchdog_init(void);
void app_watchdog_update(int progress);
int app_watchdog_report(uint8_t *buf, size_t len, uint8_t *finished, char **topicp, char **basenamep);

#endif /* APP_WATCHDOG_H */
