# name of your application
APPLICATION = mqttsn_publisher

# If no BOARD is found in the environment, use this default:
BOARD ?= avr-rss2

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT-OS

EXTERNAL_MODULE_DIRS += $(CURDIR)/mqttsn_publisher
INCLUDES += -I$(CURDIR)/mqttsn_publisher

# Network stack to use - sim7020 or gnrc
NETSTACK ?= sim7020

USE_DNS ?= true

USEMODULE += mqttsn_publisher

ifeq ($(NETSTACK), sim7020)
  USEMODULE += sim7020_sock_udp
  ifeq ($(USE_DNS), true)
    USEMODULE += sim7020_sock_dns
  endif
  USEMODULE += auto_init_sim7020
else ifeq ($(NETSTACK), gnrc)
  # Include packages that pull up and auto-init the link layer.
  # NOTE: 6LoWPAN will be included if IEEE802.15.4 devices are present
  USEMODULE += gnrc_netdev_default
  USEMODULE += auto_init_gnrc_netif
  # Activate ICMPv6 error messages
  USEMODULE += gnrc_icmpv6_error
  # Specify the mandatory networking modules for IPv6 and UDP
  USEMODULE += gnrc_ipv6_router_default
  USEMODULE += gnrc_udp
  # Add a routing protocol
  USEMODULE += gnrc_rpl
  USEMODULE += gnrc_rpl_srh
  USEMODULE += auto_init_gnrc_rpl
  # This application dumps received packets to STDIO using the pktdump module
  #USEMODULE += gnrc_pktdump
  # Additional networking modules that can be dropped if not needed
  USEMODULE += gnrc_icmpv6_echo
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
CFLAGS += -DAT_PRINT_INCOMING=1
else ifeq ($(USE_DNS), true)
  USEMODULE += sock_dns
  # Need resolver for DNS lookups with gnrc
  CFLAGS += -DDNS_RESOLVER=\"::ffff:0808:0808\"
endif

# Enable publisher thread
CFLAGS += -DMQTTSN_PUBLISHER_THREAD
# Autolauch at startup
CFLAGS += -DAUTO_INIT_MQTTSN

CFLAGS += -DDEBUG_ASSERT_VERBOSE

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

ifeq ($(NETSTACK), gnrc)
  # Uncomment the following 2 lines to specify static link lokal IPv6 address
  # this might be useful for testing, in cases where you cannot or do not want to
  # run a shell with ifconfig to get the real link lokal address.
  #IPV6_STATIC_LLADDR ?= '"fe80::cafe:cafe:cafe:1"'
  #CFLAGS += -DGNRC_IPV6_STATIC_LLADDR=$(IPV6_STATIC_LLADDR)

  # Uncomment this to join RPL DODAGs even if DIOs do not contain
  # DODAG Configuration Options (see the doc for more info)
  # CFLAGS += -DGNRC_RPL_DODAG_CONF_OPTIONAL_ON_JOIN

  # For RPL auto initialization
CFLAGS += -DGNRC_RPL_DEFAULT_NETIF=6
endif

# Override I2C defaults for lower speed
CFLAGS += -DI2C_NUMOF=1U -DI2C_BUS_SPEED=I2C_SPEED_NORMAL

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

# Specify custom dependencies for your application here ...
ifeq ($(NETSTACK), gnrc)
  APPDEPS += gnrc_rpl.o
endif

include $(RIOTBASE)/Makefile.include

# ... and define them here (after including Makefile.include,
# otherwise you modify the standard target):

#	./script.py

# Set a custom channel 
# For KTH Contiki gateway
#
DEFAULT_CHANNEL=25
DEFAULT_PAN_ID=0xFEED
include $(RIOTMAKE)/default-radio-settings.inc.mk

