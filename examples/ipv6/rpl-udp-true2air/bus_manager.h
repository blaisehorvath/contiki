/*
 * bus_manager.h
 *
 *  Created on: Nov 9, 2016
 *      Author: v
 */

#ifndef SIMULATED
#ifndef EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_BUS_MANAGER_H_
#define EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_BUS_MANAGER_H_

#include <stdint.h>
#include <stdbool.h>
#include "SAM.h"
#include "ti-lib.h"


// DEFINES
#define NO_INTERFACE 0xFF
#define CC1310_IOID_SDA	22//13
#define CC1310_IOID_SCL 24 //14
#define I2C_BUS_ADDRESS_RANGE 127
//Tru2Air protocol defines
#define TRU2AIR_HEADER_BUFF_SIZE 2
#define TRU2AIR_SENSOR_RETURN_TYPE_SIZE 1

/*---------------------------------------------------------------------------*/
#define BOARD_I2C_INTERFACE_0     0
#define BOARD_I2C_INTERFACE_1     1
/*---------------------------------------------------------------------------*/

//TODO: doc
bool board_i2c_read_until(uint8_t *data, char end);
void bus_manager_disable_i2c_slave();
void bus_manager_init_i2c_slave(uint8_t slave_addr);
static bool accessible(void);
static bool i2c_status();
void bus_manager_clear_i2c_slave_data_int();
void bus_manager_register_i2c_isr (void (i2c_slave_data_isr)());
void bus_manager_w_sensact(sensact_descriptor_t* sensact, unsigned int* toWrite, sensact_rw_result_t* result);
void bus_manager_r_sensact(sensact_descriptor_t* sensact, sensact_rw_result_t* result);


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
uint8_t bus_manager_register_i2c_device();


/**
 * This function removes a i2c device from the managed devices
 * @section Destription
 * When a device is no longer reachable it is automatically removed. This function also calls the proper function
 * that removes every sensor related to this device from the spgbz.
 */
void bus_manager_unregister_i2c_device (uint8_t i2c_addr);

/*!
 * This array holds the device ids of the connected i2c devices.
 *
 * @section Description
 * Every device has a burnt in dev_id in the sensor firmware, and it is used to identify the device.
 * The array index represents the i2c slave address of the item.
 */
uint32_t i2c_devices [127];

/*!
 * This function returns the i2c address of a sensact if it was registered by the bus manager.
 * If the device is not found the unsigned int 0x00 is returned
 */
uint8_t bus_manager_get_sensact_i2c_id (uint32_t* device_id);

/*!
 * i2c_read function is the function which is called when
 * one tries to access a sensor which is attached to the i2c bus
 * The slave knows the difference between read or write from the number of written bytes
 * Node	|														| Sensor
 * 		|----------W,addr|uint32_t dev_id|uin8_t sensor_id----->|
 * 		|<---------------R,addr|bus_comm_t data|--------------->| Master request, slave answer
 */
sensact_rw_result_t i2c_read(uint32_t dev_id, uint8_t sensor_id);
/*!
 * i2c_write function is the function which is called when
 * one tries to access a sensor which is attached to the i2c bus
 * The slave knows the difference between read or write from the number of written bytes
 * Node	|														| Sensor
 * 		|--W,addr|uint32_t dev_id|uin8_t sensor_id,double data->|
 * 		|<---------------R,addr|bus_comm_t data|--------------->| Master request, slave answer
 */
sensact_rw_result_t i2c_write(uint32_t dev_id, uint8_t sensor_id,double data);

/**
 * \brief Select an I2C slave
 * \param interface The I2C interface to be used (BOARD_I2C_INTERFACE_0 or _1)
 * \param slave_addr The slave's address
 *
 * The various sensors on the sensortag are connected either on interface 0 or
 * 1. All sensors are connected to interface 0, with the exception of the MPU
 * that is connected to 1.
 */
void board_i2c_select(uint8_t interface, uint8_t slave_addr);

/**
 * \brief Burst read from an I2C device
 * \param buf Pointer to a buffer where the read data will be stored
 * \param len Number of bytes to read
 * \return True on success
 */
bool board_i2c_read(uint8_t *buf, uint8_t len);

/**
 * \brief Burst write to an I2C device
 * \param buf Pointer to the buffer to be written
 * \param len Number of bytes to write
 * \return True on success
 */
bool board_i2c_write(uint8_t *buf, uint8_t len);

/**
 * \brief Single write to an I2C device
 * \param data The byte to write
 * \return True on success
 */
bool board_i2c_write_single(uint8_t data);


/**
 * \brief Enables the I2C peripheral with defaults
 *
 * This function is called to wakeup and initialise the I2C.
 *
 * This function can be called explicitly, but it will also be called
 * automatically by board_i2c_select() when required. One of those two
 * functions MUST be called before any other I2C operation after a chip
 * sleep / wakeup cycle or after a call to board_i2c_shutdown(). Failing to do
 * so will lead to a bus fault.
 */
void board_i2c_wakeup(void);

/**
 * \brief Stops the I2C peripheral and restores pins to s/w control
 *
 * This function is called automatically by the board's LPM logic, but it
 * can also be called explicitly.
 */
void board_i2c_shutdown(void);

#endif /* EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_BUS_MANAGER_H_ */
#endif /**/
