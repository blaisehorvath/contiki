all: udp-client udp-server
CONTIKI=../../..
PROJECT_SOURCEFILES += true2air_prot.c bus_manager.c
CFLAGS += -DPROJECT_CONF_H=\"project-conf.h\" -Wno-discarded-qualifiers -Wno-pointer-sign -Wno-unused-variable -Wno-unused-function

ifdef WITH_COMPOWER
APPS+=powertrace
CFLAGS+= -DCONTIKIMAC_CONF_COMPOWER=1 -DWITH_COMPOWER=1 -DQUEUEBUF_CONF_NUM=4
endif

ifdef SERVER_REPLY
CFLAGS+=-DSERVER_REPLY=$(SERVER_REPLY)
endif
ifdef PERIOD
CFLAGS+=-DPERIOD=$(PERIOD)
endif
ifdef SIMULATED
CFLAGS+=-DSIMULATED=$(SIMULATED)
endif
ifdef WITH_BME280
CFLAGS+=DWITH_BME280=$(WITH_BME280)
endif
CONTIKI_WITH_IPV6 = 1
include $(CONTIKI)/Makefile.include


