#ifndef EEDATA_H
#define EEDATA_H

/*
 * Functions to read and store a block of data in eeprom. 
 * The data block starts with a 32-bit hash, which is used
 * as a checksom to check the validy of the data in eeprom. 
 * If the hash does not match the data block, the block is
 * not valid
 */ 

typedef uint32_t eehash_t; /* Hash stored in first 32 bits */

int read_eeprom(void *ram_dst, void *eeprom_src, size_t len);
void update_eeprom(void *ram_src, void *eeprom_dst, size_t len);
void erase_eeprom(void *eeprom_dst, __attribute__((unused)) size_t len);
#endif /* EEDATA_H */
