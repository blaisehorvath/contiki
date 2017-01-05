/*
 * This header contains the functions and contants that is needed for the tru2air i2c commonication protocol between the node and the sensor controller module
 * tru2air_i2c_com.h
 *
 *  Created on: Dec 27, 2016
 *      Author: blaise
 */

#ifndef EXAMPLES_SAUL_TRU2AIR_I2C_COM_H_
#define EXAMPLES_SAUL_TRU2AIR_I2C_COM_H_

enum I2C_COMM_PROT_ACTION {GET_SENSACT_NUM, GET_SENSOR_NAME, GET_SENSOR_TYPE, SENSOR_READ, SENS_ACT_WRITE};

enum TRU2AIR_SENSOR_DATA_TYPE {SENS_DOUBLE, SENS_UINT32 };

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

#endif /* EXAMPLES_SAUL_TRU2AIR_I2C_COM_H_ */
