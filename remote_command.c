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

#include "xtimer.h"
#include "timex.h"
#include "shell.h"

#ifdef MODULE_SIM7020
#include "net/af.h"
#include "net/ipv4/addr.h"
#include "net/ipv6/addr.h"
#include "net/sock/udp.h"

#include "net/sim7020.h"
#endif /* MODULE_SIM7020 */

#include "hashes.h"

#ifdef BOARD_AVR_RSS2
#include "pstr_print.h"
#endif

#include "mqttsn_publisher.h"

#ifndef COMMAND_BUFSIZE
#define COMMAND_BUFSIZE       (128U)
#endif

static const shell_command_t *_shell_commands;
static char topicstr[MQPUB_TOPIC_LENGTH];

static void sub_cb(const emcute_topic_t *topic, void *data, size_t len);

void init_remote_commands(const shell_command_t *shell_commands) {
    _shell_commands = shell_commands;
    char nodeidstr[20];
    (void) get_nodeid(nodeidstr, sizeof(nodeidstr));
    mqpub_init_topic(topicstr, sizeof(topicstr), nodeidstr, "/config");
    mqpub_start_subscription(topicstr, sub_cb);
}

void remote_command_str(const shell_command_t *shell_commands, char *str);

static void sub_cb(__attribute__ ((unused)) const emcute_topic_t *topic, void *data, size_t len) {
    char command[COMMAND_BUFSIZE];
    if (len >= sizeof(command))
        return;
    strncpy(command, data, len);
    command[len] = '\0';
    remote_command_str(_shell_commands, command);
}
