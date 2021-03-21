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

#include "msg.h"
#include "net/ipv6/addr.h"
#include "xtimer.h"

#include "mqttsn_publisher.h"
#include "report.h"

#ifdef EPCGW
#include "../epcgw.h"
#endif /* EPCGW */
#define ENABLE_DEBUG    (0)
#include "debug.h"

#ifdef BOARD_AVR_RSS2
#include "pstr_print.h"
#endif

static int seq_nr_value = 0;

#if defined(MODULE_GNRC_RPL)
int rpl_report(uint8_t *buf, size_t len, uint8_t *finished);
#endif
#if defined(MODULE_SIM7020)
int sim7020_report(uint8_t *buf, size_t len, uint8_t *finished);
#endif
int mqttsn_report(uint8_t *buf, size_t len, uint8_t *finished);
int boot_report(uint8_t *buf, size_t len, uint8_t *finished);

static size_t preamble(uint8_t *buf, size_t len) {
     char *s = (char *) buf;
     size_t l = len;
     size_t n;
     int nread = 0;
     
     RECORD_START(s + nread, l - nread);
     PUTFMT("{\"bn\":\"urn:dev:mac:");
     n = get_nodeid(RECORD_STR(), RECORD_LEN());
     RECORD_ADD(n);
     PUTFMT(";\"");
     PUTFMT(",\"bu\":\"count\",\"bt\":%lu}", (uint32_t) (xtimer_now_usec()/1000000));
     PUTFMT(",{\"n\":\"seq_no\",\"u\":\"count\",\"v\":%d}", 9000+seq_nr_value++);
     RECORD_END(nread);

     return (nread);
}

/*
 * Report scheduler -- return report generator function to use next
 */

typedef enum {
#if defined(MODULE_GNRC_RPL)
  s_rpl_report,
#endif
#if defined(MODULE_SIM7020)
  s_sim7020_report,
#endif
  s_mqttsn_report,
  s_max_report
} report_state_t;

report_gen_t next_report_gen(void) {
     static unsigned int reportno = 0;

     if (reportno++ == 0)
       return(boot_report);

#ifdef EPCGW
     /* EPC reports have priority. If there is an epc report
      * waiting, send it now
      */
     report_gen_t epcgen = epcgw_report_gen();
     if (epcgen != NULL)
         return epcgen;
#endif /* EPCGW */

     switch (reportno % s_max_report) {
#if defined(MODULE_GNRC_RPL)
     case s_rpl_report:
          return(rpl_report);
#endif
#if defined(MODULE_SIM7020)
     case s_sim7020_report:
          return(sim7020_report);
#endif
     case s_mqttsn_report:
          return(mqttsn_report);
     default:
         printf("Bad report no %d\n", reportno);
     }
     return NULL;
}

static char *reportfunstr(report_gen_t fun) {
  if (fun == NULL)
    return "NUL";
  else if (fun == boot_report)
    return "boot";
#if defined(MODULE_GNRC_RPL)
  else if (fun == rpl_report)
    return("rpl");
#endif
#if defined(MODULE_SIM7020)
  else if (fun == sim7020_report)
    return("sim7020");
#endif
  else if (fun == mqttsn_report)
    return("mqttsn");
  else
    return("???");
}

/*
 * Reports -- build report by writing records to buffer 
 * Call report function generator to schedule next report function.
 * The report function fills the report with sensor data
 */
static size_t reports(uint8_t *buf, size_t len, uint8_t *finished) {
     char *s = (char *) buf;
     size_t l = len;
     size_t nread = 0;
     static report_gen_t reportfun = NULL;
     //static uint8_t finished;

     if (reportfun == NULL) {
          reportfun = next_report_gen();
     }
     do {
          int n = reportfun((uint8_t *) s + nread, l - nread, finished);
          DEBUG("reportfun '%s', n %d (tot %d) finished %d\n", reportfunstr(reportfun), n, nread, (int) finished);
          if (n == 0)
               return (nread);
          else
               nread += n;
     } while (!finished);
     reportfun = NULL;
     return (nread);
}

/*
 * make a sensor report. Write senml preamble to buffer, then call report function to fill
 * with data, and then write senml finish
 */
size_t makereport(uint8_t *buffer, size_t len, uint8_t *finished) {
     char *s = (char *) buffer;
     size_t l = len;
     size_t n;
     int nread = 0;
     
     RECORD_START(s + nread, l - nread);
     PUTFMT("[");
     n = preamble((uint8_t *) RECORD_STR(), RECORD_LEN()-1); /* Save one for last bracket */
     RECORD_ADD(n);
     n = reports((uint8_t *) RECORD_STR(), RECORD_LEN()-1, finished); /* Save one for last bracket */
     RECORD_ADD(n);
     PUTFMT("]");
     RECORD_END(nread);

     return (nread);
}

