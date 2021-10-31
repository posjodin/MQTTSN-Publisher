MODULE = mqttsn_publisher
include $(RIOTBASE)/Makefile.base

CFLAGS += -DAPPLICATION=\"$(APPLICATION)\"
CFLAGS += -DAPP_WATCHDOG
