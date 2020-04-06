/*
 * Copyright (C) 2018       HAW Hamburg
 * Copyright (C) 2015–2017  Cenk Gündoğan <mail-github@cgundogan.de>
 *
 * This file is subject to the terms and conditions of the GNU Lesser General
 * Public License v2.1. See the file LICENSE in the top level directory for
 * more details.
 */

/**
 * @ingroup     sys_shell_commands
 * @{
 *
 * @file
 *
 * @author      Cenk Gündoğan <cenk.guendogan@haw-hamburg.de>
 */

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


#undef MODULE_NETSTATS_RPL
#ifdef MODULE_NETSTATS_RPL
static int stats(char *str, size_t len) {
     int n = 0;
     
     n += snprintf(str + n, len - n, ",{\"n\":\"rpl;stats;dio\",\"vj\":[");
     n += snprintf(str + n, len - n, "{\"n\":\"u_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dio_rx_ucast_count);
     n += snprintf(str + n, len - n, ",{\"n\":\"u_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dio_tx_ucast_count);
     n += snprintf(str + n, len - n, ",{\"n\":\"m_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dio_rx_mcast_count);
     n += snprintf(str + n, len - n, ",{\"n\":\"m_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dio_tx_mcast_count);
     n += snprintf(str + n, len - n, "]");
     
     n += snprintf(str + n, len - n, ",{\"n\":\"rpl;stats;dis\",\"vj\":[");
     n += snprintf(str + n, len - n, "{\"n\":\"u_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dis_rx_ucast_count);
     n += snprintf(str + n, len - n, ",{\"n\":\"u_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dis_tx_ucast_count);
     n += snprintf(str + n, len - n, ",{\"n\":\"m_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dis_rx_mcast_count);
     n += snprintf(str + n, len - n, ",{\"n\":\"m_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dis_tx_mcast_count);
     n += snprintf(str + n, len - n, "]");
     
     n += snprintf(str + n, len - n, ",{\"n\":\"rpl;stats;dao\",\"vj\":[");
     n += snprintf(str + n, len - n, "{\"n\":\"u_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_rx_ucast_count);
     n += snprintf(str + n, len - n, ",{\"n\":\"u_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_tx_ucast_count);
     n += snprintf(str + n, len - n, ",{\"n\":\"m_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_rx_mcast_count);
     n += snprintf(str + n, len - n, ",{\"n\":\"m_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_tx_mcast_count);
     n += snprintf(str + n, len - n, "]");
     
     n += snprintf(str + n, len - n, ",{\"n\":\"rpl;stats;dao_ack\",\"vj\":[");
     n += snprintf(str + n, len - n, "{\"n\":\"u_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_ack_rx_ucast_count);
     n += snprintf(str + n, len - n, ",{\"n\":\"u_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_ack_tx_ucast_count);
     n += snprintf(str + n, len - n, ",{\"n\":\"m_rx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_ack_rx_mcast_count);
     n += snprintf(str + n, len - n, ",{\"n\":\"m_tx\",\"u\":\"count\",\"v\":%d}", gnrc_rpl_netstats.dao_ack_tx_mcast_count);
     n += snprintf(str + n, len - n, "]");
     return n;
}
#endif

static int instances(char *str, size_t len) {
     int n = 0;
     n += snprintf(str + n, len - n, ",{\"n\":\"rpl;inst\",\"vj\":[");

     for (uint8_t i = 0; i < GNRC_RPL_INSTANCES_NUMOF; ++i) {
          uint8_t first = 1;
          if (gnrc_rpl_instances[i].state != 0) {
               if (first)
                    first = 0;
               else
                    n+= snprintf(str + n, len - n, ",");
               n += snprintf(str + n, len - n, "{\"id\":%d,\"mop\":%d,\"ocp\":%d}",
                             gnrc_rpl_instances[i].id, gnrc_rpl_instances[i].mop, gnrc_rpl_instances[i].of->ocp);
          }
     }
     n += snprintf(str + n, len - n, "]}");
     return n;
}

static int dags(char *str, size_t len) {
     int n = 0;
     n += snprintf(str + n, len - n, ",{\"n\":\"rpl;dag\",\"vj\":[");

     for (uint8_t i = 0; i < GNRC_RPL_INSTANCES_NUMOF; ++i) {
          uint8_t first = 1;
          if (gnrc_rpl_instances[i].state != 0) {
               if (first)
                    first = 0;
               else
                    n+= snprintf(str + n, len - n, ",");

               gnrc_rpl_dodag_t *dodag = NULL;
               dodag = &gnrc_rpl_instances[i].dodag;


               n += snprintf(str + n, len - n,
                             "{\"root\":\":%02x%02x\", \"inst\":%d, \"rank\":%d, \"ver\":%d, \"pref\":%d, \"status\":\"%s%s\"",
               //ipv6_addr_to_str(addr_str, &dodag->dodag_id, sizeof(addr_str)),
                             dodag->dodag_id.u8[14], dodag->dodag_id.u8[15], 
                             dodag->instance->id,
                             dodag->my_rank, dodag->version, dodag->prf,
                             dodag->grounded ? "g": "",
                             (dodag->node_status == GNRC_RPL_LEAF_NODE ? "l" : "r"));
               n += snprintf(str + n, len - n, "}");
          }
     }
     n += snprintf(str + n, len - n, "]}");
     return n;
}

static int parents(char *str, size_t len) {
     int n = 0;
     n += snprintf(str + n, len - n, ",{\"n\":\"rpl;parents\",\"vj\":[");

     for (uint8_t i = 0; i < GNRC_RPL_INSTANCES_NUMOF; ++i) {
          uint8_t first = 1;
          if (gnrc_rpl_instances[i].state != 0) {
               if (first)
                    first = 0;
               else
                    n+= snprintf(str + n, len - n, ",");

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
                              n += snprintf(str + n, len - n, ",");
                         n += snprintf(str + n, len - n, "{\"dag\":\"%02x%02x\"",
                                       dodag->dodag_id.u8[14], dodag->dodag_id.u8[15]);
                         n += snprintf(str + n, len - n, ",\"parent\":\"%s\",\"rank\":%d}",
                                       ipv6_addr_to_str(addr_str, &parent->addr, sizeof(addr_str)),
                                       parent->rank);
                    }
               }
          }
     }
     n += snprintf(str + n, len - n, "]}");
     return n;
}

int rpl_report(uint8_t *buf, size_t len) {
     char *str = (char *) buf;
     int n = 0;

     n += instances(str + n, len - n);
     n += dags(str + n, len - n);
     n += parents(str + n, len - n);
#ifdef MODULE_NETSTATS_RPL
     n += stats(str + n, len - n);
#endif
     return n;
}
/**
 * @}
 */
