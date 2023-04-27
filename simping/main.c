
/*
 * Copyright (C) 2020 Robert Olsson
 *
 */

/**
 * @ingroup     application
 * @{
 *
 * @file
 * @brief       WSSN application
 *
 * @author      Robert Olsson <roolss@kth.se>
 *
 * @}
 */

#include <stdio.h>
#include <string.h>
#include "thread.h"
#include "msg.h"
#include "xtimer.h"
#include "timex.h"
#include "shell.h"

#ifdef MODULE_NETIF
#include "net/gnrc.h"
#endif
#ifndef SHELL_BUFSIZE
#define SHELL_BUFSIZE       (128U)
#endif

#include "net/af.h"
#include "net/ipv4/addr.h"
#include "net/ipv6/addr.h"
#include "net/sock/udp.h"

#include "net/sim7020.h"

kernel_pid_t me;
 
extern kernel_pid_t rime_pid;

static int cmd_upgr(__attribute__((unused)) int ac, __attribute__((unused)) char **av)
{
    pm_reboot();

    return 0;
}

#ifdef MODULE_SIM7020
int sim7020cmd_init(int argc, char **argv);
int sim7020cmd_status(int argc, char **argv);
int sim7020cmd_stats(int argc, char **argv);
int sim7020cmd_at(int argc, char **argv);
int sim7020cmd_stop(int argc, char **argv);
int sim7020cmd_reset(int argc, char **argv);
int cmd_sim7020_conf(int argc, char **argv);
#endif /* MODULE_SIM7020 */
#ifdef UPING
int cmd_uping(int argc, char **argv);
#endif /* UPING */

static const shell_command_t shell_commands[] = {
#ifdef MODULE_SIM7020
    { "init", "Init SIM7020", sim7020cmd_init },
    { "stats", "SIM7020 statistics", sim7020cmd_stats },
    { "status", "Report SIM7020 status", sim7020cmd_status },
    { "at", "Send AT string to SIM7020", sim7020cmd_at },
    { "reset", "Reset SIM7020", sim7020cmd_reset },
    { "stop", "Stop SIM7020", sim7020cmd_stop },
    { "sim", "Configure SIM7020", cmd_sim7020_conf},
#endif /* MODULE_SIM7020 */
#ifdef UPING
    {"uping", "UDP ping", cmd_uping},
#endif /* UPING */
    { "upgr", "reboot (for flash)", cmd_upgr },
    { NULL, NULL, NULL }
};

int main(void)
{
    void atmega_reset_cause(void);
    printf("Reset cause: \n");
    atmega_reset_cause();

    me = thread_getpid();
    printf("main pid=%u\n", me);

    sim7020_init();
    char line_buf[SHELL_BUFSIZE];
    shell_run(shell_commands, line_buf, SHELL_BUFSIZE);
}
