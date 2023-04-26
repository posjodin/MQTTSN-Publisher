#ifndef EEDATA_H
#define EEDATA_H

typedef uint32_t eehash_t;

int read_eeprom(void *ram_dst, void *eeprom_src, size_t len);
void update_eeprom(void *ram_src, void *eeprom_dst, size_t len);
#endif /* EEDATA_H */
