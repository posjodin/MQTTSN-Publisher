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

int boot_report(uint8_t *buf, size_t len, uint8_t *finished) {
     char *s = (char *) buf;
     size_t l = len;
     int nread = 0;
     
     *finished = 0;
     
     RECORD_START(s + nread, l - nread);
     PUTFMT(",{\"n\": \"boot;application\",\"vs\":\"" APPLICATION "\"}");
     PUTFMT(",{\"n\": \"boot;build\",\"vs\":\"RIOT %s\"}", RIOT_VERSION);
     RECORD_END(nread);

     *finished = 1;

     return nread;
}
