/*
 * true2air_prot.c
 *
 *  Created on: Oct 11, 2016
 *      Author: v
 */
#include "true2air_prot.h"

#ifndef SIMULATED
#include "bus_manager.h"
#endif

#include <stdio.h>
#include <stdint.h>
#include "net/ip/uip.h"
#include "SAM.h"

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"

#ifndef SIMULATED
#endif

#define NO_INTERFACE 0xFF

/* Includes and globals by Blaise*/
/* for SAM:*/
//extern sensact_descriptor_t device_list[];

uint8_t node_initialized = 0;
uint8_t slave_addr = 0x02;
int node_pkt_reply(rfnode_pkt* pkt_in, rfnode_pkt* pkt_out)
{
	node_initialized = 1;
	print_pkt_without_addr(pkt_in);
	pkt_out->pkt_cnt = pkt_in->pkt_cnt;
	switch(pkt_in->msg){

		case SET_IPADDR:
			printf("SET IPADDR, init:%d\n",node_initialized);
			pkt_out->msg = SET_IPADDR;// Don't answer
			pkt_out->data = 0;
			pkt_out->new_device = node_initialized;
			sprintf(pkt_out->name,"REPLY SET_IPADDR!");
			pkt_out->cnt = 2;
			return 1;

		/* Get the number of sensors actuators the tru2air node has*/
		case GET_SENSACT_LIST:
			pkt_out->msg = SENSACT_LIST_ACK;
			pkt_out->data = 0;
			pkt_out->new_device = 0;
			sprintf(pkt_out->name,"REPLY FROM NODE!");
			pkt_out->cnt = sam_get_sensact_num();
			return 1;

		/* Senging the info of the pkt_in->cnt'th sensor's name */
		case SENSACT_LIST_ITEM:
			pkt_out->msg = SENSACT_LIST_ITEM;
			pkt_out->data = 0;
			pkt_out->new_device = 0;
			pkt_out->cnt = pkt_in->cnt;

			if (pkt_in->cnt >= 0 && pkt_in->cnt < SAM_SENSACTS_MAX_NUM) sprintf(pkt_out->name, device_list[pkt_in->cnt].name);
			else { pkt_out->msg = ERROR_PKT_MSG; }
			return 1;

		/* Send something mesured by the pkt_in'th sensor */
		case GET_SENSACT: // Dummy sensor handler

			pkt_out->msg = GET_SENSACT_ACK;
			pkt_out->new_device = 0;
			pkt_out->cnt = pkt_in->cnt;

			sensact_rw_result_t result;

			if( (pkt_in->cnt >= 0 && pkt_in->cnt < SAM_SENSACTS_MAX_NUM) && !strcmp(device_list[pkt_in->cnt].name, pkt_in->name)) { //This ordering of the check protects against buffer overflow and undefined behaviour too

				device_list[pkt_in->cnt].read(&device_list[pkt_in->cnt], &result);

				if(result.err == NO_SENSACT_ERROR) {
					sprintf(pkt_out->name, device_list[pkt_in->cnt].name);
					pkt_out->data = result.data;
				}
				else {
					sprintf(pkt_out->name, "SENSACT ERROR"); //TODO: use my error types somehow
				}

			} else {
				sprintf(pkt_out->name, "SENSACT ERROR"); //TODO: same as above
			}

#ifdef WITH_BME280
			else if(pkt_in->cnt == 2 && !strcmp(pkt_in->name,"TEMP")){
				sprintf(pkt_out->name,"TEMP");
				  board_i2c_select(0,slave_addr);

				  uint8_t data[24];


				  board_i2c_read(&data[0],24);
				  board_i2c_shutdown();


				  double temp = 0;
				  double pres = 0;
				  double hum = 0;
				  int i = 0;
				  for (i = 0; i<8; i++) {
					  *(((char*)&temp)+i)=data[i];
					  *(((char*)&pres)+i)=data[i+8];
					  *(((char*)&hum)+i)=data[i+16];
				  }
				pkt_out->data = (uint32_t)(temp*1000);
			}
			else if(pkt_in->cnt == 3 && !strcmp(pkt_in->name,"PRESS")){
				sprintf(pkt_out->name,"PRESS");
				  board_i2c_select(0,slave_addr);

				  uint8_t data[24];


				  board_i2c_read(&data[0],24);
				  board_i2c_shutdown();


				  double temp = 0;
				  double pres = 0;
				  double hum = 0;
				  int i = 0;
				  for (i = 0; i<8; i++) {
					  *(((char*)&temp)+i)=data[i];
					  *(((char*)&pres)+i)=data[i+8];
					  *(((char*)&hum)+i)=data[i+16];
				  }
				pkt_out->data = (uint32_t)(pres*1);
			}
			else if(pkt_in->cnt == 4 && !strcmp(pkt_in->name,"HUMIDITY")){
				sprintf(pkt_out->name,"HUMIDITY");
				  board_i2c_select(0,slave_addr);

				  uint8_t data[24];


				  board_i2c_read(&data[0],24);
				  board_i2c_shutdown();


				  double temp = 0;
				  double pres = 0;
				  double hum = 0;
				  int i = 0;
				  for (i = 0; i<8; i++) {
					  *(((char*)&temp)+i)=data[i];
					  *(((char*)&pres)+i)=data[i+8];
					  *(((char*)&hum)+i)=data[i+16];
				  }
				pkt_out->data =(uint32_t)(hum*1000);
			}
#endif
			return 1;

			break;

		case SET_SENSACT:

			pkt_out->msg = SET_SENSACT_ACK;
			pkt_out->new_device = 0;
			pkt_out->cnt = pkt_in->cnt;

			if( (pkt_in->cnt >= 0 && pkt_in->cnt < SAM_SENSACTS_MAX_NUM) && !strcmp(device_list[pkt_in->cnt].name, pkt_in->name)) { //This ordering of the check protects against buffer overflow and undefined behaviour too

				sensact_rw_result_t result;

				device_list[pkt_in->cnt].write(&device_list[pkt_in->cnt], &(pkt_in->data), &result);
				if (result.err == NO_SENSACT_ERROR) pkt_out->data = pkt_in->data; //TODO: possible bug source
				else {
					sprintf(pkt_out->name, "SENSACT ERROR"); //TODO: error handling
					pkt_out->data = pkt_in->data;
				}
			} else {
				sprintf(pkt_out->name, "SENSACT ERROR");
				pkt_out->data = pkt_in->data;
			}

			return 1;

		case SENSACT_ACK:
		case SENSACT_LIST_ACK:
		case ERROR_PKT_MSG:
		default:
			pkt_out->msg = ERROR_PKT_MSG;
			break;
		}
	return 0;
}
int node_is_initialized(){
	return node_initialized;
}
void node_init_pkt(rfnode_pkt* pkt_out){
	pkt_out->msg = SET_IPADDR;
}
void print_pkt_bin(rfnode_pkt* pkt,uip_ip6addr_t* addr){
	int i = 0;
	printf("NEW PACKET!:\n");
	for(i = 0; i<sizeof(rfnode_pkt);i++) printf("%c",((char*)pkt)[i]);
	for(i = 0; i < sizeof(uip_ip6addr_t);i++) printf("%c",addr->u8[i]);
	printf("\nEND OF PACKET!\n");
}
void print_pkt(rfnode_pkt* pkt,uip_ip6addr_t* addr){
	printf("NEW PKT!:\npkt->msg:%d\npkt->cnt:%d\n(int)pkt->data:%d\npkt->name:%s\npkt->new_device:%d\npkt->pkt_cnt:%d\n"
			,pkt->msg,pkt->cnt,(int)pkt->data,pkt->name,pkt->new_device,pkt->pkt_cnt);
	PRINT6ADDR(addr);
	printf("\n");
}
void print_pkt_without_addr(rfnode_pkt* pkt){
	printf("NEW PKT!:\npkt->msg:%d\npkt->cnt:%d\n(int)pkt->data:%d\npkt->name:%s\npkt->new_device:%d\npkt->pkt_cnt:%d\n"
			,pkt->msg,pkt->cnt,(int)pkt->data,pkt->name,pkt->new_device,pkt->pkt_cnt);
	printf("\n");
}

