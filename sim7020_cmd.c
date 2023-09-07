/*
 * Copyright (C) 2023 Peter Sjödin, KTH
 *
 * This file is subject to the terms and conditions of the GNU Lesser
 * General Public License v2.1. See the file LICENSE in the top level
 * directory for more details.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <errno.h>

#include <avr/eeprom.h>

#include "net/af.h"
#include "net/ipv4/addr.h"
#include "net/ipv6/addr.h"
#include "net/sock/udp.h"

#include "async_at.h"
#include "xtimer.h"
#include "cond.h"
#include "periph/uart.h"
#include "shell.h"

#include "net/sim7020.h"

#ifdef BOARD_AVR_RSS2
#include "pstr_print.h"
#endif

#include "eedata.h"

sim7020_conf_t conf = {
    .flags = SIM7020_CONF_FLAGS_DEFAULT,
    .apn = "4g.tele2.se",
    .operator = "24007"
};

static EEMEM struct {
    eehash_t ee_hash;
    sim7020_conf_t ee_conf;
} ee_data;

    
#include <string.h>
#include "hashes.h"

static void printconf(void) {
    printf("apn: %s\n", (conf.flags & SIM7020_CONF_MANUAL_APN) ? conf.apn : "auto");
    printf("operator: %s\n", (conf.flags & SIM7020_CONF_MANUAL_OPERATOR) ? conf.operator : "auto");
}

static inline void _apply(void) {
    sim7020_setconf(&conf);
}

void sim7020_conf_init(void) {
    sim7020_conf_t confdata;
    int n = read_eeprom(&confdata, &ee_data, sizeof(confdata));
    if (n != 0) {
        memcpy(&conf, &confdata, sizeof(conf));
        printf("Apply SIM config: \n");
        printconf();
        _apply();
    }
    else {
      printf("No SIM config\n");
    }
}

static int cmd_apn(int argc, char **argv) {
    if (argc == 2) {
        int sim7020_apn(char *buf, int len);
        char apn[32];
        int res = sim7020_apn(apn, sizeof(apn));
        if (res != 0)
            printf("%s\n", apn);
        return res != 0;
    }
    else {
        if (strcmp(argv[2], "auto") == 0) {
            /* Toggle manual flag */
            conf.flags ^= SIM7020_CONF_MANUAL_APN;
        }
        else {
            strncpy(conf.apn, argv[2], sizeof(conf.apn));
            conf.flags |= SIM7020_CONF_MANUAL_APN;
        }
        update_eeprom(&conf, &ee_data, sizeof(conf));
        _apply();
        return 1;
    }
}

static int cmd_operator(int argc, char **argv) {
    if (argc == 2) {
        char operator[32];
        int res = sim7020_operator(operator, sizeof(operator));
        if (res != 0)
            printf("%s\n", operator);
        return res != 0;

    }
    else {
        if (strcmp(argv[2], "auto") == 0) {
            /* Toggle manual flag */
            conf.flags ^= SIM7020_CONF_MANUAL_OPERATOR;
        }
        else {
            strncpy(conf.operator, argv[2], sizeof(conf.operator));
            conf.flags |= SIM7020_CONF_MANUAL_OPERATOR;
        }
        update_eeprom(&conf, &ee_data, sizeof(conf));
        _apply();
        return 1;
    }
}

static int cmd_scan(__attribute__((unused)) int argc, __attribute__((unused)) char **argv) {
    sim7020_operator_t op;
    int first = 1;

    while (sim7020_scan(&op, first) != 0) {
        first = 0;
        /* 
         * Status symbols: '?' - unknown (0), <space> - available (1), 
         * '*' - current (2), '-' - unavailable (3)
         */
        char *statsym = "? *-"; 
        char stat = statsym[op.stat];
                
        /*
         * Netact: 0 - User-specified GSM, 1 - GSM compact, 3 - EGPRS, 7 - Cat-M, 9 - NB-IoT 
         */
        printf("%c%s: %s (%d)\n", stat, op.longname, op.numname, op.netact);
    }
    return 1;
}

//static int sim7020cmd_init(__attribute__((unused)) int argc, __attribute__((unused)) char **argv) {
int cmd_init(int argc, char **argv) {
    (void) argc; (void) argv;
  return sim7020_init();  
}

static int cmd_reset(__attribute__((unused)) int argc, __attribute__((unused)) char **argv) {
    return sim7020_reset();
}

static int cmd_stop(__attribute__((unused)) int argc, __attribute__((unused)) char **argv) {
    return sim7020_stop();
}

static int cmd_at(int argc, char **argv) {
    char cmd[256];
  
    uint8_t first = 1;
    int i;
    cmd[0] = '\0';
    for (i = 1; i < argc; i++) {
        if (first != 1) {
            (void) strlcat(cmd, " ", sizeof(cmd));
        }
        else
            first = 0;
        (void) strlcat(cmd, argv[i], sizeof(cmd));
    }
    return sim7020_at(cmd);
}

static int cmd_status(__attribute__((unused)) int argc, __attribute__((unused)) char **argv) {
  return sim7020_status();
}

static int cmd_stats(__attribute__((unused)) int argc, __attribute__((unused)) char **argv) {
  
  sim7020_netstats_t *ns = sim7020_get_netstats();

  printf("tx_unicast_count: %" PRIu32 "\n", ns->tx_unicast_count);
  printf("tx_mcast_count: %" PRIu32 "\n", ns->tx_mcast_count);
  printf("tx_success: %" PRIu32 "\n", ns->tx_success);
  printf("tx_failed: %" PRIu32 "\n", ns->tx_failed);
  printf("tx_bytes: %" PRIu32 "\n", ns->tx_bytes);
  printf("rx_count: %" PRIu32 "\n", ns->rx_count);
  printf("rx_bytes: %" PRIu32 "\n", ns->rx_bytes);
  printf("commfail_count: %" PRIu32 "\n", ns->commfail_count);
  printf("reset_count: %" PRIu32 "\n", ns->reset_count);
  printf("activation_count: %" PRIu32 "\n", ns->activation_count);
  printf("activation_fail_count: %" PRIu32 "\n", ns->activation_fail_count);
  {
    extern uint32_t sim7020_activation_usecs;
    if (sim7020_active()) {
      printf("activation_time: %" PRIu32 "\n", sim7020_activation_usecs/1000);
    }
  }
  {
    extern uint64_t sim7020_prev_active_duration_usecs;
    if (sim7020_prev_active_duration_usecs != 0) {
      printf("prev_duration: %" PRIu32 "\n", (uint32_t) (sim7020_prev_active_duration_usecs/1000));
    }
  }
  extern uint32_t longest_send;
  printf("Longest send %" PRIu32 "\n", longest_send);

  return 0;
}

typedef struct sim_command {
    const char *command;             /* Command */
    const char *desc;                /* Command description for help command */
    const char *param_help;          /* Parameter(s) description for help command */
    shell_command_handler_t handler;         /* Handler function */
} sim_command_t;

static const sim_command_t sim_commands[] = {
    { "init", "Init device", NULL, cmd_init },
    { "stats", "Statistics", NULL, cmd_stats },
    { "status", "Report status", NULL, cmd_status },
    { "at", "Send string to device", "<string>", cmd_at },
    { "reset", "Reset", NULL, cmd_reset },
    { "stop", "Stop", NULL, cmd_stop },
    { "apn", "Set APN",  "[<apn>|auto]", cmd_apn},
    { "operator", "Set operator",  "[<operator>|auto]", cmd_operator},
    { "scan", "Operator scan", NULL, cmd_scan },

    { NULL, NULL, NULL, NULL }
};

#define MINMATCH 3
int sim7020cmd_conf(int argc, char **argv) {
    if (argc == 1) {
        printconf();
        return 1;
    }

    const sim_command_t *cmd;
    for (cmd = &sim_commands[0]; cmd->command != NULL; cmd++) {
        if (strcmp(cmd->command, argv[1]) == 0) {
            return cmd->handler(argc, argv);
        }
    }
    /* 
     * Print help 
     */
    char *indent = "";
    for (cmd = &sim_commands[0]; cmd->command != NULL; cmd++) {
        int n = printf("%s%s", indent, cmd->command);
        if (cmd->param_help)
            n += printf(" %s", cmd->param_help);
        int i = 40 - n;
        while (i-- > 0)
            putchar(' ');
        printf("%s\n", cmd->desc);        
    }
    return -1;  
}



