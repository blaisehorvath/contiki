CONTIKI_PROJECT = saul.c

all: $(CONTIKI_PROJECT)
PROJECT_SOURCEFILES += spgbz.c test_saul.c board-i2c.c bus_manager.c

# Wformat flag disables printf type checking warnings
CFLAGS += -O -g -Wformat -Wno-unused-variable -Wno-pointer-sign -Wno-incompatible-pointer-types
CONTIKI = ../..
include $(CONTIKI)/Makefile.include
