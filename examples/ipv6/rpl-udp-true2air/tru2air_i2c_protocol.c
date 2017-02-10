/*
 * tru2air_i2c_protocol.c
 *
 *  Created on: Jan 20, 2017
 *      Author: blaise
 */

#ifndef SIMULATED

#include <string.h>
#include <stdbool.h>
#include "tru2air_i2c_protocol.h"
#include "bus_manager.h"
#include "SAM.h"
#include "clock.h"
#include "sys/etimer.h"


/* Temporary Variables */
extern void i2c_slave_data_isr (); //TODO: init_tru2air_snesor_node should require a function pointer to this instead of extern

/* Globals */
volatile tru2air_sensor_node_t DEVICE = {0,0,0};
volatile unsigned char STATE = I2C_SLAVE_LISTEN;
volatile unsigned char ERROR = NO_I2C_ERROR;
unsigned char currentSensor = 0; //TODO: reset on the proper place

void printI2CPkt(i2c_pkt_t *pkt) {
	int i = 0;
	printf("pkt->dev_id: ");
	printf("%d\n",pkt->dev_id);
	printf("pkt->action: ");
	printf("%d\n",pkt->action);
	printf("pkt->error: ");
	printf("%d\n",pkt->error);
	for (i = 0; i < 32; i++) {
		printf("pkt->data[");
		printf("%d",i);
		printf("]: ");
		printf("%d\n",pkt->data[i]);
	}
	printf("pkt->CRC: ");
	printf("%d\n",pkt->CRC);
}


uint16_t crc16(uint8_t* data_p, uint8_t length){
	uint8_t x;
	uint16_t crc = 0xFFFF;

	while (length--){
		x = crc >> 8 ^ *data_p++;
		x ^= x>>4;
		crc = (crc << 8) ^ ((uint16_t)(x << 12)) ^ ((uint16_t)(x <<5)) ^ ((uint16_t)x);
	}
	return crc;
}
void convertLEToBE4 (uint32_t* from, uint32_t* to) {
	char i = 0;
	while(i<4) {
		((char*)to)[3-i] = *(((char*)from)+i);
		i++;
	}
}
uint32_t convertLEToBE4Return (uint32_t* from) {
	uint32_t retVal = 0;
	char i = 0;
	while(i<4) {
		((char*)&retVal)[3-i] = *(((char*)from)+i);
		i++;
	}
	return retVal;
}
bool bus_manager_exchange_pkts(i2c_pkt_t* pkt_out, i2c_pkt_t* pkt_in,uint8_t i2c_addr){
	i2c_pkt_t pktToSend = *pkt_out;
	convertLEToBE4(&pkt_out->dev_id,&pktToSend.dev_id);
	pktToSend.CRC = crc16(((uint8_t*)(&pktToSend)), sizeof(i2c_pkt_t)- sizeof(uint16_t));
	board_i2c_select(BOARD_I2C_INTERFACE_0, i2c_addr);
	if(!board_i2c_write(&pktToSend, sizeof(i2c_pkt_t))) return false;
	if(!board_i2c_read(pkt_in, sizeof(i2c_pkt_t))) return false;
	//convertLEToBE4(&temp,&pkt_in->dev_id); //TODO: Who will be responsible for le2be??CRC checking etc...
	board_i2c_shutdown();
	return true;
}
volatile int testSum = 0;
volatile int testGood = 0;
void i2c_bus_checker(){
	//printf("in bus checker %d\n",STATE);
	uint8_t sensactBuff, i,retvalread,retvalwrite;
	for (i = 8; i< I2C_BUS_ADDRESS_RANGE; i++){
		if(i2c_devices[i]){
			//printf("%d device exsists \n",i);
			board_i2c_select(BOARD_I2C_INTERFACE_0, i);
			retvalwrite = board_i2c_write_single(GET_SENSACT_NUM);
			sensactBuff = 0;
			retvalread = board_i2c_read_single(&sensactBuff); // TODO: Check return value too.
			if(!retvalwrite || (!sensactBuff && retvalread)){
				//printf("%d device deleted \n",i);
				sam_del_device(i2c_devices[i]);
				i2c_devices[i] = 0;
				// TODO: unregister device from SAM and i2c_devices
			}
			board_i2c_shutdown();
		}
	}
	/* Register I2C Slave Interrupt */
	bus_manager_register_i2c_isr(i2c_slave_data_isr);
	/* Inititng I2C SLAVE as 0x25  and  enabling the registered I2C Slave Interrupt */
	bus_manager_init_i2c_slave(0x25);
}
bool errorHandler(bool error, unsigned char errorType){
	if(error){
		STATE = I2C_ERROR;
		ERROR = errorType;
	}
	return error;
}
void init_tru2air_sensor_node(){
	bool tru2air_sensor_device_inited = false;
	unsigned char headerBuff[2];
	unsigned char nameBuff[23];
	unsigned char typeBuff[2];
	unsigned char sensactBuff;
	i2c_pkt_t in_pkt = 		{0,0,0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},0};
	i2c_pkt_t out_pkt = 	{0,0,0,{0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0},0};
	/* Timing variables */
	while (!tru2air_sensor_device_inited) {
		switch (STATE) {
			case (I2C_SLAVE_LISTEN):

				/* Reseting DEVICE */
				DEVICE = (tru2air_sensor_node_t){0,0,0};

				/* Register I2C Slave Interrupt */
				bus_manager_register_i2c_isr(i2c_slave_data_isr);

				/* Inititng I2C SLAVE as 0x25  and  enabling the registered I2C Slave Interrupt */
				bus_manager_init_i2c_slave(0x25);

				//printf("\n[STATE] -> NODE_I2C_SLAVE_LISTEN \n\n");
				tru2air_sensor_device_inited = true;
				break;

			case (NODE_I2C_MASTER_INIT):
				//printf("[STATE] -> NODE_I2C_MASTER_INIT\n");

				if (DEVICE.i2c_addr) {
					I2CIntUnregister(I2C0_BASE);
					bus_manager_disable_i2c_slave();
					//printf("[INFO] disabling i2c slave \n");
					out_pkt.action = GET_SENSACT_NUM;
					out_pkt.data[0]=2;
					out_pkt.data[1]=3;
					out_pkt.dev_id= DEVICE.dev_addr;
					out_pkt.error = 5;
					//out_pkt.CRC = crc16(((uint8_t*)(&out_pkt)), sizeof(i2c_pkt_t)- sizeof(uint16_t));
					bus_manager_exchange_pkts(&out_pkt,&in_pkt,DEVICE.i2c_addr);
					testSum++;
					if (in_pkt.CRC == crc16((uint8_t *)&in_pkt, sizeof(i2c_pkt_t)- sizeof(uint16_t)))
						testGood++;
					printf("testGood: %d, testSum:%d \n",testGood,testSum);
					//printf("[INFO] tru2air sensor node: 0x%08x", convertLEToBE4Return(&in_pkt.dev_id));
					//printI2CPkt(&in_pkt);
					STATE = I2C_SLAVE_LISTEN;
				}
				break;
		}
	}
}


#endif /* If simulated */
