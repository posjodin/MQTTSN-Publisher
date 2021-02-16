/*
 * Copyright (C) 2020       Peter Sjödin <psj@kth.se>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for
 * more details.
 */

/* Generate RPL reports */

#include <string.h>
#include <stdio.h>
#include "net/gnrc/netif.h"
#include "net/gnrc/rpl.h"
#include "net/gnrc/rpl/structs.h"
#include "net/gnrc/rpl/dodag.h"
#include "utlist.h"
#include "trickle.h"
#ifdef MODULE_GNRC_RPL_P2P
#include "net/gnrc/rpl/p2p.h"
#include "net/gnrc/rpl/p2p_dodag.h"
#include "net/gnrc/rpl/p2p_structs.h"
#endif

#include "report.h"

#ifdef BOARD_AVR_RSS2
#include "pstr_print.h"
#endif

#ifdef MODULE_NETSTATS_RPL
typedef enum {s_dio, s_dis, s_dao, s_dao_ack} s_stats_t;

static int stats(char *str, size_t len, uint8_t *done) {
     char *s = str;
     size_t l = len;
     static s_stats_t state = s_dio;
     *done = 0;
     int nread = 0;
     
     switch (state) {
     case s_dio:
       RECORD_START(s + nread, l - nread);
       PUTFMT(",{\"n\":\"rpl;stats;dio\",\"vj\":[");
       PUTFMT("{\"n\":\"u_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dio_rx_ucast_count);
       PUTFMT(",{\"n\":\"u_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dio_tx_ucast_count);
       PUTFMT(",{\"n\":\"m_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dio_rx_mcast_count);
       PUTFMT(",{\"n\":\"m_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dio_tx_mcast_count);
       PUTFMT("]}");
       RECORD_END(nread);
       state = s_dis;

     case s_dis:
       RECORD_START(s + nread, l - nread);
       PUTFMT(",{\"n\":\"rpl;stats;dis\",\"vj\":[");
       PUTFMT("{\"n\":\"u_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dis_rx_ucast_count);
       PUTFMT(",{\"n\":\"u_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dis_tx_ucast_count);
       PUTFMT(",{\"n\":\"m_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dis_rx_mcast_count);
       PUTFMT(",{\"n\":\"m_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dis_tx_mcast_count);
       PUTFMT("]}");
       RECORD_END(nread);     
       state = s_dao;

     case s_dao:
       RECORD_START(s + nread, l - nread);
       PUTFMT(",{\"n\":\"rpl;stats;dao\",\"vj\":[");
       PUTFMT("{\"n\":\"u_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_rx_ucast_count);
       PUTFMT(",{\"n\":\"u_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_tx_ucast_count);
       PUTFMT(",{\"n\":\"m_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_rx_mcast_count);
       PUTFMT(",{\"n\":\"m_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_tx_mcast_count);
       PUTFMT("]}");
       RECORD_END(nread);
       state = s_dao_ack;

     case s_dao_ack:
       RECORD_START(s + nread, l - nread);
       PUTFMT(",{\"n\":\"rpl;stats;dao_ack\",\"vj\":[");
       PUTFMT("{\"n\":\"u_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_ack_rx_ucast_count);
       PUTFMT(",{\"n\":\"u_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_ack_tx_ucast_count);
       PUTFMT(",{\"n\":\"m_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_ack_rx_mcast_count);
       PUTFMT(",{\"n\":\"m_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_ack_tx_mcast_count);
       PUTFMT("]}");
       RECORD_END(nread);     
       state = s_dio;
       
       *done = 1;
     }
     return nread;
}

#endif /* MODULE_NETSTATS_RPL */

static int instances(char *str, size_t len) {
     char *s = str;
     size_t l = len;
     size_t nread = 0;
     
     RECORD_START(s + nread, l - nread);
     PUTFMT(",{\"n\":\"rpl;inst\",\"vj\":[");
     for (uint8_t i = 0; i < GNRC_RPL_INSTANCES_NUMOF; ++i) {
          uint8_t first = 1;
          if (gnrc_rpl_instances[i].state != 0) {
               if (first)
                    first = 0;
               else
                    PUTFMT(",");
               PUTFMT("{\"id\":%d,\"mop\":%d,\"ocp\":%d}",
                       gnrc_rpl_instances[i].id, gnrc_rpl_instances[i].mop, gnrc_rpl_instances[i].of->ocp);
          }
     }
     PUTFMT("]}");
     RECORD_END(nread);     

     return nread;
}

static int dags(char *str, size_t len) {
     char *s = str;
     size_t l = len;
     size_t nread = 0;
     
     RECORD_START(s + nread, l - nread);
     PUTFMT(",{\"n\":\"rpl;dag\",\"vj\":[");

     for (uint8_t i = 0; i < GNRC_RPL_INSTANCES_NUMOF; ++i) {
          uint8_t first = 1;
          if (gnrc_rpl_instances[i].state != 0) {
               if (first)
                    first = 0;
               else
                    PUTFMT(",");
               
               gnrc_rpl_dodag_t *dodag = NULL;
               dodag = &gnrc_rpl_instances[i].dodag;

               PUTFMT("{\"root\":\":%02x%02x\", \"inst\":%d, \"rank\":%d, \"ver\":%d, \"pref\":%d, \"status\":\"%s%s\"}",
               //ipv6_addr_to_str(addr_str, &dodag->dodag_id, sizeof(addr_str)),
                      dodag->dodag_id.u8[14], dodag->dodag_id.u8[15], 
                      dodag->instance->id,
                      dodag->my_rank, dodag->version, dodag->prf,
                      dodag->grounded ? "g": "",
                      (dodag->node_status == GNRC_RPL_LEAF_NODE ? "l" : "r"));
          }
     }
     PUTFMT("]}");
     RECORD_END(nread);     
     return nread;
}

static int parents(char *str, size_t len) {
     char *s = str;
     size_t l = len;
     size_t nread = 0;
     
     RECORD_START(s + nread, l - nread);
     PUTFMT(",{\"n\":\"rpl;parents\",\"vj\":[");

     for (uint8_t i = 0; i < GNRC_RPL_INSTANCES_NUMOF; ++i) {
          uint8_t first = 1;
          if (gnrc_rpl_instances[i].state != 0) {
               if (first)
                    first = 0;
               else
                    PUTFMT(",");

               gnrc_rpl_dodag_t *dodag = NULL;
               dodag = &gnrc_rpl_instances[i].dodag;

               {
                    gnrc_rpl_parent_t *parent = NULL;
                    uint8_t first = 1;
                    char addr_str[IPV6_ADDR_MAX_STR_LEN];
                    LL_FOREACH(gnrc_rpl_instances[i].dodag.parents, parent) {
                         if (first)
                              first = 0;
                         else
                              PUTFMT(",");
                         PUTFMT("{\"dag\":\"%02x%02x\"", dodag->dodag_id.u8[14], dodag->dodag_id.u8[15]);
                         PUTFMT(",\"parent\":\"%s\",\"rank\":%d}",
                                ipv6_addr_to_str(addr_str, &parent->addr, sizeof(addr_str)),
                                parent->rank);
                    }
               }
          }
     }
     PUTFMT("]}");
     RECORD_END(nread);     
     return nread;
}

typedef enum {s_instances, s_dags, s_parents, s_stats} rpl_state_t;

int rpl_report(uint8_t *buf, size_t len, uint8_t *finished) {
     char *s = (char *) buf;
     size_t l = len;
     int nread = 0, n;
     static rpl_state_t state = s_instances;

     *finished = 0;

     switch (state) {
     case s_instances:
       n = instances(s + nread, l - nread);
       if (n == 0)
         return (nread);
       nread += n;
       state = s_dags;

     case s_dags:
       n = dags(s + nread, l - nread);
       if (n == 0)
         return (nread);
       nread += n;
       state = s_parents;

     case s_parents:
       n = parents(s + nread, l - nread);
       if (n == 0)
         return (nread);
       nread += n;
       state = s_stats;

     case s_stats:
#ifdef MODULE_NETSTATS_RPL
       {
            static uint8_t done;
     
            do {
                 n = stats(s + nread, l - nread, &done);
                 if (n == 0)
                      return (nread);
                 nread += n;
            } while (!done);
       }
#endif
       state = s_instances;

       *finished = 1;
     }
     return nread;
}
