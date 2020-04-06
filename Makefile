# name of your application
APPLICATION = mqtt_publisher

# If no BOARD is found in the environment, use this default:
BOARD ?= avr-rss2

# This has to be the absolute path to the RIOT base directory:
RIOTBASE ?= $(CURDIR)/../RIOT-OS

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
#USEMODULE += gnrc_icmpv6_echo
# Add also the shell, some shell commands
USEMODULE += shell
USEMODULE += shell_commands
USEMODULE += ps
USEMODULE += netstats_l2
USEMODULE += netstats_ipv6
USEMODULE += netstats_rpl

USEMODULE += emcute
USEMODULE += xtimer
USEMODULE += at24mac

# Comment this out to disable code in RIOT that does safety checking
# which is not needed in a production environment but helps in the
# development process:
DEVELHELP ?= 1

# Uncomment the following 2 lines to specify static link lokal IPv6 address
# this might be useful for testing, in cases where you cannot or do not want to
# run a shell with ifconfig to get the real link lokal address.
#IPV6_STATIC_LLADDR ?= '"fe80::cafe:cafe:cafe:1"'
#CFLAGS += -DGNRC_IPV6_STATIC_LLADDR=$(IPV6_STATIC_LLADDR)

# Uncomment this to join RPL DODAGs even if DIOs do not contain
# DODAG Configuration Options (see the doc for more info)
# CFLAGS += -DGNRC_RPL_DODAG_CONF_OPTIONAL_ON_JOIN

# Change this to 0 show compiler invocation lines by default:
QUIET ?= 1

include $(RIOTBASE)/Makefile.include

# Set a custom channel if needed
# For KTH Contiki gateway
#
# ifconfig 7 add unicast fd02::3c8b:2185:3d8b:2184/64
# ifconfig 7 add unicast fd02::3c8b:2185:3d8b:9999/64
# ping6 64:ff9b::c010:7dea -- gridgw.ssvl.kth.se
# usage: udp send <addr> <port> <data> [<num> [<delay in us>]]
# gridgw.ssvl.kth.se:
# udp send 64:ff9b::c010:7dea 8888 hejhopp
# udp send 64:ff9b::82ed:ca25 8888 hejhopp
# con 64:ff9b::c0a8:0196 10000
#
# pub KTH/avr-rss2/fcc23d00000007ce/sensors [{"bn": "urn:dev:mac:fcc23d00000007ce;", "bt": 4711},{"n": "hello", "vs": "hello from RIOT" }]
#
DEFAULT_CHANNEL=25
DEFAULT_PAN_ID=0xFEED
include $(RIOTMAKE)/default-radio-settings.inc.mk
