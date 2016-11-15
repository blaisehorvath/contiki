/*
 * bus_manager.h
 *
 *  Created on: Nov 9, 2016
 *      Author: v
 */

#ifndef EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_BUS_MANAGER_H_
#define EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_BUS_MANAGER_H_

#include <stdint.h>

/*! bus_comm_t describes a data type which is returned both from the
 * i2c bus and to the spgbz as a data value from a sensor
 * err is the flag which is true if some kind of error occured during the transmission
 * data is the data returned
 */
typedef struct bus_comm{
	uint8_t err;
	double data;
} bus_comm_t;

/*!
 * I2C_list_t is one element of the linked of I2C modules
 * dev_id holds the dev_id which is burned into the sensor firmware, and used to identify the sensor
 * addr is the address where the sensor can be accessed
 */
typedef struct I2C_list_item{
	uint32_t dev_id;
	uint8_t addr;
	struct I2C_list_item* NEXT;
} I2C_list_t;

/*!
 * sensor_descriptor_t is a structure which holds all the parameters needed to initialize a sensor
 */
typedef struct sensor_descriptor_item{
	bus_comm_t (*read) (int32_t,char );
	bus_comm_t (*write)(int32_t,char, double);
	char name[23];
	uint32_t dev_id;
	uint8_t sensor_num;
} sensor_descriptor_t;
/*!
 * i2c_read function is the function which is called when
 * one tries to access a sensor which is attached to the i2c bus
 * The slave knows the difference between read or write from the number of written bytes
 * Node	|														| Sensor
 * 		|----------W,addr|uint32_t dev_id|uin8_t sensor_id----->|
 * 		|<---------------R,addr|bus_comm_t data|--------------->| Master request, slave answer
 */
bus_comm_t i2c_read(uint32_t dev_id, uint8_t sensor_id);
/*!
 * i2c_write function is the function which is called when
 * one tries to access a sensor which is attached to the i2c bus
 * The slave knows the difference between read or write from the number of written bytes
 * Node	|														| Sensor
 * 		|--W,addr|uint32_t dev_id|uin8_t sensor_id,double data->|
 * 		|<---------------R,addr|bus_comm_t data|--------------->| Master request, slave answer
 */
bus_comm_t i2c_write(uint32_t dev_id, uint8_t sensor_id,double data);
/*!
 * The bus_check function checks if there's any new or disconnected sensors and
 * calls the add or remove function from the spgbz accordingly
 */
void bus_check(void);

#endif /* EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_BUS_MANAGER_H_ */
