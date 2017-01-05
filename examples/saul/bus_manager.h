/*
 * bus_manager.h
 *
 *  Created on: Nov 9, 2016
 *      Author: v
 */

#ifndef EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_BUS_MANAGER_H_
#define EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_BUS_MANAGER_H_

#include <stdint.h>
#include "board-i2c.h"
#include "tru2air_spi.h"
#include "ti-lib.h"

/**
 * This function initializes the bus manager.
 *
 * @section Description
 * Every element of the devices array has to be {0, 0} because that means that there is
 * no device available on that address.
 */
void init_i2c_bus_manager ();

/**
 * This function is used to get the lowest possible empty i2c address for a new device.
 * @param uint32_t devAddr Is the address that was burnt into the sensor device's firmware.
 * @returns uint8_t 0 if error happens else returns a valid i2c slave address (1-127) because the address 0 is reserved for the msp430 on the Tru2Air node.
 */
uint8_t register_i2c_device();


/**
 * This function removes a i2c device from the managed devices
 * @section Destription
 * When a device is no longer reachable it is automatically removed. This function also calls the proper function
 * that removes every sensor related to this device from the spgbz.
 */
void remove_i2c_device (uint8_t i2c_addr);

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
 * This array holds the device ids of the connected i2c devices.
 *
 * @section Description
 * Every device has a burnt in dev_id in the sensor firmware, and it is used to identify the device.
 * The array index represents the i2c slave address of the item.
 */
uint32_t i2c_devices [127];

/*!
 * sensor_descriptor_t is a structure which holds all the parameters needed to initialize a sensor
 */
typedef struct sensor_descriptor_item{
	bus_comm_t (*read) (int32_t,char );
	bus_comm_t (*write)(int32_t,char, double);
	char name[23];
	uint32_t dev_id;
	uint8_t sensor_id;
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
 * The scan_bus function checks if there's any new or disconnected sensors and
 * calls the add or remove function from the spgbz accordingly
 */

_Bool detectDevice (uint8_t address);
void scan_bus(void);


#endif /* EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_BUS_MANAGER_H_ */
