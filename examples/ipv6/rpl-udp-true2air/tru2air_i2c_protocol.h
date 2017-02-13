/*
 * This header contains the functions and contants that is needed for the tru2air i2c commonication protocol between the node and the sensor controller module
 * tru2air_i2c_com.h
 *
 *  Created on: Dec 27, 2016
 *      Author: blaise
 */

#ifndef EXAMPLES_SAUL_TRU2AIR_I2C_COM_H_
#define EXAMPLES_SAUL_TRU2AIR_I2C_COM_H_
#include <stdbool.h>
#include <stdint.h>
#include "SAM.h"
enum TRU2AIR_I2C_HEADER_ACTION {
	GET_SENSACT_NUM,
	GET_SENSOR_NAME,
	GET_SENSOR_TYPE,
	SENS_ACT_READ,
	SENS_ACT_WRITE,
	CHECK_ALIVE
};

enum TRU2AIR_CLIENT_NODE_I2C_HANDLER_STATE {
	I2C_SLAVE_LISTEN,
	NODE_I2C_MASTER_INIT,
	REQUIRE_SENSACT_NAME,
	REQUIRE_SENSOR_RETURN_TYPE,
	REGISTER_TO_SAM,
	I2C_ERROR
};

enum TRU2AIR_I2C_ERROR {
	NO_I2C_ERROR,
	I2C_SENSACT_NUM_NULL_ERROR,
	I2C_SENSACT_NUM_ERROR,
    I2C_GET_SENSOR_TYPE_ERROR,
    I2C_GET_SENSOR_NAME_ERROR,
    I2C_CRC_ERROR
};


/**
 * A stuct that hold an action (see I2C_COMM_PROT_ACTION and e specifier.
 * The specifier specifies the target of the action.
 */
typedef struct tru2air_header_t {
  unsigned char action;
  unsigned char specifier;
} tru2air_header_t;

/**
 * This struct describes a tru2air client node
 */
typedef struct tru2air_sensor_node_t {
    unsigned int dev_addr;
    unsigned char i2c_addr;
    unsigned char sensact_num;
} tru2air_sensor_node_t;
// buffer in Wire.h should be set accordingly, default is 32... both in wire and twi.
typedef struct i2c_pkt_t {
	uint32_t dev_id;
	enum TRU2AIR_I2C_HEADER_ACTION action;
	enum TRU2AIR_I2C_ERROR error;
	unsigned char data[32];
	uint16_t CRC;
} i2c_pkt_t;
#ifndef SIMULATED
/*
 *  Functions
 */
void init_tru2air_sensor_node();
void i2c_bus_checker(void);
bool bus_manager_exchange_pkts(i2c_pkt_t* pkt_out, i2c_pkt_t* pkt_in,uint8_t address);
void convertLEToBE4 (uint32_t* from, uint32_t* to);
uint32_t convertLEToBE4Return (uint32_t* from);
uint16_t crc16(uint8_t* data_p, uint8_t length);
bool bus_manager_r_sensact(sensact_descriptor_t* sensact, uint8_t* resultData);
bool bus_manager_w_sensact(sensact_descriptor_t* sensact, uint8_t* writeData, uint8_t* resultData);
#endif

#endif /* EXAMPLES_SAUL_TRU2AIR_I2C_COM_H_ */
