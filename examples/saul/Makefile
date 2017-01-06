CONTIKI_PROJECT = saul.c

all: $(CONTIKI_PROJECT)
PROJECT_SOURCEFILES += SAM.c test_saul.c bus_manager.c

# Wformat flag disables printf type checking warnings
CFLAGS += -O -g -Wformat -Wno-unused-variable -Wno-pointer-sign -Wno-incompatible-pointer-types -Wno-discarded-qualifiers -Wno-unused-function 
CONTIKI = ../..
include $(CONTIKI)/Makefile.include
