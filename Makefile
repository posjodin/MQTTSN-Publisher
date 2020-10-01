# name of your application
APPLICATION = mqttsn_publisher

# If no BOARD is found in the environment, use this default:
BOARD ?= avr-rss2

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT-OS

# Network stack to use - sim7020 or gnrc

NETSTACK ?= sim7020

ifeq ($(NETSTACK), sim7020)
  USEMODULE += sim7020_ipv6
  USEMODULE += sim7020_sock_udp
  USEMODULE += auto_init_sim7020
  USEMODULE += at
  USEMODULE += at_urc
  USEMODULE += ipv6_addr
  USEMODULE += ipv4_addr
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
USEMODULE += sock_dns
USEMODULE += core_mbox
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
ifeq ($(NETSTACK), gnrc)
USEMODULE += netstats_l2
USEMODULE += netstats_ipv6
USEMODULE += netstats_rpl
endif
USEMODULE += emcute
USEMODULE += xtimer
USEMODULE += at24mac

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
APPDEPS = application.h
ifeq ($(NETSTACK), gnrc)
  APPDEPS += gnrc_rpl.o
endif

include $(RIOTBASE)/Makefile.include

# ... and define them here (after including Makefile.include,
# otherwise you modify the standard target):

#	./script.py

application.h: Makefile
	echo '#ifndef APPLICATION' >$@
	echo '#define APPLICATION "'$(APPLICATION)'"' >>$@
	echo '#endif' >>$@

# Set a custom channel 
# For KTH Contiki gateway
#
DEFAULT_CHANNEL=25
DEFAULT_PAN_ID=0xFEED
include $(RIOTMAKE)/default-radio-settings.inc.mk

