#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <avr/eeprom.h>

#include "net/ipv6/addr.h"
#if defined(MODULE_SOCK_DNS)
#include "net/sock/dns.h"
#else
#include "net/sock/dns.h"
#endif
#include "xtimer.h"
#include "hashes.h"

#include "eedata.h"

#ifdef APP_WATCHDOG
#include "app_watchdog.h"
#endif /* APP_WATCHDOG */
#ifdef BOARD_AVR_RSS2
#include "pstr_print.h"
#endif

static void hdump(uint8_t data[], size_t len) {
    size_t i;
    for (i = 0; i < len; i++)
        printf("%02x ", (uint8_t) data[i]);
}

int read_eeprom(void *ram_dst, void *eeprom_src, size_t len) {
    uint8_t dumbuf[128];
    eeprom_read_block(dumbuf, eeprom_src, len+sizeof(eehash_t)*2);
    hdump(dumbuf,  len+sizeof(eehash_t)*2);
    printf("\n\n");
    void *data = ((eehash_t *) eeprom_src) + 1;
    eeprom_read_block(ram_dst, data, len);
    eehash_t ehash = eeprom_read_dword((eehash_t *) eeprom_src);
    uint32_t hash = dek_hash((uint8_t *) ram_dst, len);
    hdump(ram_dst, len);
    printf("\n");
    hdump((uint8_t *)&ehash, sizeof(ehash));
    printf("\n");
    hdump((uint8_t *)&hash, sizeof(hash));    
    printf("\n");
    return ehash == hash;
}

void update_eeprom(void *ram_src, void *eeprom_dst, size_t len) {
    void *data = ((eehash_t *) eeprom_dst) + 1;
    eeprom_update_block(ram_src, data, len);
    uint32_t hash = dek_hash((uint8_t *) ram_src, len);
    hdump((uint8_t *)&hash, sizeof(hash));
    printf("\n");
    eeprom_update_dword((eehash_t *) eeprom_dst, hash);
    printf("Rereading...\n\n");
    uint8_t dumbuf[128];
    read_eeprom(dumbuf, eeprom_dst, len);
}

/*
 * Update cache and its hash in EEPROM

static void update_eeprom(void) {
    eeprom_update_block(&dns_resolve_cache, &ee_dns_resolve_cache, sizeof(dns_resolve_cache));
    uint32_t hash = dek_hash((uint8_t *) &dns_resolve_cache, sizeof(dns_resolve_cache));
    eeprom_update_dword(&ee_hash, hash);
}
 */
