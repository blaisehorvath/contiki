/*
 * true2air_prot.h
 *
 *  Created on: Oct 11, 2016
 *      Author: v
 */

#ifndef EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_TRUE2AIR_PROT_H_
#define EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_TRUE2AIR_PROT_H_
#include <stdint.h>
#include "net/ip/uip.h"


#define SIMULATED 1
/**
 * @brief   Our application level communication protocol
 * Flow:
 *
 * The flow is directed be the msg in the rfnode_pkt type.
 * The server first sends a GET_SENSACT_LIST command where cnt is 0
 * The node then sends a SENSACT_LIST_ACK back, where the cnt contains the count of the
 * available sensors/actuators
 * After that the server sends a SENSACT_LIST_ITEM command with the number of the item in the cnt
 * The response is a SENSACT_LIST_ITEM with the number of the sensor/actuator in the cnt
 * and the name of the device in the name field of the rfnode_pkt, and the possible
 * binary values of dimensions in the data field
 *
 * Setting/getting the actuator/sensor is possible with the SET/GET_SENSACT type, the answer from the
 * node is a SENSACT_ACK type with the new values. The name and the value in the cnt must be the same.
 *						Description																packet initializer
 */
#define PKT_SIZE sizeof(rfnode_pkt)
typedef enum {
	ERROR_PKT_MSG = 0,	/** Error msg, don't send the PKT													*/
	SET_IPADDR,			/** The node sends this until the server responds with the same msg		(node	)	*/
	GET_SENSACT_LIST,	/**< The gateway requests a list of sensors with this command			(server	)	*/
	SENSACT_LIST_ACK, 	/**< Reply command for a list request: count of sensors in cnt			(node	)	*/
	SENSACT_LIST_ITEM,	/**< Item from the sensor list, with number in cnt						(node	)	*/
	GET_SENSACT,      	/**< Get value and properties of sensor, number in cnt					(server	)	*/
	SET_SENSACT,      	/**< Set value of sensor, number in cnt									(server	)	*/
	SENSACT_ACK,	  	/**< Return the value of sensor for set/get, number in cnt				(node	)	*/
	GET_SENSACT_ACK,	/**< Reply to GET_SENSACT from node										(node	)	*/
	SET_SENSACT_ACK		/**< Reply to SET_SENSACT from node										(node	)	*/
} pkt_msg;

typedef struct {
	uint32_t data;      		/**< Measured / current data */
	uint16_t pkt_cnt;
	pkt_msg msg;        /**< Message type */
	uint8_t cnt;       /**< Count: for ex count of devs / number of current dev */
#ifdef SIMULATED
	char name[22];
#else
	char name[23];
#endif
	uint8_t new_device; /**< New sensor on the node side if true, request of a GET_SENSACT_LIST */
} rfnode_pkt; //TODO: Proper alignment

int node_pkt_reply(rfnode_pkt* pkt_in, rfnode_pkt* pkt_out);
int node_is_initialized();
void node_init_pkt(rfnode_pkt* pkt_out);
void print_pkt(rfnode_pkt* pkt,uip_ip6addr_t* addr);
void print_pkt_bin(rfnode_pkt* pkt,uip_ip6addr_t* addr);
void print_pkt_without_addr(rfnode_pkt* pkt);
#endif /* EXAMPLES_IPV6_RPL_UDP_TRUE2AIR_TRUE2AIR_PROT_H_ */
