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

/*! bus_comm_t describes a data type which is returned both from the
 * i2c bus and to the spgbz as a data value from a sensor
 * err is the flag which is true if some kind of error occured during the transmission
 * data is the data returned
 */
typedef struct sensact_rw_result_t {
	unsigned char err;
	double data;
} sensact_rw_result_t;

enum SENSACT_COMM_ERR_T {
	NO_SENSACT_ERROR, /* There was no error */
	WRITE_VALIE_OUT_OF_RANGE, /* The write operation exceeded the sensors input range */
	SENSACT_MISSING /* The adressed sensact is not there (or no longer there) */
};

#endif /* EXAMPLES_SAUL_TRU2AIR_I2C_COM_H_ */
