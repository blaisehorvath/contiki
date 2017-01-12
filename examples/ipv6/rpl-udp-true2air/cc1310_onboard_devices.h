#include "dev/leds.h"
#include "tru2air_comm.h"
#include "bus_manager.h"

#define ONBOARD_DEV_ADDR 1

void set_red_led (double toWrite, sensact_rw_result_t* result);
void read_red_led (sensact_rw_result_t* result);

void set_green_led (double toWrite, sensact_rw_result_t* result);
void read_green_led (sensact_rw_result_t* result);
