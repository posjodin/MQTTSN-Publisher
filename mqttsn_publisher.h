#ifndef MQTTSN_PUBLISHER_T
#define MQTTSN_PUBLISHER_T

#include "net/emcute.h"

typedef enum {
    MQTTSN_NOT_CONNECTED,
    MQTTSN_CONNECTED,
    MQTTSN_PUBLISHING,
    MQTTSN_DISCONNECTED,
} mqttsn_state_t;

typedef struct mqttsn_stats {
  uint16_t connect_ok;
  uint16_t register_ok;
  uint16_t publish_ok;
  uint16_t connect_fail;
  uint16_t register_fail;
  uint16_t publish_fail;
  uint16_t reset;
} mqttsn_stats_t;

extern mqttsn_stats_t mqttsn_stats;

typedef emcute_topic_t mqpub_topic_t;

void mqttsn_publisher_init(void);
mqttsn_state_t mqttsn_publisher_state(void);


#ifndef MQTT_TOPIC_BASE
#define MQTT_TOPIC_BASE "KTH/avr-rss2"
#endif

#ifndef MQTTSN_GATEWAY_HOST
#define MQTTSN_GATEWAY_HOST "lab-pc.ssvl.kth.se"
#endif /* MQTTSN_GATEWAY_HOST */
#ifndef MQTTSN_GATEWAY_PORT
//#define MQTTSN_GATEWAY_PORT 1884
#define MQTTSN_GATEWAY_PORT 10000
#endif /* MQTTSN_GATEWAY_PORT */

#ifndef MQTTSN_BUFFER_SIZE
#define MQTTSN_BUFFER_SIZE (EMCUTE_BUFSIZE-16)
#endif  /* MQTTSN_BUFFER_SIZE */

#ifndef MQTTSN_PUBLISH_INTERVAL
#define MQTTSN_PUBLISH_INTERVAL 120
#endif /* MQTTSN_PUBLISH_INTERVAL */

void mqttsn_publisher_init(void);

int get_nodeid(char *buf, size_t size);

size_t makereport(uint8_t *buffer, size_t len, uint8_t *finished);

int mqttsn_stats_cmd(int argc, char **argv);

int mqpub_pub(mqpub_topic_t *topic, void *data, size_t len);
int mqpub_con(char *host, uint16_t port);
int mqpub_reg(mqpub_topic_t *topic, char *topicstr);
int mqpub_discon(void);
int mqpub_reset(void);
size_t mqpub_init_topic(char *topic, size_t topiclen, char *suffix);

int mqpub_pubtopic(char *topicstr, uint8_t *data, size_t datalen);

void mqpub_report_ready(void);

int dns_resolve_inetaddr(char *host, ipv6_addr_t *result);

#endif /* MQTTSN_PUBLISHER_T */

