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

#include "shell.h"
#include "net/emcute.h"
#include "net/ipv6/addr.h"

#include "mqttsn_publisher.h"

#ifndef EMCUTE_ID
#define EMCUTE_ID           ("avr-rss2")
#endif
#define EMCUTE_PORT         (1883U)
#define EMCUTE_PRIO         (THREAD_PRIORITY_MAIN - 1)

static int cmd_hello(int argc, char **argv)
{
    (void)argc;
    (void)argv;

    printf("hello, world\n");
    return 0;
}

static const shell_command_t shell_commands[] = {
    { "hello", "say hello", cmd_hello },
    { NULL, NULL, NULL }
};

int main(void)
{
    ///* the main thread needs a msg queue to be able to run `ping6`*/
    //msg_init_queue(queue, ARRAY_SIZE(queue));

    mqttsn_publisher_init();
    /* start shell */
    char line_buf[SHELL_DEFAULT_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_DEFAULT_BUFSIZE);

    /* should never be reached */
    return 0;
}
