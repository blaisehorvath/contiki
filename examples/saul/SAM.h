#include <stdio.h>
#include <string.h>
#include "bus_manager.h"

#ifndef EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_SPGBZ_H_
#define EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_SPGBZ_H_

void sam_init();

/*!
 * The del_list_item function deletes all sensors with the given dev_id value from the SAM
 * \param dev_id The dev_id of the sensor which should be deleted, often because it disconnected from the bus
 */
void sam_del_device(uint32_t dev_id);

/*!
 * The add_list_item function adds a sensor item to the SAM, often because a sensor
 * has been connected to the bus
 * \param sensor The parameters of the sensor
 */
void sam_add_sensact(sensor_descriptor_t sensor);

//TODO: DOC
void sam_read_sensact(uint32_t device_addr, char sensact_addr, sensact_rw_result_t* result);
void sam_write_sensact(uint32_t device_addr, char sensact_addr, double data, sensact_rw_result_t* result);
unsigned char sam_get_sensact_num();
sensor_descriptor_t* sam_get_sensact_by_name(char* name);

#endif /* EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_SPGBZ_H_ */
