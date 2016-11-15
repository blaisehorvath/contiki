#include <stdio.h>
#include <string.h>
#include "bus_manager.h"

#ifndef EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_SPGBZ_H_
#define EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_SPGBZ_H_



/*!
 * The spgbz_list_t is the element of the linked list which holds
 * the current sensors which are on the bus currently.
 * Since the sensors could use different buses than i2c
 * the read and write functions are also in the list item.
 * \param read a function pointer to the function which could read the value of the sensor
 * \param write a function pointer to the function which could write the value of the sensor
 * \name the name of the sensor
 * \dev_id is the device id of the sensor module
 * \sensor_id is the number of the sensor in the sensor module
 * */
typedef struct spgbz_list_item {
	bus_comm_t (*read) (int32_t,char );
	bus_comm_t (*write)(int32_t,char, double);
	char name[23];
	uint32_t dev_id;
	uint8_t sensor_id;
} spgbz_list_t;


void init_spgbz();

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
