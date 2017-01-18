#include "dev/leds.h"
#include "tru2air_comm.h"
#include "bus_manager.h"

#define ONBOARD_DEV_ADDR 1


void set_red_led (uint32_t* dev_addr, char* sensact_id, double toWrite, sensact_rw_result_t* result);
void read_red_led (uint32_t* dev_addr, char* sensact_id, sensact_rw_result_t* result);
void set_green_led (uint32_t* dev_addr, char* sensact_id, double toWrite, sensact_rw_result_t* result);
void read_green_led (uint32_t* dev_addr, char* sensact_id, sensact_rw_result_t* result);
