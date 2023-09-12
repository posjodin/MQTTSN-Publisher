#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include <avr/eeprom.h>

#include "hashes.h"

#include "eedata.h"

static inline uint32_t eehash(uint8_t *data, size_t len) {
    return djb2_hash(data, len);
}

int read_eeprom(void *ram_dst, void *eeprom_src, size_t len) {
    void *data = ((eehash_t *) eeprom_src) + 1;
    eeprom_read_block(ram_dst, data, len);
    eehash_t ehash = eeprom_read_dword((eehash_t *) eeprom_src);
    uint32_t hash = eehash((uint8_t *) ram_dst, len);
    return ehash == hash;
}

void update_eeprom(void *ram_src, void *eeprom_dst, size_t len) {
    void *data = ((eehash_t *) eeprom_dst) + 1;
    eeprom_update_block(ram_src, data, len);
    uint32_t hash = eehash((uint8_t *) ram_src, len);
    eeprom_update_dword((eehash_t *) eeprom_dst, hash);
}

/*
 * Invalidate eeprom block by inverting hash
 */
void erase_eeprom(void *eeprom_dst, __attribute__((unused)) size_t len) {
    eehash_t ehash = eeprom_read_dword((eehash_t *) eeprom_dst);
    eeprom_update_dword((eehash_t *) eeprom_dst, ~ehash);
}


/*
 * Update cache and its hash in EEPROM

static void update_eeprom(void) {
    eeprom_update_block(&dns_resolve_cache, &ee_dns_resolve_cache, sizeof(dns_resolve_cache));
    uint32_t hash = dek_hash((uint8_t *) &dns_resolve_cache, sizeof(dns_resolve_cache));
    eeprom_update_dword(&ee_hash, hash);
}
 */
