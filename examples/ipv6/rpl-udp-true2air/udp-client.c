#define ROM_BOOTLOADER_ENABLE                 1
#define BOOTLOADER_ENABLE 0xC5
#define BL_LEVEL 0x00
#define BL_PIN_NUMBER 0x0B
#define BL_ENABLE 0xC5
#define MMEM_CONF_SIZE 2048

/*---------------------------------------------------------------------------*/
#include <stdio.h>
#include "contiki.h"
#include "contiki-conf.h"
#include "bus_manager.h"
#include "tru2air_i2c_com.h"

// temporary includes
#include "ti-lib.h"
#include <stdbool.h>
#include <string.h>
/*---------------------------------------------------------------------------*/

/*-----------------------------------TESTS-----------------------------------*/
#include "dev/leds.h"


/* Globals */
unsigned char master_dev_id_buff[4];
unsigned char rec_bytes = 4;
unsigned char buff [5];
volatile  tru2air_sensor_node_t DEVICE = {0,0,0};
unsigned char currentSensor = 0; //TODO: reset on the proper place

tru2air_header_t HEADER;

/* BUFFERS */
unsigned char headerBuff[2];
unsigned char nameBuff[23];
unsigned char typeBuff[2];

/* States */
enum states {NODE_I2C_SLAVE_INIT, NODE_I2C_MASTER_INIT, REQUIRE_SENSACT_NAME, REQUIRE_SENSOR_RETURN_TYPE, DEBUG };
volatile enum states STATE = NODE_I2C_SLAVE_INIT;
enum I2C_COMM_PROT_ACTION comm_type = GET_SENSACT_NUM;

//TEMPORARY VARIABLES
#define NO_INTERFACE 0xFF
void clearBuffer();

/*---------------------------------------------------------------------------*/
PROCESS(saul, "saul");
AUTOSTART_PROCESSES(&saul);
/*---------------------------------------------------------------------------*/


void i2c_slave_data_isr () {
	// Reading the Slave Status
	uint32_t ss = I2CSlaveStatus(I2C0_BASE);

	// Clearing the event
	I2CSlaveIntClear(I2C0_BASE, I2C_SLAVE_INT_DATA | I2C_SLAVE_INT_START | I2C_SLAVE_INT_STOP);

	/* Waiting 2 clocks after clearing as suggested in Ti Driverlib CC13xx Ware */
	int i =0;
	i++;

	// If the first byte (FBR) or any master written byte arrived from the master
	if( (I2C_SLAVE_ACT_RREQ_FBR | I2C_SLAVE_ACT_RREQ) & ss) {
		master_dev_id_buff[--rec_bytes] = (unsigned char) I2CSlaveDataGet(I2C0_BASE);
		if (rec_bytes == 0) {
			rec_bytes = 4;
			memcpy(&(DEVICE.dev_addr), master_dev_id_buff, 4);
			DEVICE.i2c_addr = register_i2c_device(DEVICE.dev_addr);
		}
	}
	// If a read byte request came from the master
	else if ( I2C_SLAVE_ACT_TREQ & ss ) {
		I2CSlaveDataPut(I2C0_BASE, DEVICE.i2c_addr);

		// switching state to DEVICE init
		STATE = NODE_I2C_MASTER_INIT;
	}

	//TODO: make an else for error handling

}

PROCESS_THREAD(saul, ev, data)
{
  PROCESS_BEGIN();

  /* Initing the Tru2Air Bus Manager */
  init_i2c_bus_manager();

  /* Init SAM */
//  init_SAM();

  /* RUnning tests */
//  runTests();

  int slave_init = 0;
  int i = 0;
  while(1) {
	  switch (STATE) {
	  	  case ( NODE_I2C_SLAVE_INIT ):
	  			if (slave_init == 0) {

	  					printf("\n[STATE] -> INIT \n[INFO] sensor node i2c_id: 0x%02x dev_addr: 0x%08x \n", DEVICE.i2c_addr, DEVICE.dev_addr);



	  					/* Inititng I2C SLAVE as 0x10 */
	  					init_i2c_slave(0x10, i2c_slave_data_isr);
	  					printf("[STATE] -> NODE_I2C_SLAVE_INIT \n[INFO] sensor node i2c_id: 0x%02x dev_addr: 0x%08x \n", DEVICE.i2c_addr, DEVICE.dev_addr);
	  					slave_init = 1;
	  			}
	  			break;

	  	  case ( NODE_I2C_MASTER_INIT ):

				printf("[STATE] -> NODE_I2C_MASTER_INIT\n");

				if (DEVICE.i2c_addr) {

					//Unregister the slave interrupt and turn off slave mode
					I2CIntUnregister(I2C0_BASE);
					printf("[INFO] unregistered the slave interrupts \n");

					disable_i2c_slave();
					slave_init = 0;
					printf("[INFO] disabling i2c slave \n");

					// Wake up as master
					board_i2c_select(BOARD_I2C_INTERFACE_0, DEVICE.i2c_addr);
					board_i2c_write_single(GET_SENSACT_NUM);
					clearBuffer();
					board_i2c_read(buff,5); //TODO: error handling, check if the dev address is valid and maybe if the SENSACT num is >0?
					DEVICE.sensact_num = buff[4];
					currentSensor = 0;
					board_i2c_shutdown();

					printf("[INFO] tru2air sensor node: 0x%08x has 0x%02x sensors\n", DEVICE.dev_addr, DEVICE.sensact_num);

					STATE = REQUIRE_SENSACT_NAME;
				}
	  			break;

	  	  case ( REQUIRE_SENSACT_NAME ):
	  			headerBuff[0] = GET_SENSOR_NAME;
	  	  	  	headerBuff[1] = currentSensor;
	  	  	  	if (++currentSensor == DEVICE.sensact_num) {
	  	  	  		STATE = REQUIRE_SENSOR_RETURN_TYPE;
	  	  	  		currentSensor = 0;
	  	  	  	}


				printf("[STATE] -> GET_SENSOR_NAME\n");

				board_i2c_select(BOARD_I2C_INTERFACE_0, DEVICE.i2c_addr);
				board_i2c_write(headerBuff, TRU2AIR_HEADER_BUFF_SIZE);
				board_i2c_read_until(nameBuff,'\0');
				board_i2c_shutdown();

				break;

	  	  case ( REQUIRE_SENSOR_RETURN_TYPE ):

	  			headerBuff[0] = GET_SENSOR_TYPE;
	  	  	  	headerBuff[1] = currentSensor;
	  	  	  	if (++currentSensor == DEVICE.sensact_num) {
	  	  	  		currentSensor = 0;
	  	  	  		STATE = 5;
	  	  	  	}

				printf("[STATE] -> GET_SENSOR_TYPE");
				board_i2c_select(BOARD_I2C_INTERFACE_0, DEVICE.i2c_addr);
				board_i2c_write(headerBuff, TRU2AIR_HEADER_BUFF_SIZE);
				board_i2c_read(typeBuff,1);
				board_i2c_shutdown();

				// Registering to SAM


	  			break;

	  	  default:
	  		  printf("[STATE] -> DEFAULT\n[INFO] tru2air sensor node i2c_id: 0x%02x dev_addr: 0x%08x \n", DEVICE.i2c_addr, DEVICE.dev_addr);
	  		  STATE = NODE_I2C_SLAVE_INIT;
	  		  break;
	  }
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/


void clearBuffer() {
	unsigned char i;
	for(i=0 ; i<5; i++ ) {
		buff[i]=0;
	}
}



///*
// * Redistribution and use in source and binary forms, with or without
// * modification, are permitted provided that the following conditions
// * are met:
// * 1. Redistributions of source code must retain the above copyright
// *    notice, this list of conditions and the following disclaimer.
// * 2. Redistributions in binary form must reproduce the above copyright
// *    notice, this list of conditions and the following disclaimer in the
// *    documentation and/or other materials provided with the distribution.
// * 3. Neither the name of the Institute nor the names of its contributors
// *    may be used to endorse or promote products derived from this software
// *    without specific prior written permission.
// *
// * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
// * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
// * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
// * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
// * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
// * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
// * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
// * SUCH DAMAGE.
// *
// * This file is part of the Contiki operating system.
// *
// */
//
//#include "contiki.h"
//#include "lib/random.h"
//#include "sys/ctimer.h"
//#include "net/ip/uip.h"
//#include "net/ipv6/uip-ds6.h"
//#include "net/ip/uip-udp-packet.h"
//#include "sys/ctimer.h"
//
//#include <stdio.h>
//#include <string.h>
//#include "true2air_prot.h"
//
///* Only for TMOTE Sky? */
//#include "dev/serial-line.h"
//#include "dev/uart1.h"
//#include "net/ipv6/uip-ds6-route.h"
//#include "dev/leds.h"
//#define UDP_CLIENT_PORT 8765
//#define UDP_SERVER_PORT 5678
//
//#define UDP_EXAMPLE_ID  190
//
//#define DEBUG DEBUG_FULL
//#include "net/ip/uip-debug.h"
//
//#ifndef PERIOD
//#define PERIOD 10
//#endif
//
//#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
//#define START_INTERVAL		(15 * CLOCK_SECOND)
//#define SEND_INTERVAL		(PERIOD * CLOCK_SECOND)
//#define SEND_TIME		(random_rand() % (SEND_INTERVAL))
//#define SET_IP_TIME (10* CLOCK_SECOND)
//#define MAX_PAYLOAD_LEN		30
//
//#define ROM_BOOTLOADER_ENABLE                 1
//#define BOOTLOADER_ENABLE = 0xC5
//#define BL_LEVEL = 0x00
//#define BL_PIN_NUMBER = 0x0B
//#define BL_ENABLE = 0xC5
//
//static struct uip_udp_conn *client_conn;
//static uip_ipaddr_t server_ipaddr;
//
////#############################################################################################
//#include "bus_manager.h"
///* Globals */
//enum states {I2C_SLAVE_LISTEN, NODE_I2C_MASTER_INIT, REQUIRE_SENSACT_NAME, REQUIRE_SENSOR_RETURN_TYPE };
//volatile enum states STATE = I2C_SLAVE_LISTEN;
//unsigned char master_dev_id_buff[4];
//unsigned char rec_bytes = 4;
//unsigned char buff [5];
//volatile  tru2air_sensor_node_t DEVICE = {0,0,0};
//unsigned char currentSensor = 0; //TODO: reset on the proper place
//void i2c_slave_data_isr () {
//	// Reading the Slave Status
//	uint32_t ss = I2CSlaveStatus(I2C0_BASE);
//
//	// Clearing the event
//	I2CSlaveIntClear(I2C0_BASE, I2C_SLAVE_INT_DATA | I2C_SLAVE_INT_START | I2C_SLAVE_INT_STOP);
//
//	/* Waiting 2 clocks after clearing as suggested in Ti Driverlib CC13xx Ware */
//	int i =0;
//	i++;
//
//	// If the first byte (FBR) or any master written byte arrived from the master
//	if( (I2C_SLAVE_ACT_RREQ_FBR | I2C_SLAVE_ACT_RREQ) & ss) {
//		master_dev_id_buff[--rec_bytes] = (unsigned char) I2CSlaveDataGet(I2C0_BASE);
//		if (rec_bytes == 0) {
//			rec_bytes = 4;
//			memcpy(&(DEVICE.dev_addr), master_dev_id_buff, 4);
//			DEVICE.i2c_addr = register_i2c_device(DEVICE.dev_addr);
//		}
//	}
//	// If a read byte request came from the master
//	else if ( I2C_SLAVE_ACT_TREQ & ss ) {
//		I2CSlaveDataPut(I2C0_BASE, DEVICE.i2c_addr);
//
//		// switching state to DEVICE init
//		STATE = NODE_I2C_MASTER_INIT;
//	}
//
//	//TODO: make an else for error handling
//
//}
//
///*---------------------------------------------------------------------------*/
//PROCESS(udp_client_process, "UDP client process");
//AUTOSTART_PROCESSES(&udp_client_process);
///*---------------------------------------------------------------------------*/
//static void
//tcpip_handler(void)
//{
//  static unsigned int slave_addr = 0x10;
//  init_i2c_slave(slave_addr, i2c_slave_data_isr);
//  printf("started i2c slave on 0x%02x \n",slave_addr);
//  rfnode_pkt pkt_out;
//  rfnode_pkt *pkt_in;
//  memset(&pkt_out,0,sizeof(rfnode_pkt));
//  if(uip_newdata()) {
//	pkt_in = (rfnode_pkt*)uip_appdata;
//	if(node_pkt_reply(pkt_in,&pkt_out))
//	  uip_udp_packet_sendto(client_conn, (void*)&pkt_out, sizeof(rfnode_pkt),
//	                        &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
//  }
//}
///*---------------------------------------------------------------------------*/
//static void
//send_init_packet(void *ptr)
//{
//  rfnode_pkt pkt;int i = 0;
//  PRINTF("Send SET_IPADDR to the server\n");
//  pkt.cnt=1;
//  pkt.data=2;
//  pkt.new_device = 0;
//  pkt.msg = SET_IPADDR;
//  pkt.pkt_cnt = 0;
//#ifdef SIMULATED
//  for ( i = 0; i < 22 ; i++) pkt.name[i] = 0;
//#else
//  for ( i = 0; i < 23 ; i++) pkt.name[i] = 0;
//#endif
//  sprintf(pkt.name, "SETIPADDRMSG");
//  PRINTF("ZEROED\n");
//  uip_udp_packet_sendto(client_conn, (void*)&pkt, sizeof(rfnode_pkt),
//                        &server_ipaddr, UIP_HTONS(UDP_SERVER_PORT));
//}
///*---------------------------------------------------------------------------*/
//static void
//print_local_addresses(void)
//{
//  int i;
//  uint8_t state;
//
//  PRINTF("Client IPv6 addresses: ");
//  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
//    state = uip_ds6_if.addr_list[i].state;
//    if(uip_ds6_if.addr_list[i].isused &&
//       (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
//      PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
//      PRINTF("\n");
//      /* hack to make address "final" */
//      if (state == ADDR_TENTATIVE) {
//	uip_ds6_if.addr_list[i].state = ADDR_PREFERRED;
//      }
//    }
//  }
//}
///*---------------------------------------------------------------------------*/
//static void
//set_global_address(void)
//{
//  uip_ipaddr_t ipaddr;
//
//  uip_ip6addr(&ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 0);
//  uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
//  uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);
//
///* The choice of server address determines its 6LoPAN header compression.
// * (Our address will be compressed Mode 3 since it is derived from our link-local address)
// * Obviously the choice made here must also be selected in udp-server.c.
// *
// * For correct Wireshark decoding using a sniffer, add the /64 prefix to the 6LowPAN protocol preferences,
// * e.g. set Context 0 to fd00::.  At present Wireshark copies Context/128 and then overwrites it.
// * (Setting Context 0 to fd00::1111:2222:3333:4444 will report a 16 bit compressed address of fd00::1111:22ff:fe33:xxxx)
// *
// * Note the IPCMV6 checksum verification depends on the correct uncompressed addresses.
// */
//
//#if 0
///* Mode 1 - 64 bits inline */
//   uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0, 0, 1);
//#elif 1
///* Mode 2 - 16 bits inline */
//  uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0, 0x00ff, 0xfe00, 1);
//#else
///* Mode 3 - derived from server link-local (MAC) address */
//  uip_ip6addr(&server_ipaddr, UIP_DS6_DEFAULT_PREFIX, 0, 0, 0, 0x0250, 0xc2ff, 0xfea8, 0xcd1a); //redbee-econotag
//#endif
//}
///*---------------------------------------------------------------------------*/
//PROCESS_THREAD(udp_client_process, ev, data)
//{
//  static struct etimer periodic;
//
//  PROCESS_BEGIN();
//
//  PROCESS_PAUSE();
//
//  set_global_address();
//
//  PRINTF("UDP client process started nbr:%d routes:%d\n",
//         NBR_TABLE_CONF_MAX_NEIGHBORS, UIP_CONF_MAX_ROUTES);
//
//  print_local_addresses();
//
//  /* new connection with remote host */
//  client_conn = udp_new(NULL, UIP_HTONS(UDP_SERVER_PORT), NULL);
//  if(client_conn == NULL) {
//    PRINTF("No UDP connection available, exiting the process!\n");
//    PROCESS_EXIT();
//  }
//  udp_bind(client_conn, UIP_HTONS(UDP_CLIENT_PORT));
//
//  PRINTF("Created a connection with the server ");
//  PRINT6ADDR(&client_conn->ripaddr);
//  PRINTF(" local/remote port %u/%u\n",
//	UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));
//
//  /* initialize serial line */
//  uart1_set_input(serial_line_input_byte);
//  serial_line_init();
//
//  etimer_set(&periodic, SEND_INTERVAL);
//  printf("sizeof(rfnode_pkt:%d \n",sizeof(rfnode_pkt));
//  leds_arch_init();
//
//
//  static unsigned int slave_addr = 0x10;
//  init_i2c_slave(slave_addr, i2c_slave_data_isr);
//  printf("started i2c slave on 0x%02x \n",slave_addr);
//
//  while(1) {
//    PROCESS_YIELD();
//    if(ev == tcpip_event) {
//      tcpip_handler();
//    }
//    if(etimer_expired(&periodic) && !node_is_initialized()){
//    	etimer_reset(&periodic);
//    	send_init_packet(0);
//    }
//  }
//
//  PROCESS_END();
//}
///*---------------------------------------------------------------------------*/
