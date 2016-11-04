/*
 * true2air_prot.c
 *
 *  Created on: Oct 11, 2016
 *      Author: v
 */
#include "true2air_prot.h"
#include <stdio.h>
#include <stdint.h>
#include "net/ip/uip.h"

#define DEBUG DEBUG_PRINT
#include "net/ip/uip-debug.h"
#include "dev/leds.h"
uint8_t node_initialized = 0;
int node_pkt_reply(rfnode_pkt* pkt_in, rfnode_pkt* pkt_out)
{
	leds_on(LEDS_BLUE);
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
		case GET_SENSACT_LIST:
			pkt_out->msg = SENSACT_LIST_ACK;
			pkt_out->data = 0;
			pkt_out->new_device = 0;
			sprintf(pkt_out->name,"REPLY FROM NODE!");
			pkt_out->cnt = 2;
			return 1;
		case SENSACT_LIST_ITEM:
			pkt_out->msg = SENSACT_LIST_ITEM;
			pkt_out->data = 0;
			pkt_out->new_device = 0;
			pkt_out->cnt = pkt_in->cnt;
			switch(pkt_in->cnt){
				case 0:
					sprintf(pkt_out->name,"LED(RED)");
					return 1;
				case 1:
					sprintf(pkt_out->name,"LED(GREEN)");
					return 1;
				default:
					pkt_out->msg = ERROR_PKT_MSG;
			}
			return 1;
		case GET_SENSACT: // Dummy sensor handler
			pkt_out->msg = GET_SENSACT_ACK;
			pkt_out->new_device = 0;
			pkt_out->cnt = pkt_in->cnt;
			if(pkt_in->cnt == 0 && !strcmp(pkt_in->name,"LED(RED)")){
				sprintf(pkt_out->name,"LED(RED)");
				pkt_out->data = (leds_get() &LEDS_RED) == 1; //WROOOOONG!!!!
			}
			else if(pkt_in->cnt == 1 && !strcmp(pkt_in->name,"LED(GREEN)")){
				sprintf(pkt_out->name,"LED(GREEN)");
				pkt_out->data = (leds_get() &LEDS_GREEN) == 1;// WROOONG
			}
			else sprintf(pkt_out->name,"WRONG NUMBER/NAME!");
			return 1;

			break;
		case SET_SENSACT: // Dummy sensor handler
			pkt_out->msg = SET_SENSACT_ACK;
			pkt_out->data = pkt_in->data;
			pkt_out->new_device = 0;
			pkt_out->cnt = pkt_in->cnt;
			if(pkt_in->cnt == 0 && !strcmp(pkt_in->name,"LED(RED)")){
				sprintf(pkt_out->name,"LED(RED)");
				pkt_in->data?leds_on(LEDS_RED):leds_off(LEDS_RED);
			}
			else if(pkt_in->cnt == 1 && !strcmp(pkt_in->name,"LED(GREEN)")){
				sprintf(pkt_out->name,"LED(GREEN)");
				pkt_in->data?leds_on(LEDS_GREEN):leds_off(LEDS_GREEN);
			}
			else sprintf(pkt_out->name,"WRONG NUMBER/NAME!");
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

