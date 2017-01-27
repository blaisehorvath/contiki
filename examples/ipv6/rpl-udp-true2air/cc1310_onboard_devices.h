#include "dev/leds.h"
#include "SAM.h"


#define TRU2AIR_CLIENT_ONBOARD_LEDS 1

void set_red_led (sensact_descriptor_t* sensor, uint32_t* toWrite, sensact_rw_result_t* result);
void read_red_led (sensact_descriptor_t* sensact, sensact_rw_result_t* result);
void set_green_led (sensact_descriptor_t* sensor, uint32_t* toWrite, sensact_rw_result_t* result);
void read_green_led (sensact_descriptor_t* sensact, sensact_rw_result_t* result);

#ifndef SIMULATED

#include "ti-lib.h"
#include "gpio.h"

#define TRU2AIR_CLIENT_ONBOARD_RELAYS 2
#define TRU2AIR_CLIENT_ONBOARD_RELAYS_NUM 4


void read_relay(sensact_descriptor_t* sensact, sensact_rw_result_t* result);
void write_relay(sensact_descriptor_t* sensact, uint32_t* setValue, sensact_rw_result_t* result);

#endif

