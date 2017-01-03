#include <stdio.h>
#include <string.h>
#include "bus_manager.h"

#ifndef EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_SPGBZ_H_
#define EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_SPGBZ_H_

void init_SAM();

/*!
 * The del_list_item function deletes all sensors with the given dev_id value from the spgbz_list
 * \param dev_id The dev_id of the sensor which should be deleted, often because it disconnected from the bus
 */
void del_list_items(uint32_t dev_id);

/*!
 * The add_list_item function adds a sensor item to the spgbz_list, often because a sensor
 * has been connected to the bus
 * \param sensor The parameters of the sensor
 */
void add_list_item(sensor_descriptor_t sensor);



#endif /* EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_SPGBZ_H_ */
