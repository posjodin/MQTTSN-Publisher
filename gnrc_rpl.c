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
#include "records.h"


#ifdef MODULE_NETSTATS_RPL
static int stats(char *str, size_t len, ITERVAR(*iter)) {
     char *s = str;
     size_t l = len;
     
     ITERSTART(*iter);

     ITERSTEP(*iter);
     STARTRECORD(s, l);
     PUTFMT(",{\"n\":\"rpl;stats;dio\",\"vj\":[");
     PUTFMT("{\"n\":\"u_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dio_rx_ucast_count);
     PUTFMT(",{\"n\":\"u_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dio_tx_ucast_count);
     PUTFMT(",{\"n\":\"m_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dio_rx_mcast_count);
     PUTFMT(",{\"n\":\"m_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dio_tx_mcast_count);
     PUTFMT("]}");
     ENDRECORD(s, l, len);

     ITERSTEP(*iter);
     STARTRECORD(s, l);
     PUTFMT(",{\"n\":\"rpl;stats;dis\",\"vj\":[");
     PUTFMT("{\"n\":\"u_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dis_rx_ucast_count);
     PUTFMT(",{\"n\":\"u_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dis_tx_ucast_count);
     PUTFMT(",{\"n\":\"m_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dis_rx_mcast_count);
     PUTFMT(",{\"n\":\"m_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dis_tx_mcast_count);
     PUTFMT("]}");
     ENDRECORD(s, l, len);     

     ITERSTEP(*iter);
     STARTRECORD(s, l);
     PUTFMT(",{\"n\":\"rpl;stats;dao\",\"vj\":[");
     PUTFMT("{\"n\":\"u_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_rx_ucast_count);
     PUTFMT(",{\"n\":\"u_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_tx_ucast_count);
     PUTFMT(",{\"n\":\"m_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_rx_mcast_count);
     PUTFMT(",{\"n\":\"m_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_tx_mcast_count);
     PUTFMT("]}");
     ENDRECORD(s, l, len);
          
     ITERSTEP(*iter);
     STARTRECORD(s, l);
     PUTFMT(",{\"n\":\"rpl;stats;dao_ack\",\"vj\":[");
     PUTFMT("{\"n\":\"u_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_ack_rx_ucast_count);
     PUTFMT(",{\"n\":\"u_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_ack_tx_ucast_count);
     PUTFMT(",{\"n\":\"m_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_ack_rx_mcast_count);
     PUTFMT(",{\"n\":\"m_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_ack_tx_mcast_count);
     PUTFMT("]}");
     ENDRECORD(s, l, len);     
     ITERSTOP(*iter);
     return len-l;
}
#endif /* MODULE_NETSTATS_RPL */

static int instances(char *str, size_t len) {
     char *s = str;
     size_t l = len;

     STARTRECORD(s, l);
     PUTFMT(",{\"n\":\"rpl;inst\",\"vj\":[");
     for (uint8_t i = 0; i < GNRC_RPL_INSTANCES_NUMOF; ++i) {
          uint8_t first = 1;
          if (gnrc_rpl_instances[i].state != 0) {
               if (first)
                    first = 0;
               else
                    PUTFMT("xx,xx");
               PUTFMT("{\"id\":%d,\"mop\":%d,\"ocp\":%d}",
                      gnrc_rpl_instances[i].id, gnrc_rpl_instances[i].mop, gnrc_rpl_instances[i].of->ocp);
          }
     }
     PUTFMT("]}");
     ENDRECORD(s, l, len);     

     return len-l;
}

static int dags(char *str, size_t len) {
     char *s = str;
     size_t l = len;

     STARTRECORD(s, l);
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
     ENDRECORD(s, l, len);     
     return len-l;
}

static int parents(char *str, size_t len) {
     char *s = str;
     size_t l = len;

     STARTRECORD(s, l);
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
     ENDRECORD(s, l, len);     
     return len-l;

}

int rpl_report(uint8_t *buf, size_t len, ITERVAR(*iter)) {
     char *s = (char *) buf;
     size_t l = len;
     int n;
     
     ITERSTART(*iter);

     ITERSTEP(*iter);
     n = instances(s, l);
     if (n == 0)
       return (len - l);
     else {
       s += n; l -= n;
     }

     ITERSTEP(*iter);
     n = dags(s, l);
     if (n == 0)
       return (len - l);
     else {
       s += n; l -= n;
     }

     ITERSTEP(*iter);
     n = parents(s, l);
     if (n == 0)
       return (len - l);
     else {
       s += n; l -= n;
     }


#ifdef MODULE_NETSTATS_RPL
     ITERSTEP(*iter);
     static ITERVAR(statsiter) = NULL;
     do {
       n = stats(s, l, &statsiter);
       if (n == 0)
         return (len - l);
       else {
         s += n; l -= n;
       }
     } while (ITERMORE(statsiter));
#endif

     ITERSTOP(*iter);
     return n;
}
