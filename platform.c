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

#include "mqttsn_publisher.h"
#include "report.h"

#ifdef BOARD_AVR_RSS2
#include "pstr_print.h"
#endif

extern uint8_t soft_rst __attribute__((section(".noinit")));

static char * reset_cause(uint8_t reg)
{
    if (reg & (1 << PORF)) {
        return("Power-on reset");
    }
    if (reg & (1 << EXTRF)) {
        return("External reset");
    }
    if (reg & (1 << BORF)) {
        return("Brownout reset");
    }
    if (reg & (1 << WDRF)) {
        if (soft_rst & 0xAA) {
            return("Software reset");
        } else {
            return("Watchdog reset");
        }
    }
    if(reg & (1 << JTRF)) {
        return("JTAG reset");
    }
    {
      static char cause[sizeof("0xff")];
      snprintf(cause, sizeof(cause), "0x%02" PRIx8, reg);
      return cause;
    }
}

int boot_report(uint8_t *buf, size_t len, uint8_t *finished, __attribute__((unused)) char **topicstr) {
     char *s = (char *) buf;
     size_t l = len;
     int nread = 0;
     
     *finished = 0;
     
     RECORD_START(s + nread, l - nread);
     PUTFMT(",{\"n\": \"boot;application\",\"vs\":\"" APPLICATION "\"}");
     PUTFMT(",{\"n\": \"boot;build\",\"vs\":\"RIOT %s\"}", RIOT_VERSION);
     PUTFMT(",{\"n\": \"boot;reset_cause\",\"vs\":\"%s\"}", reset_cause(GPIOR0));
     RECORD_END(nread);

     *finished = 1;

     return nread;
}
