#ifndef MQTTSN_PUBLISHER_T
#define MQTTSN_PUBLISHER_T

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
  uint16_t reset;
} mqttsn_stats_t;

extern mqttsn_stats_t mqttsn_stats;

void mqttsn_publisher_init(void);
mqttsn_state_t mqttsn_publisher_state(void);


#ifndef MQTT_TOPIC_BASE
#define MQTT_TOPIC_BASE "KTH/avr-rss2"
#endif

#ifndef MQTTSN_GATEWAY_HOST
//#define MQTTSN_GATEWAY_HOST "64:ff9b::c010:7dea" /* broker.ssvl.kth.se IPv6-mapped IPv4 address */
//#define MQTTSN_GATEWAY_HOST "64:ff9b::c0a8:0196" /* lxc-ha IPv6-mapped IPv4 address */
#define MQTTSN_GATEWAY_HOST "fd95:9bba:768f:0:216:3eff:fec6:99db" /* lxc-ha IPv6 static ULA */
//#define MQTTSN_GATEWAY_HOST "::ffff:82ed:ca25" /* minime.sjodin.net */
//#define MQTTSN_GATEWAY_HOST "::ffff:c010:7de8" /* lab-pc.ssvl.kth.se */
//#define MQTTSN_GATEWAY_HOST "lab-pc.ssvl.kth.se" /* lab-pc.ssvl.kth.se */
//#define MQTTSN_GATEWAY_HOST "::ffff:5eff:b333" /* home.sjodin.net */
#endif /* MQTTSN_GATEWAY_HOST */

#ifndef MQTTSN_GATEWAY_PORT
//#define MQTTSN_GATEWAY_PORT 1884
#define MQTTSN_GATEWAY_PORT 10000
#endif /* MQTTSN_GATEWAY_PORT */

#ifndef MQTTSN_BUFFER_SIZE
#define MQTTSN_BUFFER_SIZE (EMCUTE_BUFSIZE-16)
#endif  /* MQTTSN_BUFFER_SIZE */

#ifndef MQTTSN_PUBLISH_INTERVAL
#define MQTTSN_PUBLISH_INTERVAL 30
#endif /* MQTTSN_PUBLISH_INTERVAL */

void mqttsn_publisher_init(void);

int get_nodeid(char *buf, size_t size);

size_t makereport(uint8_t *buffer, size_t len);

int mqttsn_stats_cmd(int argc, char **argv);

#endif /* MQTTSN_PUBLISHER_T */
