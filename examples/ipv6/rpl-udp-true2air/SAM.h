#include <stdio.h>
#include <string.h>
#include "bus_manager.h"
#include "tru2air_i2c_protocol.h"

#ifndef EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_SPGBZ_H_
#define EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_SPGBZ_H_

/* Defines */
#define SAM_SENSACTS_MAX_NUM 127

/*!
 * sensor_descriptor_t is a structure which holds all the parameters needed to initialize a sensor
 */
typedef struct sensact_descriptor_t {
	void (*read) (struct sensact_descriptor_t* sensact, sensact_rw_result_t* result);
	void (*write)(struct sensact_descriptor_t* sensact, uint32_t* data, sensact_rw_result_t* result);
	char name[23];
	uint32_t dev_id;
	uint8_t sensact_id;
} sensact_descriptor_t;


//TODO: theoretically this array can be greater that one byte, but not sure if that ever will be the case...
sensact_descriptor_t device_list[SAM_SENSACTS_MAX_NUM];

/* Functions */
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
void sam_add_sensact(sensact_descriptor_t sensor);

//TODO: DOC
void sam_read_sensact(sensact_descriptor_t* sensact, sensact_rw_result_t* result);
void sam_write_sensact(sensact_descriptor_t* sensact, uint32_t* data, sensact_rw_result_t* result);
unsigned char sam_get_sensact_num();

sensact_descriptor_t* sam_get_sensact_by_name(char* name);

#endif /* EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_SPGBZ_H_ */
