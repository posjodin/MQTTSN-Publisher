# name of your application
APPLICATION = simping

# If no BOARD is found in the environment, use this default:
BOARD ?= avr-rss2

# RIOT version -- our label
# RIOT20 or RIOT22
_RIOT_VERSION=RIOT20

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT-OS

ifeq ($(_RIOT_VERSION), RIOT20)
  CFLAGS += -Datmega_reset_cause=_reset_cause
else
  CFLAGS += -Datmega_reset_cause=avr8_reset_cause
endif
EXTERNAL_MODULE_DIRS += $(CURDIR)/..
INCLUDES += -I$(CURDIR)/..

# Network stack to use - sim7020 or gnrc
NETSTACK ?= sim7020
CFLAGS += -DSIM7000G
#CFLAGS += -DSIM7020
#NETSTACK ?= gnrc

# Use UDP ping
CFLAGS += -DUPING

USE_DNS ?= true

USEMODULE += hashes
USEMODULE += mqttsn_publisher
USEMODULE += sntp
USEMODULE += timex
CFLAGS += -DPANIC_REBOOT
CFLAGS += -DTHREAD_STACKSIZE_MAIN=800

FEATURES_REQUIRED += periph_wdt
USEMODULE += atmega_common_periph

BOARD_PROVIDES_NETIF := acd52832 airfy-beacon atmega256rfr2-xpro avr-rss2 b-l072z-lrwan1 cc2538dk fox \
        derfmega128 derfmega256 hamilton iotlab-m3 iotlab-a8-m3 lobaro-lorabox lsn50 mulle microbit msba2 \
        microduino-corerf native nrf51dk nrf51dongle nrf52dk nrf52840dk nrf52840-mdk nrf6310 \
        nucleo-f207zg nucleo-f767zi openmote-b openmote-cc2538 pba-d-01-kw2x remote-pa \
        remote-reva ruuvitag samr21-xpro samr30-xpro spark-core telosb thingy52 yunjia-nrf51822 z1
# include and auto-initialize all available sensors
ifneq (,$(filter $(BOARD),$(BOARD_PROVIDES_NETIF)))
  # Use modules for networking
  # Use netif, netapi, netreg and packet buf from gnrc. 
  USEMODULE += gnrc_netif
  USEMODULE += gnrc_netapi
  USEMODULE += gnrc_netreg
  USEMODULE += gnrc_netif_hdr
  USEMODULE += gnrc_pktbuf
  USEMODULE += netdev_default
  # automatically initialize network interface
  USEMODULE += auto_init_gnrc_netif

  # We use only the lower layers of the GNRC network stack, hence, we can
  # reduce the size of the packet buffer a bit
  CFLAGS += -DGNRC_PKTBUF_SIZE=512
endif

ifeq ($(NETSTACK), sim7020)
  USEMODULE += async_at
  USEMODULE += sim7020_sock_udp
  ifeq ($(USE_DNS), true)
    USEMODULE += sim7020_sock_dns
  endif
#  USEMODULE += auto_init_sim7020
  CFLAGS += -DCONFIG_AT_SEND_SKIP_ECHO=1
  USEMODULE += netstats_l2
endif

USEMODULE += core_mbox
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += schedstatistics
ifeq ($(NETSTACK), gnrc)
USEMODULE += netstats_l2
USEMODULE += netstats_ipv6
USEMODULE += netstats_rpl
endif
USEMODULE += emcute
USEMODULE += xtimer
USEMODULE += at24mac

ifeq ($(NETSTACK), sim7020)
#CFLAGS += -DAT_PRINT_INCOMING=1 -DAT_PRINT_OUTGOING=1
endif

CFLAGS += -DDEBUG_ASSERT_VERBOSE

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Override I2C defaults for lower speed
CFLAGS += -DI2C_NUMOF=1U -DI2C_BUS_SPEED=I2C_SPEED_NORMAL

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Specify custom dependencies for your application here ...

include $(RIOTBASE)/Makefile.include

# ... and define them here (after including Makefile.include,
# otherwise you modify the standard target):

#	./script.py

