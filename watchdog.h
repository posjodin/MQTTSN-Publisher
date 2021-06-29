#ifndef WATCHDOG_H
#define WATCHDOG_H

#ifndef WATCHDOG_REBOOT_FAILS
#define WATCHDOG_REBOOT_FAILS 80
#endif /* WATCHDOG_REBOOT_FAILS */
#ifndef WATCHDOG_RESET_FAILS
#define WATCHDOG_RESET_FAILS 20
#endif /* WATCHDOG_RESET_FAILS */

#ifndef WATCHDOG_ENABLE
#define WATCHDOG_ENABLE 0
#endif /* WATCHDOG_ENABLE */

void _watchdog_fail(void);
void _watchdog_update(void);
#if WATCHDOG_ENABLE
#define watchdog_fail(...) _watchdog_fail() 
#define watchdog_update(...) _watchdog_update() 
#else
#define watchdog_fail(...) 
#define watchdog_update(...) 
#endif /* WATCHDOG_ENABLE */

#endif /* WATCHDOG_H */
