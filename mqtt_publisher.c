/*
 * Copyright (C) 2015 Freie Universität Berlin
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

/**
 * @ingroup     examples
 * @{
 *
 * @file
 * @brief       Example application for demonstrating RIOT's MQTT-SN library
 *              emCute
 *
 * @author      Hauke Petersen <hauke.petersen@fu-berlin.de>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "shell.h"
#include "msg.h"
#include "net/emcute.h"
#include "net/ipv6/addr.h"

#include "at24mac.h"

#include "mqtt_publisher.h"
#include "records.h"

#define MQPUB_PRIO         (THREAD_PRIORITY_MAIN - 1)

#define NUMOFSUBS           (16U)
#define TOPIC_MAXLEN        (64U)

#define MQPUB_TOPIC_LENGTH  64

/* State machine interval in secs */
#define MQPUB_STATE_INTERVAL 2

typedef enum {
    MQTTSN_NOT_CONNECTED,
    MQTTSN_CONNECTED,
    MQTTSN_PUBLISHING,
} mqttsn_state_t;

typedef struct mqttsn_stats {
  uint16_t connect_ok;
  uint16_t register_ok;
  uint16_t publish_ok;
  uint16_t connect_fail;
  uint16_t register_fail;
  uint16_t publish_fail;
} mqttsn_stats_t;

mqttsn_stats_t mqttsn_stats;

static char stack[THREAD_STACKSIZE_DEFAULT];

static char topicstr[MQPUB_TOPIC_LENGTH];
emcute_topic_t emcute_topic;

 int get_nodeid(char *buf, size_t size) {
    int n = 0;
    eui64_t e64;

    if (at24mac_get_eui64(0, &e64) != 0) {
        printf("Can't get nodeid\n");
        return 0;
    }
    for (unsigned i = 0; i < sizeof(e64.uint8); i++) {
      n += snprintf(buf + n, size - n, "%02x", e64.uint8[i]);
    }
    return n;
}

static void _init_topic(void) {
  
    //static char nodeid[sizeof(e64.uint8)*2+1];
    int n; 

    n = snprintf(topicstr, sizeof(topicstr), "%s/", MQTT_TOPIC_BASE);
    n += get_nodeid(topicstr + n, sizeof(topicstr) - n);
    n += snprintf(topicstr + n, sizeof(topicstr) - n, "/sensor");
}


uint8_t publish_buffer[MQTTSN_BUFFER_SIZE];

static int mqpub_pub(void) {
    unsigned flags = EMCUTE_QOS_0;
    size_t publen;
    int errno;
    
    publen = makereport(publish_buffer, sizeof(publish_buffer));
    printf("Publish %d: \"%s\"\n", publen, publish_buffer);
    if ((errno = emcute_pub(&emcute_topic, publish_buffer, publen, flags)) != EMCUTE_OK) {
        printf("error: unable to publish data to topic '%s [%i]' (error %d)\n",
               emcute_topic.name, (int)emcute_topic.id, errno);
        mqttsn_stats.publish_fail += 1;
        if (errno == EMCUTE_OVERFLOW)
             return 0;
    }
    mqttsn_stats.publish_ok += 1;
    return 0;
    
}
static int mqpub_con(void) {
    sock_udp_ep_t gw = { .family = AF_INET6, .port = MQTTSN_GATEWAY_PORT };
    int errno;
    printf("Conneect\n");

    /* parse address */
    if (ipv6_addr_from_str((ipv6_addr_t *)&gw.addr.ipv6, MQTTSN_GATEWAY_HOST) == NULL) {
        printf("error parsing IPv6 address\n");
        return 1;
    }
    if ((errno = emcute_con(&gw, true, NULL, NULL, 0, 0)) != EMCUTE_OK) {
         printf("error: unable to connect to gateway [%s]:%d (error %d)\n", MQTTSN_GATEWAY_HOST, MQTTSN_GATEWAY_PORT, errno);
        mqttsn_stats.connect_fail += 1;
        return 1;
    }
    printf("Successfully connected to gateway [%s]:%d\n", MQTTSN_GATEWAY_HOST, MQTTSN_GATEWAY_PORT);
    mqttsn_stats.connect_ok += 1;
    return 0;
}

static int mqpub_reg(void) {
     int errno;
    emcute_topic.name = topicstr;
    if ((errno = emcute_reg(&emcute_topic)) != EMCUTE_OK) {
        mqttsn_stats.register_fail += 1;
        printf("error: unable to obtain topic ID for \"%s\" (error %d)\n", topicstr, errno);
        return 1;
    }
    printf("Obtained topic ID %d for \"%s\"\n", (int)emcute_topic.id, topicstr);
    mqttsn_stats.register_ok += 1;
    return 0;
}

static mqttsn_state_t state;

static void *mqpub_thread(void *arg)
{
    uint32_t sleepsecs;
    (void)arg;
    
    printf("Here is mqtt_publisher thread\n");
 again:
    state = MQTTSN_NOT_CONNECTED;
    sleepsecs = MQPUB_STATE_INTERVAL;
    while (1) {
      switch (state) {
      case MQTTSN_NOT_CONNECTED:
        if (mqpub_con() == 0)
          state = MQTTSN_CONNECTED;
        break;
      case MQTTSN_CONNECTED:
        if (mqpub_reg() == 0) {
          state = MQTTSN_PUBLISHING;
        }
        break;
      case MQTTSN_PUBLISHING:
        if (mqpub_pub() != 0) {
          state = MQTTSN_NOT_CONNECTED;
          goto again;
        }
        sleepsecs = MQTTSN_PUBLISH_INTERVAL;
        break;
      default:
        printf("Do nothin in state %d\n", state);
        break;
      }
      xtimer_sleep(sleepsecs);
    }
    return NULL;    /* should never be reached */
}

void mqtt_publisher_init(void) {
    _init_topic();
    /* start publisher thread */
    thread_create(stack, sizeof(stack), MQPUB_PRIO, 0,
                  mqpub_thread, NULL, "emcute");
}

int mqttsn_report(uint8_t *buf, size_t len, ITERVAR(*iter)) {
     char *s = (char *) buf;
     size_t l = len;
     
     ITERSTART(*iter);

     ITERSTEP(*iter);
     STARTRECORD(s, l);
     PUTFMT(",{\"n\": \"mqtt_sn;gateway\",\"vs\":\"[%s]:%d\"}",MQTTSN_GATEWAY_HOST, MQTTSN_GATEWAY_PORT);
     ENDRECORD(s, l, len);
          
     STARTRECORD(s, l);
     PUTFMT(",{\"n\":\"mqtt_sn;stats;connect\",\"vj\":[");
     PUTFMT("{\"n\":\"ok\",\"u\":\"count\",\"v\":%d},", mqttsn_stats.connect_ok);
     PUTFMT("{\"n\":\"fail\",\"u\":\"count\",\"v\":%d}", mqttsn_stats.connect_fail);
     PUTFMT("]}");
     ENDRECORD(s, l, len);


     ITERSTEP(*iter);
     STARTRECORD(s, l);
     PUTFMT(",{\"n\":\"mqtt_sn;stats;register\",\"vj\":[");
     PUTFMT("{\"n\":\"ok\",\"u\":\"count\",\"v\":%d},", mqttsn_stats.register_ok);
     PUTFMT("{\"n\":\"fail\",\"u\":\"count\",\"v\":%d}", mqttsn_stats.register_fail);
     PUTFMT("]}");
     ENDRECORD(s, l, len);

     ITERSTEP(*iter);
     STARTRECORD(s, l);
     PUTFMT(",{\"n\":\"mqtt_sn;stats;publish\",\"vj\":[");
     PUTFMT("{\"n\":\"ok\",\"u\":\"count\",\"v\":%d},", mqttsn_stats.publish_ok);
     PUTFMT("{\"n\":\"fail\",\"u\":\"count\",\"v\":%d}", mqttsn_stats.publish_fail);
     PUTFMT("]}");
     ENDRECORD(s, l, len);

     ITERSTOP(*iter);

     return len-l;
}

