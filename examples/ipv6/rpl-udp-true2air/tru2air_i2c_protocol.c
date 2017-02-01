/*
 * tru2air_i2c_protocol.c
 *
 *  Created on: Jan 20, 2017
 *      Author: blaise
 */

#include <string.h>
#include "tru2air_i2c_protocol.h"
#include "bus_manager.h"
#include "SAM.h"

#ifndef SIMULATED


/* Temporary Variables */
extern void i2c_slave_data_isr (); //TODO: init_tru2air_snesor_node should require a function pointer to this instead of extern

/* Globals */
volatile  tru2air_sensor_node_t DEVICE = {0,0,0};
volatile enum TRU2AIR_CLIENT_NODE_I2C_HANDLER_STATE STATE = I2C_SLAVE_LISTEN;
unsigned char currentSensor = 0; //TODO: reset on the proper place

void init_tru2air_sensor_node(){
	bool tru2air_sensor_device_inited = false;
	unsigned char headerBuff[2];
	unsigned char nameBuff[23];
	unsigned char typeBuff[2];
	unsigned char sensactBuff[5];

	while (!tru2air_sensor_device_inited) {
		switch (STATE) {
			case (I2C_SLAVE_LISTEN):

				/* Reseting DEVICE */
				DEVICE = (tru2air_sensor_node_t){0,0,0};

				printf("\n[STATE] -> INIT \n[INFO] sensor node i2c_id: 0x%02x dev_addr: 0x%08x \n", DEVICE.i2c_addr, DEVICE.dev_addr);

				/* Register I2C Slave Interrupt */
				bus_manager_register_i2c_isr(i2c_slave_data_isr);

				/* Inititng I2C SLAVE as 0x10  and  enabling the registered I2C Slave Interrupt */
				bus_manager_init_i2c_slave(0x10);

				printf(
						"[STATE] -> NODE_I2C_SLAVE_INIT \n[INFO] sensor node i2c_id: 0x%02x dev_addr: 0x%08x \n",
						DEVICE.i2c_addr, DEVICE.dev_addr);
				tru2air_sensor_device_inited = true;
				break;

			case (NODE_I2C_MASTER_INIT):

				printf("[STATE] -> NODE_I2C_MASTER_INIT\n");

				if (DEVICE.i2c_addr) {

					//Unregister the slave interrupt and turn off slave mode
					I2CIntUnregister(I2C0_BASE);
					printf("[INFO] unregistered the slave interrupts \n");

					/* Disabling power to slave module and disabling i2c slave data interrupt */
					bus_manager_disable_i2c_slave();
					printf("[INFO] disabling i2c slave \n");

					// Wake up as master
					board_i2c_select(BOARD_I2C_INTERFACE_0, DEVICE.i2c_addr);
					board_i2c_write_single(GET_SENSACT_NUM);
					board_i2c_read(sensactBuff, 5); //TODO: error handling, check if the dev address is valid and maybe if the SENSACT num is >0?
					DEVICE.sensact_num = sensactBuff[4];
					currentSensor = 0;
					board_i2c_shutdown();

					printf("[INFO] tru2air sensor node: 0x%08x has 0x%02x sensors\n",
							DEVICE.dev_addr, DEVICE.sensact_num);

					STATE = REQUIRE_SENSACT_NAME;
				}
				break;

			case (REQUIRE_SENSACT_NAME):

				printf("[STATE] -> GET_SENSOR_NAME\n");

				headerBuff[0] = GET_SENSOR_NAME;
				headerBuff[1] = currentSensor;


				board_i2c_select(BOARD_I2C_INTERFACE_0, DEVICE.i2c_addr);
				board_i2c_write(headerBuff, TRU2AIR_HEADER_BUFF_SIZE);
				board_i2c_read_until(nameBuff, '\0');
				board_i2c_shutdown();

				STATE = REQUIRE_SENSOR_RETURN_TYPE;

				break;

			case (REQUIRE_SENSOR_RETURN_TYPE):

				printf("[STATE] -> GET_SENSOR_TYPE\n");

				headerBuff[0] = GET_SENSOR_TYPE;
				headerBuff[1] = currentSensor;

				board_i2c_select(BOARD_I2C_INTERFACE_0, DEVICE.i2c_addr);
				board_i2c_write(headerBuff, TRU2AIR_HEADER_BUFF_SIZE);
				board_i2c_read(typeBuff, 2);
				board_i2c_shutdown();

				printf("[GOT SENSACT] Name: %s Type: 0x%04x \n", nameBuff, *(uint16_t*)typeBuff);

				STATE = REGISTER_TO_SAM;
				break;

			case ( REGISTER_TO_SAM ):

				printf("[STATE] -> REGISTER_TO_SAM\n");
				sensact_descriptor_t new_sensact;

				strcpy((char*)&(new_sensact.name),nameBuff);
				new_sensact.dev_id = DEVICE.dev_addr;
				new_sensact.sensact_id = currentSensor;
				new_sensact.read = bus_manager_r_sensact;
				new_sensact.write = bus_manager_w_sensact;
				new_sensact.sensact_type = *((uint16_t*)typeBuff);

				sam_add_sensact(new_sensact);

				if (++currentSensor == DEVICE.sensact_num) {
					currentSensor = 0;
					STATE = I2C_SLAVE_LISTEN;
				} else {
					STATE = REQUIRE_SENSACT_NAME;
				}

				break;

			default:
				printf(
						"[STATE] -> DEFAULT\n[INFO] tru2air sensor node i2c_id: 0x%02x dev_addr: 0x%08x \n",
						DEVICE.i2c_addr, DEVICE.dev_addr);
				break;
		}
	}
}

void init_sensact () {

};

#endif /* If simulated */
