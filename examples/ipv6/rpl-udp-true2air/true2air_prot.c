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

uint8_t device_initialized = 0;
uint8_t new_sensor = 0;

int node_pkt_reply(rfnode_pkt *pkt_in, rfnode_pkt *pkt_out) {
    int i = 0;
    /*Initializing local vars*/
    sensact_rw_result_t result;
    device_initialized = 1;

    /*Managing pkt*/
    print_pkt_without_addr(pkt_in);
    pkt_out->pkt_cnt = pkt_in->pkt_cnt;

    switch (pkt_in->msg) {

        case SET_IPADDR:
            printf("SET IPADDR, init:%d\n", new_sensor);
            pkt_out->msg = SET_IPADDR;// Don't answer
            memset(pkt_out->data[0], 0x00, SENSACT_DATA_SIZE);
            pkt_out->data[0] = 0;
            pkt_out->new_device = new_sensor;
            sprintf(pkt_out->name, "REPLY SET_IPADDR!");
            pkt_out->cnt = 2;
            pkt_out->error = NO_SENSACT_ERROR;
            return 1;

            /* Get the number of sensors actuators the tru2air node has*/
        case GET_SENSACT_LIST: //TODO: it would be better and more consistent to use pkt-> data instead of pkt->cnt for sending the sam devices number
            pkt_out->msg = SENSACT_LIST_ACK;
            new_sensor = 0;
            memset(pkt_out->data, 0x00, SENSACT_DATA_SIZE);
            pkt_out->data[0] = 0;
            new_sensor = 0;
            pkt_out->new_device = new_sensor;
            sprintf(pkt_out->name, "REPLY FROM NODE!");
            for (i = 0; i < SAM_SENSACTS_MAX_NUM; i++) {
                if (device_list[i].name[0])//TODO: device exsists.. check a better way
                {
                    pkt_out->data[i] = 1; //TODO: We could address different states of sensor here, so this should be a part of an enum
                }
            }
            pkt_out->cnt = sam_get_sensact_num();
            pkt_out->error = NO_SENSACT_ERROR;
            return 1;

            /* Senging the info of the pkt_in->cnt'th sensor's name */
        case SENSACT_LIST_ITEM: //TODO: again it would be better to use data instead of cnt

            /* Setting up the outgoing pkt */
            memset(pkt_out->data, 0x00, SENSACT_DATA_SIZE); //TODO: remove this when read is used through SAM
            pkt_out->msg = SENSACT_LIST_ITEM;
            pkt_out->new_device = new_sensor;
            pkt_out->cnt = pkt_in->cnt;

            /* If everything ok, continue filling in the out pkt*/
            if (pkt_in->cnt >= 0 && pkt_in->cnt < SAM_SENSACTS_MAX_NUM) {

                //uint16_t* ptr = &device_list[pkt_in->cnt].sensact_type-1;
                //memcpy(pkt_out->data, ptr, 2); // copy the sensact type to the data field

                pkt_out->data[0] = ((char *) (&device_list[pkt_in->cnt].sensact_type))[1]; //TODO: This is so bad on many levels... Should find a good way...
                pkt_out->data[1] = ((char *) (&device_list[pkt_in->cnt].sensact_type))[0];


                strcpy(pkt_out->name, device_list[pkt_in->cnt].name); // copying name

                printf("[SENSACT TYPE] %s : 0x%04x : 0x%04x \n", device_list[pkt_in->cnt].name,
                       device_list[pkt_in->cnt].sensact_type, *(uint16_t * ) & pkt_out->data[1]);
            }
            else { pkt_out->error = INVALID_SAM_ADDR; }
            return 1;

            /* Send something mesured by the pkt_in'th sensor */
        case GET_SENSACT: // Dummy sensor handler

            memset(result.data, 0x00, SENSACT_DATA_SIZE);

            pkt_out->msg = GET_SENSACT_ACK;
            pkt_out->new_device = new_sensor;
            pkt_out->cnt = pkt_in->cnt;


            if (pkt_in->cnt < 0 && pkt_in->cnt > SAM_SENSACTS_MAX_NUM) {
                pkt_out->error = INVALID_SAM_ADDR;
                return 1;
            }
            if (strcmp(device_list[pkt_in->cnt].name, pkt_in->name)) {
                pkt_out->error = SENSACT_MISSING;
                return 1;
            }

            device_list[pkt_in->cnt].read(&device_list[pkt_in->cnt], &result);

            if (result.err == NO_SENSACT_ERROR) {
                sprintf(pkt_out->name, device_list[pkt_in->cnt].name);
                memcpy(pkt_out->data, result.data, SENSACT_DATA_SIZE);
            }
            else {
                pkt_out->error = result.err;
            }

            return 1;
            break;

        case SET_SENSACT:

            pkt_out->msg = SET_SENSACT_ACK;
            pkt_out->new_device = new_sensor;
            pkt_out->cnt = pkt_in->cnt;

            if (pkt_in->cnt < 0 || pkt_in->cnt > SAM_SENSACTS_MAX_NUM) {
                memcpy(pkt_out->data, pkt_in->data, SENSACT_DATA_SIZE); //TODO: is it unecessary?
                pkt_out->error = INVALID_SAM_ADDR;
                return 1;
            }

            if (strcmp(device_list[pkt_in->cnt].name,
                       pkt_in->name)) { //This ordering of the check protects against buffer overflow and undefined behaviour too
                pkt_out->error = SENSACT_MISSING;
                return 1;
            }

            device_list[pkt_in->cnt].write(&device_list[pkt_in->cnt], pkt_in->data, &result);

            if (result.err == NO_SENSACT_ERROR)
                memcpy(pkt_out->data, pkt_in->data, SENSACT_DATA_SIZE); //TODO: possible bug source
            else {
                sprintf(pkt_out->name, "SENSACT ERROR"); //TODO: error handling
                memcpy(pkt_out->data, pkt_in->data, SENSACT_DATA_SIZE);
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

int node_is_initialized() {
    return device_initialized;
}

void node_init_pkt(rfnode_pkt *pkt_out) {
    pkt_out->msg = SET_IPADDR;
}

void print_pkt_bin(rfnode_pkt *pkt, uip_ip6addr_t *addr) {
    int i = 0;
    printf("NEW PACKET!:\n");
    for (i = 0; i < sizeof(rfnode_pkt); i++) printf("%c", ((char *) pkt)[i]);
    for (i = 0; i < sizeof(uip_ip6addr_t); i++) printf("%c", addr->u8[i]);
    printf("\nEND OF PACKET!\n");
}

void print_pkt(rfnode_pkt *pkt, uip_ip6addr_t *addr) {
    printf("NEW PKT!:\npkt->msg:%d\npkt->cnt:%d\n(int)pkt->data:%d\npkt->name:%s\npkt->new_device:%d\npkt->pkt_cnt:%d\n",
           pkt->msg, pkt->cnt, (int) pkt->data, pkt->name, pkt->new_device, pkt->pkt_cnt);
    PRINT6ADDR(addr);
    printf("\n");
}

void print_pkt_without_addr(rfnode_pkt *pkt) {
    printf("NEW PKT!:\npkt->msg:%d\npkt->cnt:%d\n(int)pkt->data:%d\npkt->name:%s\npkt->new_device:%d\npkt->pkt_cnt:%d\n",
           pkt->msg, pkt->cnt, (int) pkt->data, pkt->name, pkt->new_device, pkt->pkt_cnt);
    printf("\n");
}

